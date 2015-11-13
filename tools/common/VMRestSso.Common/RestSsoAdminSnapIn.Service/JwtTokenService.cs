/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using System.Collections.Generic;
using System.IdentityModel.Tokens;
using System.Net;
using System.Security.Claims;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service
{
    public class JwtTokenService : IAuthenticationService
    {
        private readonly IWebRequestManager _webRequestManager;
        public JwtTokenService(IWebRequestManager webRequestManager)
        {
            _webRequestManager = webRequestManager;
        }

        public AuthTokenDto Authenticate(ServerDto serverDto, LoginDto loginDto, string clientId)
        {
            var tenant = Uri.EscapeDataString(loginDto.TenantName);
            var url = string.Format(ServiceConfigManager.LoginEndPoint, serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var data = string.Format(ServiceConfigManager.LoginArguments, loginDto.User, loginDto.Pass, loginDto.DomainName, clientId);
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders();
            var result = _webRequestManager.GetResponse(url, requestConfig, headers, null, data);
            var token = JsonConvert.Deserialize<Token>(result);
            token.Raw = result;
            token.ClientId = clientId;
            token.TokenType = TokenType.Bearer.ToString();
            var certificates = GetCertificates(serverDto, loginDto.TenantName, CertificateScope.TENANT, token);
            var claimsPrincipal = Validate(serverDto, loginDto.User + "@" + loginDto.DomainName, certificates[certificates.Count - 1], loginDto.TenantName, token.IdToken);
            if (claimsPrincipal != null)
                return new AuthTokenDto(Refresh) { Token = token, ClaimsPrincipal = claimsPrincipal, Login = loginDto, ServerDto = serverDto };
            return new AuthTokenDto(Refresh) { Token = token, ClaimsPrincipal = claimsPrincipal, Login = loginDto, ServerDto = serverDto };
            //throw new AuthenticationException(@"Login Failure: Invalid username or password");
        }

        public Token Refresh(ServerDto serverDto, LoginDto loginDto, Token tokenToRefresh)
        {
            var tenant = Uri.EscapeDataString(loginDto.TenantName);
            var url = string.Format(ServiceConfigManager.RefreshTokenEndPoint, serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var data = string.Format(ServiceConfigManager.RefreshTokenArguments, tokenToRefresh.RefreshToken, tokenToRefresh.ClientId);
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders();
            var result = _webRequestManager.GetResponse(url, requestConfig, headers, null, data);
            var token = JsonConvert.Deserialize<Token>(result);
            token.RefreshToken = tokenToRefresh.RefreshToken;
            token.ClientId = tokenToRefresh.ClientId;
            return token;
        }

        private ClaimsPrincipal Validate(ServerDto serverDto, string audience, CertificateChainDto certificateChain, string tenantName, string token)
        {
            var certificate = certificateChain.Certificates[0];
            var publicKey = certificate.Encoded;
            var x509Certificate2 = new X509Certificate2();
            var cert = Encoding.UTF8.GetBytes(publicKey);
            x509Certificate2.Import(cert);
            var hostName = ServiceHelper.GetHostName(serverDto.ServerName);
            var validationParams = new TokenValidationParameters
            {
                ValidIssuer = string.Format(ServiceConfigManager.ValidationUri, serverDto.Protocol, hostName, tenantName),
                ValidAudience = audience,
                IssuerSigningToken = new X509SecurityToken(x509Certificate2),
                ValidateIssuer = false
            };

            var jwtSecurityTokenHandler = new JwtSecurityTokenHandler();
            SecurityToken validatedToken;
            return jwtSecurityTokenHandler.ValidateToken(token, validationParams, out validatedToken);
        }

        private List<CertificateChainDto> GetCertificates(ServerDto serverDto, string tenantName, CertificateScope scope, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var url = string.Format(ServiceConfigManager.CertificatesEndPoint, serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName);
            url += "?scope=" + scope;
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Get
            };
            var headers = ServiceHelper.AddHeaders(ServiceConfigManager.JsonContentType);
            var authorization = string.Format("{0} {1}", token.TokenType, token.AccessToken);
            headers.Add(HttpRequestHeader.Authorization, authorization);
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, null);
            return JsonConvert.Deserialize<List<CertificateChainDto>>(response);
        }

        public AuthTokenDto GetTokenFromCertificate(ServerDto serverDto, X509Certificate2 certificate, RSACryptoServiceProvider rsa)
        {
            //token_class: "solution_assertion",
            //token_type: "Bearer",
            //jti: <randomly generated id string>,
            //iss: <cert subject dn>,
            //sub: <cert subject dn>,
            //aud: <token endpoint url>,
            //iat: 1431623789,
            //exp: 1462382189            
            var url = string.Format(ServiceConfigManager.LoginEndPoint, serverDto.Protocol, serverDto.ServerName, serverDto.Port, serverDto.Tenant);
            var signedToken = GetSignedJwtToken(rsa, certificate, url);
            if (signedToken == null)
                throw new Exception("Could not generate a valid token");

            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var data = string.Format(ServiceConfigManager.JwtTokenBySolutionUserArguments, signedToken);
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders();
            var result = _webRequestManager.GetResponse(url, requestConfig, headers, null, data);
            var token = JsonConvert.Deserialize<Token>(result);
            token.Raw = result;
            return new AuthTokenDto(Refresh) { Token = token, ClaimsPrincipal = null, Login = null, ServerDto = serverDto };
        }

        /*private string GetSignedJwtToken(string rsaKeyFilePath, X509Certificate2 cert, string url)
        {
            //var claimsetSerialized = @"{""token_class"":""solution_assertion"",""token_type"":""Bearer""" +
            //    @",""jti"":""" + new Random().Next().ToString() + @"""" +
            //    @",""iss"":""" + cert.Subject + @""",""sub"":""" + cert.Subject + @"""" +
            //    @",""aud"":""" + url + @""",""iat"":""" + DateTimeHelper.WindowsToUnix(DateTime.Now).ToString() + @"""" +
            //    @",""exp"":""" + DateTimeHelper.WindowsToUnix(DateTime.Now.AddTicks(356 * 24 * 60 * 60)).ToString() + @"""}";

            // header
            //var headerSerialized = @"{""typ"":""JWT"", ""alg"":""RS256""}";
            var header = new JwtHeaderDto { Typ = "JWT", Alg = "RS256" };

            // encoded header
            var headerSerialized = JsonConvert.Serialize(header);
            var headerBytes = Encoding.UTF8.GetBytes(headerSerialized);
            var headerEncoded = Convert.ToBase64String(headerBytes);

            // encoded claimset
            var claimset = new JwtClaimDto
            {
                TokenClass = "solution_assertion",
                TokenType = "Bearer",
                Jti = new Random().Next().ToString(),
                Iss = cert.Subject,
                Sub = cert.Subject,
                Aud = url,
                Iat = DateTimeHelper.WindowsToUnix(DateTime.Now).ToString(),
                Exp = DateTimeHelper.WindowsToUnix(DateTime.Now.AddTicks(356 * 24 * 60 * 60)).ToString()
            };
            //var claimset = new JwtPayload
            //{
            //    Jti = new Random().Next().ToString(),
            //    Iss = cert.Subject,
            //    Sub = cert.Subject,
            //    Aud = url,
            //    Iat = DateTimeHelper.WindowsToUnix(DateTime.Now).ToString(),
            //    Exp = DateTimeHelper.WindowsToUnix(DateTime.Now.AddTicks(356 * 24 * 60 * 60)).ToString(),
            //    Claims = new List<Claim>() { new Claim("token_class", "solution_assertion"), new Claim("token_type", "Bearer") }
            //};
            //var claimset = new JwtPayload();
            //var claims = new List<Claim>();
            //claims.Add(new Claim("token_class", "solution_assertion"));
            //claims.Add(new Claim("token_type", "Bearer"));
            //claims.Add(new Claim("jti", new Random().Next().ToString()));
            //claims.Add(new Claim("iss", cert.Subject));
            //claims.Add(new Claim("sub", cert.Subject));
            //claims.Add(new Claim("aud", url));
            //claims.Add(new Claim("iat", DateTimeHelper.WindowsToUnix(DateTime.Now).ToString()));
            //claims.Add(new Claim("exp", DateTimeHelper.WindowsToUnix(DateTime.Now.AddTicks(356 * 24 * 60 * 60)).ToString()));
            //claimset.AddClaims(claims);
            var claimsetSerialized = JsonConvert.Serialize(claimset);
            var claimsetBytes = Encoding.UTF8.GetBytes(claimsetSerialized);
            var claimsetEncoded = Convert.ToBase64String(claimsetBytes);

            // input
            var input = String.Join(".", headerEncoded, claimsetEncoded);
            var inputBytes = Encoding.UTF8.GetBytes(input);

            //var signatureBytes = rsaKey.SignData(inputBytes, "SHA256");
            var signature = ShaWithRsaSigner.Sign(input, rsaKeyFilePath, "SHA256");
            var signatureBytes = Encoding.UTF8.GetBytes(signature);
            var signatureEncoded = Convert.ToBase64String(signatureBytes);

            // jwt
            return String.Join(".", input, signatureEncoded);
        }*/
        private string GetSignedJwtToken(RSACryptoServiceProvider rsa, X509Certificate2 cert, string url)
        {
            var claims = new List<Claim>();
            claims.Add(new Claim("token_class", "solution_assertion"));
            claims.Add(new Claim("token_type", "Bearer"));
            claims.Add(new Claim("jti", new Random().Next().ToString()));
            claims.Add(new Claim("sub", cert.Subject));
            var payload = new JwtPayload(cert.Issuer, url, claims, DateTime.Now, DateTime.Now.AddMinutes(5));

            //var cert = new X509Certificate2(Encoding.UTF8.GetBytes(certificate.Encoded));
            //var signingCredentials = new X509SigningCredentials(cert);
            //var payload = new JwtPayload(cert.Subject,url, DateTime.Now,DateTime.Now.AddMinutes(5));
            //var claims = new List<Claim>();
            //claims.Add(new Claim("token_class", "solution_assertion"));
            //claims.Add(new Claim("token_type", "Bearer"));
            //claims.Add(new Claim("jti", new Random().Next().ToString()));
            //claims.Add(new Claim("iss", cert.Subject));
            //claims.Add(new Claim("sub", cert.Subject));
            //claims.Add(new Claim("aud", url));
            //claims.Add(new Claim("iat", DateTimeHelper.WindowsToUnix(DateTime.Now).ToString()));
            //claims.Add(new Claim("exp", DateTimeHelper.WindowsToUnix(DateTime.Now.AddTicks(356 * 24 * 60 * 60)).ToString()));
            //payload.Exp = DateTimeHelper.WindowsToUnix(DateTime.Now.AddTicks(356 * 24 * 60 * 60));
            //payload.Iat = DateTimeHelper.WindowsToUnix(DateTime.Now);
            //payload.AddClaims(claims);

            //var keyBytes = GetBytes(privateKey.Encoded);
            //var key = new InMemorySymmetricSecurityKey(keyBytes);
            var key = new RsaSecurityKey(rsa);
            var signingCredentials = new SigningCredentials(key, SecurityAlgorithms.RsaSha256Signature, SecurityAlgorithms.Sha256Digest);

            var header = new JwtHeader(signingCredentials);
            var token = new JwtSecurityToken(header, payload);
            var jwtSecurityTokenHandler = new JwtSecurityTokenHandler();
            try
            {                
                var jsonToken = jwtSecurityTokenHandler.WriteToken(token);
                return jsonToken;
            }
            catch (Exception)
            {
                // todo: Log exception                
            }
            return null;
        }

        private byte[] GetBytes(string str)
        {
            byte[] bytes = new byte[str.Length * sizeof(char)];
            System.Buffer.BlockCopy(str.ToCharArray(), 0, bytes, 0, bytes.Length);
            return bytes;
        }

        public AuthTokenDto GetTokenFromGssTicket(ServerDto serverDto, string base64EncodedGSSTicketBytes, string clientId)
        {
            var url = string.Format(ServiceConfigManager.LoginEndPoint, serverDto.Protocol, serverDto.ServerName, serverDto.Port, serverDto.Tenant);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var data = string.Format(ServiceConfigManager.GssTicketLoginArguments, base64EncodedGSSTicketBytes, clientId);
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders();
            var result = _webRequestManager.GetResponse(url, requestConfig, headers, null, data);
            var token = JsonConvert.Deserialize<Token>(result);
            token.Raw = result;
            token.ClientId = clientId;
            return new AuthTokenDto(Refresh) { Token = token, ClaimsPrincipal = null, Login = null, ServerDto = serverDto };
        }
    }
}

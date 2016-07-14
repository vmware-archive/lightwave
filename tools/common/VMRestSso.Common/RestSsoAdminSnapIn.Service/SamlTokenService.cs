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
using System.IO;
using System.Net;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Xml;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service
{
    public class SamlTokenService : IAuthenticationService
    {
        private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public SamlTokenService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
        {
            _webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
        }
        public AuthTokenDto Authenticate(ServerDto serverDto, LoginDto loginDto, string clientId)
        {
            var url = string.Format(_serviceConfigManager.GetSamlLegacyEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, serverDto.StsUrl + "/" + loginDto.TenantName);
            string soapString = XmlResourceHelper.GetResourceXml("Vmware.Tools.RestSsoAdminSnapIn.Service.xml.SAMLRequest.xml");

            int lifeInSeconds = 300;
            var dt = DateTime.Now;
            dt = TimeZoneInfo.ConvertTimeToUtc(dt);
            var dtEnd = dt.AddSeconds(lifeInSeconds);
            string format = "yyyy-MM-ddTHH:mm:ss.fffZ";
            var pass = WrapInCDATA(loginDto.Pass);
            var principalName = loginDto.User + "@" + loginDto.DomainName;
            soapString = string.Format(soapString, dt.ToString(format), dtEnd.ToString(format), principalName, pass, dt.ToString(format), dtEnd.ToString(format));

            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders("text/xml");
            var customHeaders = new Dictionary<string,string>();
            customHeaders.Add("SOAPAction", "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Issue");
            var responseFromServer = _webRequestManager.GetResponse(url, requestConfig, headers, customHeaders, soapString);

            XmlDocument doc = new XmlDocument();
            doc.PreserveWhitespace = false;
            using (var reader = new StringReader(responseFromServer))
                doc.Load(reader);
            var node = doc.GetElementsByTagName("saml2:Assertion")[0];
            var signature = doc.GetElementsByTagName("ds:SignatureValue")[0];
			var groups = doc.GetElementsByTagName ("saml2:AttributeValue");

			var role = GetRole (loginDto.TenantName, groups);
            var rawToken = string.Empty;
            if (node != null)
            {
                rawToken = node.OuterXml;
            }
            byte[] bytes = System.Text.Encoding.UTF8.GetBytes(rawToken);
            rawToken = Convert.ToBase64String(bytes);
			var token = new Token(rawToken, serverDto.TokenType) { 
				Raw = rawToken, 
				ClientId = clientId, 
				Signature = signature.InnerXml,
				TokenType = TokenType.SAML.ToString(), 
				Role = role
			};
            return new AuthTokenDto(Refresh) { Token = token, ClaimsPrincipal = null, Login = loginDto, ServerDto = serverDto };
            throw new Exception(responseFromServer);
        }

		private string GetRole(string domain, XmlNodeList groups){

			var role = string.Empty;
			var adminGroup = domain + "\\Administrators";
			var usersGroup = domain + "\\Users";
			var everyoneGroup = domain + "\\Everyone";

			if (groups != null) {
				bool isAdmin = false;
				bool isUser = false;
				bool isGuest = false;
				foreach (XmlNode group in groups) {
					if (group.InnerText == adminGroup) {
						isAdmin = true;
					}
					if (group.InnerText == usersGroup) {
						isUser = true;;
					}
					if (group.InnerText == everyoneGroup) {
						isGuest = true;
					}
				}

				if (isAdmin) {
					role = "Administrator";
				} else if (isUser) {
					role = "RegularUser";
				} else if (isGuest) {
					role = "GuestUser";
				}
			}
			return role;
		}
        public string GetSamlTokenFromToken(ServerDto serverDto, string tokenXML, X509Certificate2 cert, RSACryptoServiceProvider rsaKey)
        {            
            var soapString = XmlResourceHelper.GetResourceXml("Vmware.Tools.RestSsoAdminSnapIn.Service.xml.SamlTokenByToken.xml");
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            string signed = "";
            var dt = DateTime.Now;
            var dtStart = TimeZoneInfo.ConvertTimeToUtc(dt);
            var dtEnd = dtStart.AddMinutes(10);
            string format = "yyyy-MM-ddTHH:mm:ss.fffZ";

            var xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(tokenXML);
            var samlAssertion = xmlDoc.GetElementsByTagName("Assertion", "urn:oasis:names:tc:SAML:2.0:assertion");

            var certString = Convert.ToBase64String(cert.RawData);
            string dtStartStr = dtStart.ToString(format);
            string dtEndStr = dtEnd.ToString(format);
            soapString = string.Format(soapString, dtStartStr, dtEndStr, certString, samlAssertion[0].OuterXml, dtStartStr, dtEndStr);

            signed = SigningHelper.SignXmlFile(soapString, rsaKey);

            string xml2 = XmlResourceHelper.GetResourceXml("Vmware.Tools.RestSsoAdminSnapIn.Service.xml.SamlTokenByToken2.xml");
            xml2 = string.Format(xml2, dtStartStr, dtEndStr, certString, samlAssertion[0].OuterXml, signed, dtStartStr, dtEndStr);

            XmlDocument doc = new XmlDocument();
            doc.PreserveWhitespace = false;
            doc.LoadXml(xml2);

            soapString = doc.InnerXml;
            var customHeaders = new Dictionary<string, string>();
            customHeaders.Add("SOAPAction", "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Issue");
            var headers = ServiceHelper.AddHeaders("text/xml");
            var url = serverDto.Url;
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, customHeaders, soapString);
            return response;
        }

        public string GetSamlTokenFromCertificate(ServerDto serverDto, X509Certificate2 cert, RSACryptoServiceProvider rsaKey)
        {
            var soapString = XmlResourceHelper.GetResourceXml("Vmware.Tools.RestSsoAdminSnapIn.Service.xml.SamlTokenByCertificate.xml");
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            string signed = "";
            var dt = DateTime.Now;
            var dtStart = TimeZoneInfo.ConvertTimeToUtc(dt);
            var dtEnd = dtStart.AddMinutes(10);
            string format = "yyyy-MM-ddTHH:mm:ss.fffZ";

            var certString = Convert.ToBase64String(cert.RawData);
            string dtStartStr = dtStart.ToString(format);
            string dtEndStr = dtEnd.ToString(format);
            soapString = string.Format(soapString, dtStartStr, dtEndStr, certString, dtStartStr, dtEndStr);            
            signed = SigningHelper.SignXmlFile(soapString, rsaKey);

            string xml2 = XmlResourceHelper.GetResourceXml("Vmware.Tools.RestSsoAdminSnapIn.Service.xml.SamlTokenByCertificate2.xml");
            xml2 = string.Format(xml2, dtStartStr, dtEndStr, certString, signed, dtStartStr, dtEndStr);

            XmlDocument doc = new XmlDocument();
            doc.PreserveWhitespace = false;
            doc.LoadXml(xml2);

            soapString = doc.InnerXml;
            var customHeaders = new Dictionary<string, string>();
            customHeaders.Add("SOAPAction", "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Issue");
            var headers = ServiceHelper.AddHeaders("text/xml");
            var url = serverDto.Url;
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var token = _webRequestManager.GetResponse(url, requestConfig, headers, customHeaders, soapString);

            XmlDocument doc2 = new XmlDocument();
            doc2.PreserveWhitespace = false;
            doc2.LoadXml(token);
            var node = doc2.GetElementsByTagName("saml2:Assertion")[0];
            if (node != null)
                return node.OuterXml;
            else
                throw new Exception(token);
        }

        public string GetSamlTokenFromGss(ServerDto serverDto, string bet, int lifeTimeSeconds=300)
        {            
            var soapString = XmlResourceHelper.GetResourceXml("Vmware.Tools.RestSsoAdminSnapIn.Service.xml.BETLoginRequest.xml");            
            var dt = DateTime.Now;
            dt = TimeZoneInfo.ConvertTimeToUtc(dt);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var dtEnd = dt.AddSeconds(lifeTimeSeconds);

            string format = "yyyy-MM-ddTHH:mm:ss.fffZ";
            var dt1 = dt.ToString(format);
            var dt2 = dtEnd.ToString(format);

            soapString = string.Format(soapString, dt1, dt2, dt1, dt2, bet);

            var customHeaders = new Dictionary<string, string>();
            customHeaders.Add("SOAPAction", "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Issue");
            var headers = ServiceHelper.AddHeaders("text/xml");
            var url = serverDto.Url;
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var responseFromServer = _webRequestManager.GetResponse(url, requestConfig, headers, customHeaders, soapString);

            XmlDocument doc = new XmlDocument();
            doc.PreserveWhitespace = false;
            using (var reader = new StringReader(responseFromServer))
                doc.Load(reader);
            var node = doc.GetElementsByTagName("saml2:Assertion")[0];
            if (node != null)
                return node.OuterXml;
            else
                throw new Exception(responseFromServer);
        }

        public Token Refresh(ServerDto serverDto, LoginDto loginDto, Token tokenToRefresh)
        {
			var auth = Authenticate (serverDto, loginDto, string.Empty);
			return auth.Token;
        }
        private string WrapInCDATA(string text)
        {
            return string.Format("<![CDATA[{0}]]>", text);
        }
    }
}

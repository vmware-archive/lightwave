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
using System.Net;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.Certificate
{
    public class CertificateService
    {
        private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public CertificateService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
        {
            _webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
        }

        public List<CertificateChainDto> GetCertificates(ServerDto serverDto, string tenantName, CertificateScope scope, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var url = string.Format(_serviceConfigManager.GetCertificatesPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName);
            url += "?scope=" + scope;
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return JsonConvert.Deserialize<List<CertificateChainDto>>(response);
        }

        public bool Delete(ServerDto serverDto, string tenantName, string fingerprint, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var queryString = string.Format("?fingerprint={0}", fingerprint);
            var url = string.Format(_serviceConfigManager.GetCertificatesEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName);
            url = url + queryString;
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Delete
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return string.IsNullOrEmpty(response);
        }

        public bool Create(ServerDto serverDto, string tenantName, CertificateDto certificate, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var url = string.Format(_serviceConfigManager.GetCertificatesEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName);
            var json = JsonConvert.Serialize(certificate);
			json = SerializationJsonHelper.Cleanup (json);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return string.IsNullOrEmpty(response);
        }

        public PrivateKeyDto GetPrivateKey(ServerDto serverDto, string tenantName, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var url = string.Format(_serviceConfigManager.GetCertificatePrivateKeyPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return JsonConvert.Deserialize<PrivateKeyDto>(response);
        }

        public bool SetTenantCredentials(ServerDto serverDto, string tenantName, TenantCredentialsDto credentials, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var url = string.Format(_serviceConfigManager.GetCertificatePrivateKeyEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName);
            var json = JsonConvert.Serialize(credentials);
			json = SerializationJsonHelper.Cleanup (json);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return string.IsNullOrEmpty(response);
        }
    }
}

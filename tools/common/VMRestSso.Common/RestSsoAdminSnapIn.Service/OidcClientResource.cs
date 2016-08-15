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

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.OidcClient
{
    public class OidcClientService
    {
        private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public OidcClientService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
        {
            _webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
        }

        public bool Delete(ServerDto serverDto, string tenant, OidcClientDto oidcClientDto, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var  clientId = Uri.EscapeDataString(oidcClientDto.ClientId);
            var url = string.Format(_serviceConfigManager.GetOidcClientEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, clientId);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Delete,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var result = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return string.IsNullOrEmpty(result);
        }

        public OidcClientDto Create(ServerDto serverDto, string tenant, OidcClientMetadataDto oidcClientMetadataDto, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var json = JsonConvert.Serialize(oidcClientMetadataDto);
            var url = string.Format(_serviceConfigManager.GetOidcClientsEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize<OidcClientDto>(response);
        }
        public OidcClientDto Get(ServerDto serverDto, string tenant, OidcClientDto oidcClientDto, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var clientId = Uri.EscapeDataString(oidcClientDto.ClientId);
            var url = string.Format(_serviceConfigManager.GetOidcClientPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, clientId);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize<OidcClientDto>(response);
        }
        public List<OidcClientDto> GetAll(ServerDto serverDto, string tenant, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetOidcClientsPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize <List<OidcClientDto>>(response);
        }

        public OidcClientDto Update(ServerDto serverDto, string tenant, string clientId, OidcClientMetadataDto oidcClientDto, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            clientId = Uri.EscapeDataString(clientId);
            var json = JsonConvert.Serialize(oidcClientDto);
            var url = string.Format(_serviceConfigManager.GetOidcClientEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, clientId);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Put,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize<OidcClientDto>(response);
        }
    }
}

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

using System.Net;
using System.Collections.Generic;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.Mac
{
    public class ExternalIdentityProviderService : Contracts.IExternalIdentityProviderService
    {
        private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public ExternalIdentityProviderService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
        {
            _webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
        }
        public List<ExternalIdentityProviderDto> GetAll(ServerDto serverDto, string tenantName, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var url = string.Format(_serviceConfigManager.GetExternalIdentityProvidersPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
			return JsonConvert.JsonDeserialize<List<ExternalIdentityProviderDto>> (response);
        }
        public ExternalIdentityProviderDto Get(ServerDto serverDto, string tenantName, ExternalIdentityProviderDto externalIdentityProvider, Token token)
		{
			var name = Uri.EscapeDataString (externalIdentityProvider.EntityID);
			tenantName = Uri.EscapeDataString (tenantName);
			var url = string.Format (_serviceConfigManager.GetExternalIdentityProviderPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName, externalIdentityProvider.EntityID);
			ServicePointManager.ServerCertificateValidationCallback = delegate {
				return true;
			};
			var requestConfig = new RequestSettings {
				Method = HttpMethod.Post,
			};
			var headers = ServiceHelper.AddHeaders (ServiceConstants.JsonContentType);
			var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString ().ToLower ();
			var response = _webRequestManager.GetResponse (url, requestConfig, headers, null, postData);
			return JsonConvert.JsonDeserialize<ExternalIdentityProviderDto> (response);
		}
        public ExternalIdentityProviderDto Create(ServerDto server, string tenantName, ExternalIdentityProviderDto externalIdentityProvider, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var url = string.Format(_serviceConfigManager.GetExternalIdentityProvidersEndPoint(), server.Protocol, server.ServerName, server.Port, tenantName);
            var dto = typeof(ExternalIdentityProviderDto).Assembly;
            //var json = JsonConvert.Serialize(externalIdentityProvider, "root", dto.GetTypes(), true);
            //json = SerializationJsonHelper.Cleanup(json);
			var json = JsonConvert.Serialize(externalIdentityProvider, true);
            ServicePointManager.ServerCertificateValidationCallback = delegate
            {
                return true;
            };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.JsonDeserialize<ExternalIdentityProviderDto>(response);
        }

        public ExternalIdentityProviderDto Create(ServerDto server, string tenantName, string xmlMetadata, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var url = string.Format(_serviceConfigManager.GetExternalIdentityProvidersEndPoint(), server.Protocol, server.ServerName, server.Port, tenantName);

            ServicePointManager.ServerCertificateValidationCallback = delegate
            {
                return true;
            };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.XmlContentType);
            var xml = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + xmlMetadata;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, xml);
            return JsonConvert.JsonDeserialize<ExternalIdentityProviderDto>(response);
        }

        public bool Delete(ServerDto serverDto, string tenant, ExternalIdentityProviderDto externalIdentityProvider, Token token)
        {
            var url = string.Format(_serviceConfigManager.GetExternalIdentityProviderEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, externalIdentityProvider.EntityID);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Delete,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return string.IsNullOrEmpty(response);
        }
        public ExternalIdentityProviderDto Update(ServerDto serverDto, string tenant, ExternalIdentityProviderDto externalIdentityProvider, Token token)
		{
            var url = string.Format(_serviceConfigManager.GetExternalIdentityProviderEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, externalIdentityProvider.EntityID);
			//var json = JsonConvert.JsonSerialize (externalIdentityProvider);
			//json = SerializationJsonHelper.Cleanup (json);
			var json = JsonConvert.Serialize(externalIdentityProvider, true);
			ServicePointManager.ServerCertificateValidationCallback = delegate {
				return true;
			};
			var requestConfig = new RequestSettings {
				Method = HttpMethod.Put,
			};
			var headers = ServiceHelper.AddHeaders (ServiceConstants.JsonContentType);
			json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString ().ToLower () + "&" + json;
			var response = _webRequestManager.GetResponse (url, requestConfig, headers, null, json);
			return JsonConvert.JsonDeserialize<ExternalIdentityProviderDto> (response);
		}
    }
}
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
        public ExternalIdentityProviderService(IWebRequestManager webRequestManager)
        {
            _webRequestManager = webRequestManager;
        }
        public List<ExternalIdentityProviderDto> GetAll(ServerDto serverDto, string tenantName, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var url = string.Format(ServiceConfigManager.GetExternalIdentityProvidersPostEndPoint, serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConfigManager.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            response = CleanupJson(response);
            var dto = typeof(IdentityProviderDto).Assembly;
            return JsonConvert.Deserialize<List<ExternalIdentityProviderDto>>(response, "root", dto.GetTypes(), true);
        }
        public ExternalIdentityProviderDto Get(ServerDto serverDto, string tenantName, ExternalIdentityProviderDto externalIdentityProvider, Token token)
        {
            var name = Uri.EscapeDataString(externalIdentityProvider.EntityID);
            tenantName = Uri.EscapeDataString(tenantName);
            var url = string.Format(ServiceConfigManager.GetExternalIdentityProviderPostEndPoint, serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName, externalIdentityProvider.EntityID);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConfigManager.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            response = SerializationJsonHelper.JsonToDictionary("subjectFormats", response);
            var dto = typeof(ExternalIdentityProviderDto).Assembly;
            return JsonConvert.Deserialize<ExternalIdentityProviderDto>(response, "root", dto.GetTypes(), true);
        }
        public ExternalIdentityProviderDto Create(ServerDto server, string tenantName, ExternalIdentityProviderDto externalIdentityProvider, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var url = string.Format(ServiceConfigManager.ExternalIdentityProvidersEndPoint, server.Protocol, server.ServerName, server.Port, tenantName);
            var dto = typeof(ExternalIdentityProviderDto).Assembly;
            var json = JsonConvert.Serialize(externalIdentityProvider, "root", dto.GetTypes(), true);
            json = SerializationJsonHelper.Cleanup(json);
            json = Cleanup(json);

            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConfigManager.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            response = SerializationJsonHelper.JsonToDictionary("subjectFormats", response);
            return JsonConvert.Deserialize<ExternalIdentityProviderDto>(response, "root", dto.GetTypes(), true);
        }

        public bool Delete(ServerDto serverDto, string tenant, ExternalIdentityProviderDto externalIdentityProvider, Token token)
        {
            var url = string.Format(ServiceConfigManager.ExternalIdentityProviderEndPoint, serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, externalIdentityProvider.EntityID);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Delete,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConfigManager.JsonContentType);
            var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return string.IsNullOrEmpty(response);
        }
        public ExternalIdentityProviderDto Update(ServerDto serverDto, string tenant, ExternalIdentityProviderDto externalIdentityProvider, Token token)
        {
            var url = string.Format(ServiceConfigManager.ExternalIdentityProviderEndPoint, serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, externalIdentityProvider.EntityID);
            var json = JsonConvert.Serialize(externalIdentityProvider);
            json = SerializationJsonHelper.Cleanup(json);
            json = Cleanup(json);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Put,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConfigManager.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            response = SerializationJsonHelper.JsonToDictionary("subjectFormats", response);
            var dto = typeof(IdentityProviderDto).Assembly;
            return JsonConvert.Deserialize<ExternalIdentityProviderDto>(response, "root", dto.GetTypes(), true);
        }

        private string Cleanup(string json)
        {
            var startIndex = json.IndexOf("\"subjectFormats\":");
            var orig = json.Substring(0, startIndex);
            var subStr1 = json.Substring(startIndex);
            var newSubStr = subStr1.Replace("{", string.Empty).Replace("}", string.Empty).Replace("[", "{").Replace("]", "}") + "}";
            return orig + newSubStr;
        }
        static string CleanupJson(string response)
        {
            var startIndex = 0;
            var subStr = response;
            while (startIndex >= 0)
            {
                startIndex = subStr.IndexOf("\"subjectFormats\":");
                if (startIndex < 0)
                    break;
                var endIndex = subStr.Substring(startIndex).IndexOf("\"jitEnabled");
                if (endIndex < 0)
                    break;
                var subStr1 = subStr.Substring(startIndex, endIndex);
                subStr = subStr.Replace(subStr1, string.Empty);
            }
            return subStr;
        }
    }
}
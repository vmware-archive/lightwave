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
using System.Reflection;
using System.Text.RegularExpressions;
using System.Linq;
using System;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.Tenant
{
    public class RelyingPartyService
    {
        private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public RelyingPartyService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
        {
            _webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
        }
        public List<RelyingPartyDto> GetAll(ServerDto serverDto, string tenant, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetRelyingPartysPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return JsonConvert.Deserialize<List<RelyingPartyDto>>(response);
        }
        public RelyingPartyDto Get(ServerDto serverDto, string tenant, RelyingPartyDto relyingParty, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var rp = Uri.EscapeDataString(relyingParty.Name);
            var url = string.Format(_serviceConfigManager.GetRelyingPartyPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, rp);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
                                    {
                                        Method = HttpMethod.Post,
                                    };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return JsonConvert.Deserialize<RelyingPartyDto>(response);
        }
        public RelyingPartyDto Create(ServerDto server, string tenant, RelyingPartyDto relyingParty, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetRelyingPartysEndPoint(), server.Protocol, server.ServerName, server.Port, tenant);
            var json = JsonConvert.Serialize(relyingParty);
			json = SerializationJsonHelper.Cleanup (json);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize<RelyingPartyDto>(response);
        }
        public bool Delete(ServerDto serverDto, string tenant, RelyingPartyDto relyingParty, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var rp = Uri.EscapeDataString(relyingParty.Name);
            var url = string.Format(_serviceConfigManager.GetRelyingPartyEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, rp);
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
        public RelyingPartyDto Update(ServerDto serverDto, string tenant, RelyingPartyDto relyingParty, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var rp = Uri.EscapeDataString(relyingParty.Name);
            var url = string.Format(_serviceConfigManager.GetRelyingPartyEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, rp);
			var dto = typeof(RelyingPartyDto).Assembly;
			var json = JsonConvert.Serialize(relyingParty, "root", dto.GetTypes(), true);
			json = SerializationJsonHelper.Cleanup (json);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Put,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize<RelyingPartyDto>(response);
        }
    }
}
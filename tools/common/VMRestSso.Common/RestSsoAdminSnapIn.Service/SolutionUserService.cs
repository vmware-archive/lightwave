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

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.SolutionUser
{
    public class SolutionUserService
    {
        private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public SolutionUserService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
        {
            _webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
        }

        public SolutionUserDto Create(ServerDto serverDto, string tenantName, SolutionUserDto userDto, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var name = Uri.EscapeDataString(userDto.Name);
            var url = string.Format(_serviceConfigManager.GetSolutionUsersEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName, name);
            var json = JsonConvert.Serialize(userDto);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response =  _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize<SolutionUserDto>(response);
        }
        public SolutionUserDto Get(ServerDto serverDto, string tenantName, SolutionUserDto user, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var name = Uri.EscapeDataString(user.Name);
            var url = string.Format(_serviceConfigManager.GetSolutionUserPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName, name);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return JsonConvert.Deserialize<SolutionUserDto>(response);
        }

        public bool Delete(ServerDto serverDto, string tenantName, SolutionUserDto user, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var name = Uri.EscapeDataString(user.Name);
            var url = string.Format(_serviceConfigManager.GetSolutionUserEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName, name);
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

        public SolutionUserDto Update(ServerDto serverDto, string tenantName, SolutionUserDto userDto, Token token)
        {
            tenantName = Uri.EscapeDataString(tenantName);
            var name = Uri.EscapeDataString(userDto.Name);
            var url = string.Format(_serviceConfigManager.GetSolutionUserEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName, name);
            var json = JsonConvert.Serialize(userDto);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Put
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize<SolutionUserDto>(response);
        }
    }
}
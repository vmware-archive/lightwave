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

using System.Collections.Generic;
using System.Linq;
using System.Net;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.Adf
{
    public class AdfService
    {
        private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public AdfService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
        {
            _webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
        }

        public ActiveDirectoryJoinInfoDto GetActiveDirectory(ServerDto server, Token token)
        {
            var url = string.Format(_serviceConfigManager.GetPostAdfEndPoint(), server.Protocol, server.ServerName, server.Port);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return JsonConvert.Deserialize<ActiveDirectoryJoinInfoDto>(response);
        }

        public ActiveDirectoryJoinInfoDto JoinActiveDirectory(ServerDto server, ActiveDirectoryJoinRequestDto ad, Token token)
        {
            var url = string.Format(_serviceConfigManager.GetAdfEndPoint(), server.Protocol, server.ServerName, server.Port);
            var json = JsonConvert.Serialize(ad);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize<ActiveDirectoryJoinInfoDto>(response);
        }

        public bool LeaveActiveDirectory(ServerDto serverDto, CredentialsDto credentialsDto, Token token)
        {
            var url = string.Format(_serviceConfigManager.GetAdfEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port);
            var json = JsonConvert.Serialize(credentialsDto);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Delete,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return string.IsNullOrEmpty(response);
        }
    }
}

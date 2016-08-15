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

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.Server
{
    public class ServerService
    {
        private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public ServerService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
        {
            _webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
        }

        public ServerInfoDto GetServerInfo(ServerDto server, Token token)
        {
            var url = string.Format(_serviceConfigManager.GetServerAboutInfoEndPoint(), server.Protocol, server.ServerName, server.Port);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Get,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            headers.Add(HttpRequestHeader.Authorization, "Bearer " + token.AccessToken);
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, null);
            var serverInfo = JsonConvert.Deserialize<ServerInfoDto>(response);
            return serverInfo;
        } 
        public IList<ComputerDto> GetComputers(ServerDto server, Token token)
        {
            var url = string.Format(_serviceConfigManager.GetServerComputersPostEndPoint(), server.Protocol, server.ServerName, server.Port);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            var computers = JsonConvert.Deserialize<List<ComputerDto>>(response);
            return computers.Select(computer => new ComputerDto { DomainController = computer.DomainController, HostName = computer.HostName }).ToList();
        }

        public string GetStatus(ServerDto server, Token token)
        {
            var url = string.Format(_serviceConfigManager.GetServerStatusPostEndPoint(), server.Protocol, server.ServerName, server.Port);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders();
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return response;
        }
    }
}
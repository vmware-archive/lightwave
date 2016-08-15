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
using System.Linq;
using System.Net;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service
{
    public class SuperLoggingService
    {
        private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public SuperLoggingService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
        {
            _webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
        }
        public bool Start(ServerDto server, string tenant,  Token token, int size=500)
        {
            var url = string.Format(_serviceConfigManager.GetStartEventLogPostEndPoint(), server.Protocol, server.ServerName, server.Port, tenant, size);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return string.IsNullOrEmpty(response);
        }

        public bool Stop(ServerDto server, string tenant,  Token token)
        {
            var url = string.Format(_serviceConfigManager.GetStopEventLogPostEndPoint(), server.Protocol, server.ServerName, server.Port, tenant);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
           return string.IsNullOrEmpty(response);
        }

        public bool Delete(ServerDto serverDto, string tenant, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetEventLogEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
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

        public List<EventLogDto> GetEventLogs(ServerDto server,  string tenant, Token token)
        {
            var url = string.Format(_serviceConfigManager.GetEventLogPostEndPoint(), server.Protocol, server.ServerName, server.Port, tenant);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            var eventlogs = JsonConvert.JsonDeserialize<List<EventLogDto>>(response);
            return eventlogs;
        }

        public EventLogStatusDto GetStatus(ServerDto server, string tenant, Token token)
        {
            var url = string.Format(_serviceConfigManager.GetStatusEventLogPostEndPoint(), server.Protocol, server.ServerName, server.Port, tenant);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            var eventlogStatus = JsonConvert.JsonDeserialize<EventLogStatusDto>(response);
            return eventlogStatus;
        }
    }
}

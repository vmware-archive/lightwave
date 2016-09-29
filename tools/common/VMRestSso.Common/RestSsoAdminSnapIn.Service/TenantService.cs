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
using System.Linq;
using System.Collections.Generic;
using System.Net;
using System.Text;
using System.Reflection;
using System.Text.RegularExpressions;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.Tenant
{
    public class TenantService
    {
        private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public TenantService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
        {
            _webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
        }

        public TenantDto Get(ServerDto serverDto, string tenant, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetTenantPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
                                    {
                                        Method = HttpMethod.Post,
                                    };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return JsonConvert.Deserialize<TenantDto>(response);
        }

        public TenantDto Create(ServerDto server, TenantDto tenant, Token token)
        {
            tenant.Username = tenant.Username + "@" + tenant.Name;
            var url = string.Format(_serviceConfigManager.GetTenantsEndPoint(), server.Protocol, server.ServerName, server.Port);
            var json = JsonConvert.Serialize(tenant);
			json = Cleanup (json);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize<TenantDto>(response);
        }
        
        public bool Delete(ServerDto serverDto, string tenant, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetTenantEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
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

        public TenantConfigurationDto GetConfig(ServerDto serverDto, string tenant, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetTenantConfigPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return JsonConvert.Deserialize<TenantConfigurationDto>(response);
        }

        public TenantConfigurationDto GetConfig(ServerDto serverDto, string tenant, Token token, TenantConfigType type)
        {
            tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetTenantConfigPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
            url += "?type=" + type;
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return JsonConvert.Deserialize<TenantConfigurationDto>(response);
        }

        public TenantConfigurationDto UpdateConfig(ServerDto serverDto, string tenant, TenantConfigurationDto tenantConfig, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetTenantConfigEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
            var json = JsonConvert.Serialize(tenantConfig);
			json = Cleanup (json);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Put,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize<TenantConfigurationDto>(response);
        }

        public TenantConfigurationDto UpdatePasswordAndLockoutConfig(ServerDto serverDto, string tenant, TenantConfigurationDto tenantConfig, Token token)
        {
            tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetPasswordAndLockoutPolicyEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
            var json = JsonConvert.Serialize(tenantConfig);
            json = Cleanup(json);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Put,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize<TenantConfigurationDto>(response);
        }

        public TenantConfigurationDto UpdateConfig(ServerDto serverDto, string tenant, TenantConfigurationDto tenantConfig, Token token, TenantConfigType type)
        {
            tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetTenantConfigEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
            url += "?type=" + type;
            var json = JsonConvert.Serialize(tenantConfig);
			json = Cleanup (json);
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Put,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.Deserialize<TenantConfigurationDto>(response);
        }

        public GroupMembershipDto Search(ServerDto serverDto, string tenant, string domain, MemberType memberType, SearchType searchType, Token token, string searchString = null, int limit = 100)
        {
            tenant = Uri.EscapeDataString(tenant);
            var searchValue = string.IsNullOrEmpty(searchString) ? string.Empty :  Uri.EscapeDataString(searchString);
            var queryString = string.Format("?domain={0}&limit={1}&type={2}&searchBy={3}&query={4}", domain, limit, memberType, searchType, searchValue);
            var url = string.Format(_serviceConfigManager.GetTenantSearchPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
            url = url + queryString;
            ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
            var requestConfig = new RequestSettings
            {
                Method = HttpMethod.Post,
            };
            var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
            var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
            var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return JsonConvert.Deserialize<GroupMembershipDto>(response);
        }

		private string Cleanup(string json)
		{
			int start = 0;
			var legnth = "RestSsoAdminSnapIn".Length;
			do {
				start = FirstIndexOf (json, "__type");
				if (start != -1) {
					var end = FirstIndexOf (json, "RestSsoAdminSnapIn");
					if (end != -1) {
						var left = json.Substring (0, start - 1);
						var right = (((end + legnth + 6) < json.Length) ? json.Substring (end + legnth + 6) : string.Empty);
						json =  left + right;
					}
				} else
					break;
			} while (start != -1);
			return json;
		}

		private int FirstIndexOf( string str, string value) {
			Match match = Regex.Matches(str, value).Cast<Match>().FirstOrDefault();
			return match != null ? match.Index : -1;
		}
    }

    public enum TenantConfigType
    {
        LOCKOUT,
        PASSWORD,
        TOKEN,
        PROVIDER
    }
}
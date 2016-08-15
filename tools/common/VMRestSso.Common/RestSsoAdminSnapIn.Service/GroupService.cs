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

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.Group
{
	public class GroupService
	{
		private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public GroupService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
		{
			_webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
		}

		public GroupDto Get(ServerDto serverDto, string tenantName, string group, Token token)
		{
			group = Uri.EscapeDataString(group);
			tenantName = Uri.EscapeDataString(tenantName);
			var url = string.Format(_serviceConfigManager.GetGroupPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName, group);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Post,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
			return JsonConvert.Deserialize<GroupDto>(response);
		}

		public GroupDto Create(ServerDto serverDto, string tenant, GroupDto groupDto, Token token)
		{
			var principalName = Uri.EscapeDataString(groupDto.GroupName + "@" + groupDto.GroupDomain);
			tenant = Uri.EscapeDataString(tenant);
			var json = JsonConvert.Serialize(groupDto);
            var url = string.Format(_serviceConfigManager.GetGroupsEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, principalName);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Post,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
			return JsonConvert.Deserialize<GroupDto>(response);
		}

		public bool Delete(ServerDto serverDto, string tenant, GroupDto groupDto, Token token)
		{
			var principalName = Uri.EscapeDataString(groupDto.GroupName + "@" + groupDto.GroupDomain);
			tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetGroupEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, principalName);
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
		public GroupDto Update(ServerDto serverDto, string tenant, GroupDto groupDto, Token token)
		{
			var principalName = Uri.EscapeDataString(groupDto.GroupName + "@" + groupDto.GroupDomain);
			tenant = Uri.EscapeDataString(tenant);
			var url = string.Format(_serviceConfigManager.GetGroupEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, principalName);
			var g = new GroupDto() { GroupDetails = new GroupDetailsDto { Description = groupDto.GroupDetails.Description } };
			var json = JsonConvert.Serialize(g);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Put,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
			return JsonConvert.Deserialize<GroupDto>(response);
		}

		public bool AddUsers(ServerDto serverDto, string tenant, GroupDto groupDto, IList<UserDto> users, Token token)
		{
			var principalName = Uri.EscapeDataString(groupDto.GroupName + "@" + groupDto.GroupDomain);
			tenant = Uri.EscapeDataString(tenant);
			var queryString = users.Select(x => "members=" + Uri.EscapeDataString(x.Name + "@" + x.Domain)).Aggregate((x, y) => string.Format("{0}&{1}", x, y));
            var url = string.Format(_serviceConfigManager.GetMembersOfGroupEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, principalName, queryString, GroupMemberType.USER);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Put,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
			return response != null;
		}
		public bool AddSolutionUsers(ServerDto serverDto, string tenant, GroupDto groupDto, IList<SolutionUserDto> users, Token token)
		{
			var principalName = Uri.EscapeDataString(groupDto.GroupName + "@" + groupDto.GroupDomain);
			tenant = Uri.EscapeDataString(tenant);
			var queryString = users.Select(x => "members=" + x.Name + "@" + x.Domain).Aggregate((x, y) => string.Format("{0}&{1}", x, y));
            var url = string.Format(_serviceConfigManager.GetMembersOfGroupEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, principalName, queryString, GroupMemberType.SOLUTIONUSER);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Put,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
			return response != null;
		}

		public bool AddGroups(ServerDto serverDto, string tenant, GroupDto groupDto, IList<GroupDto> groups, Token token)
		{
			var principalName = Uri.EscapeDataString(groupDto.GroupName + "@" + groupDto.GroupDomain);
			tenant = Uri.EscapeDataString(tenant);
			var queryString = groups.Select(x => "members=" + x.GroupName + "@" + x.GroupDomain).Aggregate((x, y) => string.Format("{0}&{1}", x, y));
            var url = string.Format(_serviceConfigManager.GetMembersOfGroupEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, principalName, queryString, GroupMemberType.GROUP);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Put,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
			return response != null;
		}

		public GroupMembershipDto GetMembers(ServerDto serverDto, string tenant, GroupDto groupDto, GroupMemberType type, Token token)
		{
			var principalName = Uri.EscapeDataString(groupDto.GroupName + "@" + groupDto.GroupDomain);
			tenant = Uri.EscapeDataString(tenant);
			var url = string.Format(_serviceConfigManager.GetAllMembersOfGroupPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, principalName, type);
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

		public bool RemoveUsers(ServerDto serverDto, string tenant, GroupDto groupDto, IList<UserDto> users, Token token)
		{
			var principalName = Uri.EscapeDataString(groupDto.GroupName + "@" + groupDto.GroupDomain);
			tenant = Uri.EscapeDataString(tenant);
			var queryString = users.Select(x => "members=" + x.Name + "@" + x.Domain).Aggregate((x, y) => string.Format("{0}&{1}", x, y));
            var url = string.Format(_serviceConfigManager.GetMembersOfGroupEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, principalName, queryString, GroupMemberType.USER);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Delete,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
			return response != null;
		}

		public bool RemoveSolutionUsers(ServerDto serverDto, string tenant, GroupDto groupDto, IList<SolutionUserDto> users, Token token)
		{
			var principalName = Uri.EscapeDataString(groupDto.GroupName + "@" + groupDto.GroupDomain);
			tenant = Uri.EscapeDataString(tenant);
			var queryString = users.Select(x => "members=" + x.Name + "@" + x.Domain).Aggregate((x, y) => string.Format("{0}&{1}", x, y));
            var url = string.Format(_serviceConfigManager.GetMembersOfGroupEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, principalName, queryString, GroupMemberType.SOLUTIONUSER);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Delete,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
			return response != null;
		}

		public bool RemoveGroups(ServerDto serverDto, string tenant, GroupDto groupDto, IList<GroupDto> groups, Token token)
		{
			var principalName = Uri.EscapeDataString(groupDto.GroupName + "@" + groupDto.GroupDomain);
			tenant = Uri.EscapeDataString(tenant);
			var queryString = groups.Select(x => "members=" + x.GroupName + "@" + x.GroupDomain).Aggregate((x, y) => string.Format("{0}&{1}", x, y));
            var url = string.Format(_serviceConfigManager.GetMembersOfGroupEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, principalName, queryString, GroupMemberType.GROUP);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Delete,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
			return response != null;
		}

		public List<GroupDto> GetParents(ServerDto serverDto, string tenant, GroupDto groupDto, Token token)
		{
			var principalName = Uri.EscapeDataString(groupDto.GroupName + "@" + groupDto.GroupDomain);
			tenant = Uri.EscapeDataString(tenant);
			var url = string.Format(_serviceConfigManager.GetParentsOfGroupPostEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant, principalName);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Post,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
			return JsonConvert.Deserialize<List<GroupDto>>(response);
		}
	}
}
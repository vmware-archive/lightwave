/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

using AppKit;
using Foundation;
using System;
using System.Collections.Generic;
using RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Tenant;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;


using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Nodes
{
	public class GroupsNode : ScopeNode
	{
		private readonly string _tenantName;
		private readonly string _domainName;
		private readonly bool _isSystemDomain;
		private readonly ServerDto _serverDto;
		public string TenantName { get { return _tenantName; } }
		public string DomainName { get { return _domainName; } }
		public bool IsSystemDomain { get { return _isSystemDomain; } }
		public ServerDto ServerDto { get { return _serverDto; } }

		public GroupsNode(ServerDto dto, string tenantName, string domainName, bool systemDomain)
		{
			_tenantName = tenantName;
			_isSystemDomain = systemDomain;
			_serverDto = dto;
			_domainName = domainName;            
			DisplayName = "Groups";
		}

		public void AddNewGroup (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var form = new AddGroupController ();
				var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (result == VMIdentityConstants.DIALOGOK) {
					var user = AddNewGroup (form.GroupDto);
					if (user != null) {
						UIErrorHelper.ShowAlert ("Group " + user.GroupName + " created successfully", "Information");
						Refresh (sender, e);
					}
				}
			});
		}
		private GroupDto AddNewGroup (GroupDto userDto)
		{
			userDto.GroupDomain = _domainName;
			var serverDto = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto.ServerName);
			return SnapInContext.Instance.ServiceGateway.Group.Create(_serverDto, _tenantName, userDto, serverDto.Token);
		}
		public override string GetDisplayTitle ()
		{
			return string.Format ("{0} -> {1} -> {2}", Parent.Parent.Parent.Parent.Parent.DisplayName, Parent.Parent.DisplayName, DisplayName);
		}
		public List<GroupDto> GetGroups(string searchString)
		{
			var groups = new List<GroupDto> ();
			ActionHelper.Execute (delegate() {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (_serverDto.ServerName);
				var service = SnapInContext.Instance.ServiceGateway;
				var dto = service.Tenant.Search (_serverDto, _tenantName, _domainName, MemberType.GROUP, SearchType.NAME, auth.Token, searchString);
				groups = dto.Groups;
			});
			return groups;
		}

		public void DeleteGroup(GroupDto groupDto)
		{
			ActionHelper.Execute (delegate() {
				var serverDto = SnapInContext.Instance.AuthTokenManager.GetAuthToken (_serverDto.ServerName);
				var success = SnapInContext.Instance.ServiceGateway.Group.Delete (_serverDto, _tenantName, groupDto, serverDto.Token);
				if (success) {
					UIErrorHelper.ShowAlert ("Group " + groupDto.GroupName + " deleted successfully", "Information");
				} else {

					UIErrorHelper.ShowAlert ("Failed to delete Group " + groupDto.GroupName, "Information");
				}
				Refresh (this, EventArgs.Empty);
			});
		}
	}
}


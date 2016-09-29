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
	public class SolutionUsersNode : ScopeNode
	{
		private readonly string _tenantName;
		private readonly string _domainName;
		private readonly bool _systemDomain;
		private readonly ServerDto _serverDto;

		public string TenantName { get { return _tenantName; } }
		public string DomainName { get { return _domainName; } }
		public bool IsSystemDomain { get { return _systemDomain; } }
		public ServerDto ServerDto { get { return _serverDto; } }

		public SolutionUsersNode(ServerDto serverDto, string tenantName, string name, bool systemDomain)
		{
			_domainName = name;
			_systemDomain = systemDomain;
			_serverDto = serverDto;
			_tenantName = tenantName;
			DisplayName = "Solution Users";
		}

		public void AddNewUser (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var form = new AddSolutionUserController ();
				var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (result == VMIdentityConstants.DIALOGOK) {
					var user = AddUser (form.SolutionUserDto);
					if (user != null) {
						UIErrorHelper.ShowAlert ("Solution User " + user.Name + " created successfully", "Information");
						Refresh (sender, e);
					}
				}
			});
		}

		private SolutionUserDto AddUser (SolutionUserDto userDto)
		{
			userDto.Domain = _domainName;
			var serverDto = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto.ServerName);
			return SnapInContext.Instance.ServiceGateway.SolutionUser.Create(_serverDto, _tenantName, userDto, serverDto.Token);
		}

		public override string GetDisplayTitle ()
		{
			return string.Format ("{0} -> {1} -> {2}", Parent.Parent.Parent.Parent.Parent.DisplayName, Parent.Parent.DisplayName, DisplayName);
		}

		public List<SolutionUserDto> GetUsers(string searchString)
		{
			var list = new List<SolutionUserDto> ();
			ActionHelper.Execute (delegate() {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (_serverDto.ServerName);
				var service = SnapInContext.Instance.ServiceGateway;
				var dto = service.Tenant.Search (_serverDto, _tenantName, _domainName, MemberType.SOLUTIONUSER, SearchType.NAME, auth.Token, searchString);
				list = dto.SolutionUsers;
			});
			return list;
		}

		public void DeleteUser (SolutionUserDto userDto)
		{
			ActionHelper.Execute (delegate() {
				userDto.Domain = _domainName;
				var serverDto = SnapInContext.Instance.AuthTokenManager.GetAuthToken (_serverDto.ServerName);
				var success = SnapInContext.Instance.ServiceGateway.SolutionUser.Delete (_serverDto, _tenantName, userDto, serverDto.Token);
				if (success) {
					UIErrorHelper.ShowAlert ("Solution User " + userDto.Name + " deleted successfully", "Information");
				} else {

					UIErrorHelper.ShowAlert ("Failed to delete Solution User " + userDto.Name, "Information");
				}
				Refresh (this, EventArgs.Empty);
			});
		}
	}

}


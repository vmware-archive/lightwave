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
using System.IO;
using System.Net;
using System.Linq;
using System.Collections.Generic;
using RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Tenant;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Nodes
{
	public class UsersNode : ScopeNode
	{
		private readonly string _domainName;
		private readonly string _tenantName;
		private readonly string _systemDomain;
		private readonly bool _isSystemDomain;
		private readonly ServerDto _serverDto;

		public string TenantName { get { return _tenantName; } }
		public string DomainName { get { return _domainName; } }
		public string SystemDomain { get { return _systemDomain; } }
		public bool IsSystemDomain { get { return _isSystemDomain; } }
		public ServerDto ServerDto { get { return _serverDto; } }

		public UsersNode(ServerDto dto, string tenantName, string domainName, bool systemDomain)
		{
			_serverDto = dto;
			_tenantName = tenantName;
			_domainName = domainName;
			_systemDomain = tenantName;
			_isSystemDomain = systemDomain;
			DisplayName = "Users";
		}

		public void AddNewUser (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var form = new AddUserController ();
				var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (result == VMIdentityConstants.DIALOGOK) {
					try {
						var user = AddUser (form.UserDto);
						if (user != null) {
							UIErrorHelper.ShowAlert ("User " + user.Name + " created successfully", "Information");
							Refresh (sender, e);
						}
					} catch (WebException exp) {
						if (exp.Response is HttpWebResponse) {
							var response = exp.Response as HttpWebResponse;
							if (response != null && response.StatusCode == HttpStatusCode.BadRequest && response.ContentType == "application/json") {
								var resp = new StreamReader (response.GetResponseStream ()).ReadToEnd ();
								var error = JsonConvert.Deserialize<AuthErrorDto> (resp);
								if(error.Cause == "Constraint violation")
								{
									UIErrorHelper.ShowAlert ("Password does not match the password policy set on the tenant. Check tenant configuration for password policy or contact administrator", "Error");
								}
							} else {
								throw exp;
							}
						}
					}
				}
			});
		}
		public override string GetDisplayTitle ()
		{
			return string.Format ("{0} -> {1} -> {2}", Parent.Parent.Parent.Parent.Parent.DisplayName, Parent.Parent.DisplayName, DisplayName);
		}
		public List<UserDto> GetUsers(string searchString)
		{
			var list = new List<UserDto> ();
			ActionHelper.Execute (delegate() {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (_serverDto.ServerName);
				var service = SnapInContext.Instance.ServiceGateway;
				var dto = service.Tenant.Search (_serverDto, _tenantName, _domainName, MemberType.USER, SearchType.NAME, auth.Token, searchString);
				list = dto.Users;
			});
			return list;
		}
		private UserDto AddUser (UserDto userDto)
		{	
			userDto.Domain = _systemDomain;
			userDto.Alias = new PrincipalDto {
				Name = userDto.Name,
				Domain = userDto.Domain
			};
			userDto.PersonDetails.UserPrincipalName = string.Format("{0}@{1}", userDto.Name, _systemDomain);
			var serverDto = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto.ServerName);
			return SnapInContext.Instance.ServiceGateway.User.Create(_serverDto, _tenantName, userDto, serverDto.Token);
		}
		public void DeleteUser (UserDto userDto)
		{
			ActionHelper.Execute (delegate() {
				userDto.PersonDetails.UserPrincipalName = string.Format ("{0}@{1}", userDto.Name, _systemDomain);
				var serverDto = SnapInContext.Instance.AuthTokenManager.GetAuthToken (_serverDto.ServerName);
				var result = SnapInContext.Instance.ServiceGateway.User.Delete (_serverDto, _tenantName, userDto, serverDto.Token);
				var success = string.IsNullOrEmpty (result);
				if (success) {
					UIErrorHelper.ShowAlert ("User " + userDto.Name + " deleted successfully", "Information");
				} else {

					UIErrorHelper.ShowAlert ("Failed to delete User " + userDto.Name, "Information");
				}
				Refresh (this, EventArgs.Empty);
			});
		}
	}
}


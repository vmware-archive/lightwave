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

using System;
using System.Collections.Generic;
using AppKit;
using Foundation;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

using RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Nodes
{
	public class TenantNode : ScopeNode
	{
		private ServerDto _serverDto;
		private string _tenantName;
		public bool Active;
		public bool IsSystemTenant;
		public TenantNode(ServerDto serverDto, string tenantName)
		{
			_serverDto = serverDto;
			_tenantName = tenantName;
			Active = true;
			IsSystemTenant = false;
		}

		public void Delete (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate { 
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (_serverDto.ServerName);
				var service = SnapInContext.Instance.ServiceGateway;
				service.Tenant.Delete (auth.ServerDto, DisplayName, auth.Token);

				var parent = ((ServerNode)Parent);
				for (var i = 0; i < parent.Children.Count; i++) {
					if (parent.Children [i].DisplayName == DisplayName) {
						parent.Children.RemoveAt (i);
						break;
					}
				}
				Active = false;
			});
		}

		public void ShowConfiguration (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto.ServerName);
				var tenantConfig = SnapInContext.Instance.ServiceGateway.Tenant.GetConfig(_serverDto, DisplayName, auth.Token);
				var frm = new ShowTenantConfigurationController
				{
					TenantConfigurationDto = tenantConfig,
					ServerDto = _serverDto,
					TenantName = DisplayName
				};
				NSApplication.SharedApplication.RunModalForWindow (frm.Window);
				NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
			});
		}

		public override void Refresh (object sender, EventArgs e)
		{
			Refresh ();
		}

		public void Refresh(){
			Children.Clear ();
			var identitySourceNode = new IdentitySourcesNode (_serverDto, _tenantName){ Parent = this };
			var roles = new List<String>(){"Administrator", "RegularUser"};
			var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto.ServerName);
			var role = auth.Token.Role;
			if (roles.Contains(role)) {
				identitySourceNode.Refresh (this, EventArgs.Empty);
			}
			Children.Add (identitySourceNode);
			Children.Add (new IdentityProvidersNode (){ Parent = this });
			Children.Add (new RelyingPartyNode (){ Parent = this });
			Children.Add (new OidcClientNode (){ Parent = this });
			Children.Add (new ServerCertificatesNode (_serverDto, _tenantName){ Parent = this });
			NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
		}

		public override string GetDisplayTitle()
		{
			return string.Format ("{0} -> {1}", Parent.DisplayName, DisplayName);
		}
	}
}


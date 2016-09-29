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
using System.Linq;
using System.Collections.Generic;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Service.IdentityProvider;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Nodes
{
	public class ExternalDomainNode : ScopeNode
	{
		private IdentityProviderDto _provider;
		private string _tenantName;

		public ExternalDomainNode (string tenantName, IdentityProviderDto provider)
		{
			_tenantName = tenantName;
			_provider = provider;            
			this.DisplayName = _provider.Name;
		}

		public void View(object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var serverDto = GetServerDTO ();
				var service = SnapInContext.Instance.ServiceGateway;
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				var dto = service.IdentityProvider.Get (serverDto, _tenantName, _provider.Name, auth.Token);
				var form = new AddExternalIdentitySourceController {
					ServerDto = serverDto,
					TenantName = _tenantName,
					IdentityProviderDto = dto
				};
				NSApplication.SharedApplication.RunModalForWindow (form.Window);
			});
		}
		public override void Refresh (object sender, EventArgs e)
		{
			this.Children.Clear ();
			var serverDto = GetServerDTO ();
			var userAndgroupNode = new UsersAndGroupsNode (serverDto, _tenantName, _provider, false){ Parent = this };
			this.Children.Add (userAndgroupNode);
			NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
		}

		public void Delete (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var serverDto = GetServerDTO ();
				var service = SnapInContext.Instance.ServiceGateway;
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				if (service.IdentityProvider.Delete (serverDto, _tenantName, _provider.Name, auth.Token)) {
					Parent.Children.Remove (this);
					NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this.Parent);
				}
			});
		}

		private ServerDto GetServerDTO ()
		{            
			var dto = Parent.Parent.Parent.Parent.Tag as AuthTokenDto;
			return dto != null ? dto.ServerDto : null;
		}

		public override string GetDisplayTitle ()
		{
			return string.Format ("{0} -> {1} -> {2}", Parent.Parent.Parent.Parent.DisplayName, Parent.DisplayName, DisplayName);
		}
	}
}


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
using RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Tenant;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

	
using Vmware.Tools.RestSsoAdminSnapIn.Service.IdentityProvider;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Nodes
{
	public class ExternalDomainsNode : ScopeNode
	{
		private string _tenantName;

		public ExternalDomainsNode (string tenantName, List<IdentityProviderDto> domains)
		{
			DisplayName = "External Domains";
			_tenantName = tenantName;
		}

		public override void Refresh (object sender, EventArgs e)
		{
			this.Children.Clear ();
			ActionHelper.Execute (delegate() {
				var service = SnapInContext.Instance.ServiceGateway;               
				var serverDto = GetServerDto ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				var identityProviders = service.IdentityProvider.GetAll (serverDto, _tenantName, auth.Token);
				var externalProviders = identityProviders.Where (x => x.DomainType == DomainType.EXTERNAL_DOMAIN.ToString ()).ToList ();
				PopulateChildren (externalProviders);
			});

		}

		public void AddNewExternalDomain (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var form = new AddExternalIdentitySourceController {
					ServerDto = GetServerDto (),
					TenantName = _tenantName
				};
				var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (result == VMIdentityConstants.DIALOGOK) {
					UIErrorHelper.ShowAlert ("Domain " + form.IdentityProviderDto.Name + " created successfully", "Information");
					Refresh (sender, e);
				}
			});
		}

		public void PopulateChildren (List<IdentityProviderDto> providers)
		{
			if (providers != null) {
				foreach (var provider in providers) {
					var node = new ExternalDomainNode (_tenantName, provider){ Parent = this };
					node.Refresh (this, EventArgs.Empty);
					Children.Add (node);
				}
				NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this.Parent);
			}
		}

		private ServerDto GetServerDto ()
		{
			var dto = Parent.Parent.Parent.Tag as AuthTokenDto;
			return dto != null ? dto.ServerDto : null;
		}

		public override string GetDisplayTitle ()
		{
			return string.Format ("{0} -> {1} -> {2}", Parent.Parent.Parent.DisplayName, Parent.DisplayName, DisplayName);
		}
	}
}

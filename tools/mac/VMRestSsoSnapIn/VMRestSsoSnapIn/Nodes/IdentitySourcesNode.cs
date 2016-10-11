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
using System.Linq;
using AppKit;
using Foundation;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Tenant;
using Vmware.Tools.RestSsoAdminSnapIn.Service.IdentityProvider;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Nodes
{
	public class IdentitySourcesNode : ScopeNode
	{
		private ServerDto _serverDto;
		private string _tenantName;

		public IdentitySourcesNode (ServerDto serverDto, string tenantName)
		{
			_serverDto = serverDto;
			_tenantName = tenantName;
			DisplayName = "Identity Sources";
		}

		public override void Refresh (object sender, EventArgs e)
		{
			Children.Clear ();
			ActionHelper.Execute (delegate() {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (_serverDto.ServerName);
				var service = SnapInContext.Instance.ServiceGateway;
				var identityProviders = service.IdentityProvider.GetAll (_serverDto, _tenantName, auth.Token);
				var tenantConfig = new TenantConfigurationDto ();
				try{
					tenantConfig = service.Tenant.GetConfig (_serverDto, _tenantName, auth.Token, TenantConfigType.PROVIDER);
				} catch(Exception exc)
				{
					// do nothing
				}
				var systemDomains = identityProviders.Where (x => x.DomainType == DomainType.SYSTEM_DOMAIN.ToString ());
				if (systemDomains != null) {
					foreach (var provider in systemDomains) {
						var systemDomain = new IdentitySourceNode (provider, _tenantName, true, provider.Name + " (System Domain)", tenantConfig){ Parent = this };
						systemDomain.Refresh (this, EventArgs.Empty);
						Children.Add (systemDomain);
					}
				}

				var localOsDomains = identityProviders.Where (x => x.DomainType == DomainType.LOCAL_OS_DOMAIN.ToString ());
				((TenantNode)this.Parent).IsSystemTenant = (localOsDomains != null && localOsDomains.Count() > 0);

				if (localOsDomains != null) {
					foreach (var provider in localOsDomains) {
						var localOsDomain = new IdentitySourceNode (provider, _tenantName, false, provider.Name + " (Local OS Domain)", tenantConfig){ Parent = this };
						localOsDomain.Refresh (this, EventArgs.Empty);
						Children.Add (localOsDomain);
					}
				}

				var externalDomains = identityProviders.Where (x => x.DomainType == DomainType.EXTERNAL_DOMAIN.ToString ()).ToList ();
				var externalDomainNode = new ExternalDomainsNode (_tenantName, externalDomains){ Parent = this };
				externalDomainNode.PopulateChildren(externalDomains);
				Children.Add (externalDomainNode);
				NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
			});
		}

		public override string GetDisplayTitle ()
		{
			return string.Format ("{0} -> {1} -> {2}", Parent.Parent.DisplayName, Parent.DisplayName, DisplayName);
		}

	}
}


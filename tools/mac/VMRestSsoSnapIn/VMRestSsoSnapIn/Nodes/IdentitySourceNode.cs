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
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Tenant;

using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Nodes
{
	public class IdentitySourceNode : ScopeNode
	{
		private bool _systemDomain;
		private IdentityProviderDto _provider;
		private TenantConfigurationDto _tenantConfigDto;
		private readonly string _tenantName;

		public bool IsDefaultDomain { get; private set; }

		public IdentitySourceNode (IdentityProviderDto provider, string tenantName, bool systemDomain, string displayName, TenantConfigurationDto tenantConfigDto)
		{
			_provider = provider;
			_tenantName = tenantName;
			_systemDomain = systemDomain;
			_tenantConfigDto = tenantConfigDto;
			var isDefault = _tenantConfigDto.ProviderPolicy != null && _tenantConfigDto.ProviderPolicy.DefaultProvider == provider.Name;
			DisplayName = isDefault ? displayName + " (Default)" : displayName;
			IsDefaultDomain = isDefault;
		}

		private ServerDto GetServerDto ()
		{
			var dto = Parent.Parent.Parent.Tag as AuthTokenDto;
			return dto != null ? dto.ServerDto : null;
		}

		public void SetAsDefault (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var service = SnapInContext.Instance.ServiceGateway;
				var serverDto = GetServerDto ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				_tenantConfigDto.ProviderPolicy = new ProviderPolicyDto { DefaultProvider = _provider.Name };
				_tenantConfigDto = service.Tenant.UpdateConfig (serverDto, _tenantName, _tenantConfigDto, auth.Token, TenantConfigType.PROVIDER);
				IsDefaultDomain = true;
				DisplayName = IsDefaultDomain ? DisplayName + " (Default)" : DisplayName.Replace (" (Default)", string.Empty);
				((IdentitySourcesNode)(this.Parent)).Refresh (sender, e);
			});
		}

		public override void Refresh (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				Children.Clear ();
				var serverDto = GetServerDto ();
				var node = new UsersAndGroupsNode (serverDto, _tenantName, _provider, _systemDomain){ Parent = this };
				Children.Add (node);
				NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
			});
		}

		public override string GetDisplayTitle ()
		{
			return string.Format ("{0} -> {1} -> {2}", Parent.Parent.Parent.DisplayName, Parent.Parent.DisplayName, DisplayName);
		}
	}
}


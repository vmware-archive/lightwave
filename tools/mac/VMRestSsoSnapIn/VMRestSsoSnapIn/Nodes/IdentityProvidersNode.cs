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
	public class IdentityProvidersNode : ScopeNode
	{
		public IdentityProvidersNode ()
		{
			DisplayName = "External Identity Providers";
		}

		public ServerDto GetServerDto()
		{
			var dto = Parent.Parent.Tag as AuthTokenDto;
			return dto != null ? dto.ServerDto : null;
		}

		public void AddExternalIdentityProvider (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var form = new AddNewExternalIdentityProviderController
				{
					ServerDto = GetServerDto(),
					TenantName = GetTenant()
				};
				var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (result == VMIdentityConstants.DIALOGOK) {
					if (form.ExternalIdentityProviderDto != null) {
						UIErrorHelper.ShowAlert ("External IDP " + form.ExternalIdentityProviderDto.EntityID + " created successfully", "Information");
						Refresh (sender, e);
					}
				}
			});
		}

		public virtual void Refresh (object sender, EventArgs e)
		{
			NSNotificationCenter.DefaultCenter.PostNotificationName ("RefreshTableView", this);
		}

		public override string GetDisplayTitle ()
		{
			return string.Format ("{0} -> {1} -> {2}", Parent.Parent.DisplayName, Parent.DisplayName, DisplayName);
		}

		public List<ExternalIdentityProviderDto> GetIdentityProviders()
		{
			var list = new List<ExternalIdentityProviderDto> ();
			ActionHelper.Execute (delegate() {
				var serverDto = GetServerDto ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				var tenantName = GetTenant ();
				list = SnapInContext.Instance.ServiceGateway.MacExternalIdentityProviderService.GetAll (serverDto, tenantName, auth.Token);
			});
			return list;
		}

		public ExternalIdentityProviderDto GetIdentityProvider(ExternalIdentityProviderDto provider)
		{
			var idp = new ExternalIdentityProviderDto ();
			ActionHelper.Execute (delegate() {
				var serverDto = GetServerDto ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				var tenantName = GetTenant ();
				idp = SnapInContext.Instance.ServiceGateway.MacExternalIdentityProviderService.Get (serverDto, tenantName, provider, auth.Token);
			});
			return idp;
		}

		private ExternalIdentityProviderDto AddExternalIdp(ExternalIdentityProviderDto idp)
		{
			var serverDto = GetServerDto ();
			var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto.ServerName);
			var tenantName = GetTenant();
			return SnapInContext.Instance.ServiceGateway.MacExternalIdentityProviderService.Create(serverDto, tenantName, idp, auth.Token);
		}
		public void DeleteExternalIdentityProvider(ExternalIdentityProviderDto dto)
		{
			ActionHelper.Execute (delegate() {
				var serverDto = GetServerDto ();
				var tenant = GetTenant ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				var success = SnapInContext.Instance.ServiceGateway.MacExternalIdentityProviderService.Delete (serverDto, tenant, dto, auth.Token);
				if (success) {
					UIErrorHelper.ShowAlert ("External Identity Provider " + dto.EntityID + " deleted successfully", "Information");
				} else {

					UIErrorHelper.ShowAlert ("Failed to delete External Identity Provider " + dto.EntityID, "Information");
				}
				Refresh (this, EventArgs.Empty);
			});
		}
	}
}


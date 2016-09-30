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
	public class OidcClientNode : ScopeNode
	{
		public OidcClientNode ()
		{
			DisplayName = "OIDC client";
		}

		public ServerDto GetServerDto()
		{
			var dto = Parent.Parent.Tag as AuthTokenDto;
			return dto != null ? dto.ServerDto : null;
		}
		public void AddOidcClient (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var form = new AddNewOidcClientController ();
				var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (result == VMIdentityConstants.DIALOGOK) {
					var rp = AddOidcClient (form.OidcClientDto);
					if (rp != null) {
						UIErrorHelper.ShowAlert ("OIDC client with DN: " + rp.OidcClientMetadataDTO.CertSubjectDN + " and ID: " + rp.ClientId + " created successfully", "Information");
						Refresh (sender, e);
					}
				}
			});
		}

		public override string GetDisplayTitle ()
		{
			return string.Format ("{0} -> {1} -> {2}", Parent.Parent.Parent.DisplayName, Parent.DisplayName, DisplayName);
		}

		public List<OidcClientDto> GetOidcClients()
		{
			var list = new List<OidcClientDto> ();
			ActionHelper.Execute (delegate() {
				var serverDto = GetServerDto ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				var tenantName = GetTenant ();
				list = SnapInContext.Instance.ServiceGateway.OidcClient.GetAll (serverDto, tenantName, auth.Token);
			});
			return list;
		}

		private OidcClientDto AddOidcClient(OidcClientDto oidcClientDto)
		{
			var serverDto = GetServerDto ();
			var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto.ServerName);
			var tenantName = GetTenant();
			return SnapInContext.Instance.ServiceGateway.OidcClient.Create(serverDto, tenantName, oidcClientDto.OidcClientMetadataDTO, auth.Token);
		}
		public void DeleteOidc(OidcClientDto dto)
		{
			ActionHelper.Execute (delegate() {
				var serverDto = GetServerDto ();
				var tenant = GetTenant ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				var success = SnapInContext.Instance.ServiceGateway.OidcClient.Delete (serverDto, tenant, dto, auth.Token);
				if (success) {
					UIErrorHelper.ShowAlert ("Relying party " + dto.ClientId + " deleted successfully", "Information");
				} else {

					UIErrorHelper.ShowAlert ("Failed to delete relying party " + dto.ClientId, "Information");
				}
				Refresh (this, EventArgs.Empty);
			});
		}
	}
}


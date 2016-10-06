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
	public class RelyingPartyNode : ScopeNode
	{
		public RelyingPartyNode ()
		{
			DisplayName = "Relying Party";
		}

		public ServerDto GetServerDto()
		{
			var dto = Parent.Parent.Tag as AuthTokenDto;
			return dto != null ? dto.ServerDto : null;
		}
		public void AddRelyingParty (object sender, EventArgs e)
		{
			var serverDto = GetServerDto ();
			var tenant = GetTenant ();
			var form = new AddRelyingPartyController (){ ServerDto = serverDto, Tenant = tenant };
			var returnVal = NSApplication.SharedApplication.RunModalForWindow (form.Window);
			if (form.SaveSuccessful) {
				UIErrorHelper.ShowAlert ("Relying Party " + form.RelyingPartyDto.Name + " created successfully", "Information");
				Refresh (sender, e);
			}
		}

		public override string GetDisplayTitle ()
		{
			return string.Format ("{0} -> {1} -> {2}", Parent.Parent.Parent.DisplayName, Parent.DisplayName, DisplayName);
		}

		public List<RelyingPartyDto> GetRelyingParty()
		{
			var list = new List<RelyingPartyDto> ();
			ActionHelper.Execute (delegate() {
				var serverDto = GetServerDto ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				var tenantName = GetTenant ();
				list = SnapInContext.Instance.ServiceGateway.RelyingParty.GetAll (serverDto, tenantName, auth.Token);
			});
			return list;
		}

		public void DeleteRelyingParty(RelyingPartyDto dto)
		{
			ActionHelper.Execute (delegate() {
				var serverDto = GetServerDto ();
				var tenant = GetTenant ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				var success = SnapInContext.Instance.ServiceGateway.RelyingParty.Delete (serverDto, tenant, dto, auth.Token);
				if (success) {
					UIErrorHelper.ShowAlert ("Relying party " + dto.Name + " deleted successfully", "Information");
				} else {

					UIErrorHelper.ShowAlert ("Failed to delete relying party " + dto.Name, "Information");
				}
				Refresh (this, EventArgs.Empty);
			});
		}
	}
}


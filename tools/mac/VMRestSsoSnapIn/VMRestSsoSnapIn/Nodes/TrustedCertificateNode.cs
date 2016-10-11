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
using System.Text;
using System.Linq;
using System.Collections.Generic;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Tenant;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System.Security.Cryptography.X509Certificates;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Security.Certificate;
using RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using VmIdentity.CommonUtils.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Nodes
{
	public class TrustedCertificateNode : ScopeNode
	{
		
		public TrustedCertificateNode()
		{
			DisplayName = "Trusted Certificates";
		}

		public ServerDto GetServerDto()
		{
			var dto = Parent.Parent.Parent.Tag as AuthTokenDto;
			return dto != null ? dto.ServerDto : null;
		}

		public void AddCertificateChain (object sender, EventArgs e)
		{
			var tenantName = GetTenant ();
			var form = new AddNewTenantController () {UpdateCredentials = true, TenantDto = new TenantDto {Name = tenantName}};
			var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
			if (result == VMIdentityConstants.DIALOGOK) {
				var rp = AddNewCertificate(form.TenantDto);
				if (rp) {
					UIErrorHelper.ShowAlert ("Certificate chain added successfully", "Information");
					Refresh (sender, e);
				}
			}
		}
		public override string GetDisplayTitle ()
		{
			return string.Format ("{0} -> {1} -> {2}", Parent.Parent.Parent.DisplayName, Parent.DisplayName, DisplayName);
		}

		public List<CertificateDto> GetCertificates()
		{
			var result = new List<CertificateDto>();
			ActionHelper.Execute (delegate() {
				var serverDto = GetServerDto ();
				var tenantName = GetTenant ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				var service = SnapInContext.Instance.ServiceGateway;
				var chains = service.Certificate.GetCertificates (serverDto, tenantName, CertificateScope.TENANT, auth.Token);
				var chainId = 1;
				foreach (var chain in chains) {
					result.AddRange (chain.Certificates.Select (x => new CertificateDto {
						Encoded = x.Encoded,
						Chain = "Chain-" + chainId,
						IsSigner = (chainId == chains.Count)
					}));
					chainId++;
				}
			});
			return result;
		}
		private bool AddNewCertificate(TenantDto tenantDto)
		{
			var result = false;
			ActionHelper.Execute (delegate() {
				var serverDto = GetServerDto ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);
				result = SnapInContext.Instance.ServiceGateway.Certificate.SetTenantCredentials (auth.ServerDto, tenantDto.Name, tenantDto.Credentials, auth.Token);
			});
			return result;
		}

		public void DeleteCertficateChain(CertificateDto dto)
		{
			ActionHelper.Execute (delegate() {
				var serverDto = GetServerDto ();
				var tenant = GetTenant ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (serverDto.ServerName);

				var fingerprint = new X509Certificate2 (Encoding.ASCII.GetBytes(dto.Encoded)).GetFormattedThumbPrint ();
				var success = SnapInContext.Instance.ServiceGateway.Certificate.Delete (serverDto, tenant, fingerprint, auth.Token);
				if (success) {
					UIErrorHelper.ShowAlert ("Certificate chain " + dto.Chain + " deleted successfully", "Information");
				} else {

					UIErrorHelper.ShowAlert ("Failed to delete certificate chain" + dto.Chain, "Information");
				}
				Refresh (this, EventArgs.Empty);
			});
		}
	}
}


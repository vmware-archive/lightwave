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
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System.Security.Cryptography.X509Certificates;

using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn;

namespace RestSsoAdminSnapIn
{
	public partial class OidcClientDetailsViewController : AppKit.NSViewController
	{
		public OidcClientDto OidcClientDtoOriginal {get;set;}
		private OidcClientDto OidcClientDto {get;set;}
		public ServerDto ServerDto;
		public string TenantName;

		#region Constructors

		// Called when created from unmanaged code
		public OidcClientDetailsViewController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}

		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public OidcClientDetailsViewController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}

		// Call to load from the XIB/NIB file
		public OidcClientDetailsViewController () : base ("OidcClientDetailsView", NSBundle.MainBundle)
		{
			Initialize ();
		}

		// Shared initialization code
		void Initialize ()
		{
		}

		#endregion

		//strongly typed view accessor
		public new OidcClientDetailsView View {
			get {
				return (OidcClientDetailsView)base.View;
			}
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			OidcClientDto = new OidcClientDto {
				ClientId = OidcClientDtoOriginal.ClientId,
				OidcClientMetadataDTO = new OidcClientMetadataDto {
					RedirectUris = OidcClientDtoOriginal.OidcClientMetadataDTO.RedirectUris, 
					PostLogoutRedirectUris = OidcClientDtoOriginal.OidcClientMetadataDTO.PostLogoutRedirectUris,
					CertSubjectDN = OidcClientDtoOriginal.OidcClientMetadataDTO.CertSubjectDN,
					LogoutUri = OidcClientDtoOriginal.OidcClientMetadataDTO.LogoutUri,
					TokenEndpointAuthMethod = OidcClientDtoOriginal.OidcClientMetadataDTO.TokenEndpointAuthMethod
				}
			};

			TxtName.StringValue = OidcClientDtoOriginal.ClientId;
			var authIndex = OidcClientDtoOriginal.OidcClientMetadataDTO.TokenEndpointAuthMethod == "none" ? 0 : 1;
			CbTokenAuthMethod.SelectItem (authIndex);
			TxtLogoutUrl.StringValue = string.IsNullOrEmpty (OidcClientDtoOriginal.OidcClientMetadataDTO.LogoutUri) ? string.Empty :
				OidcClientDtoOriginal.OidcClientMetadataDTO.LogoutUri;
			TxtCertificateDN.StringValue = string.IsNullOrEmpty (OidcClientDtoOriginal.OidcClientMetadataDTO.CertSubjectDN) ? 
				string.Empty : OidcClientDtoOriginal.OidcClientMetadataDTO.CertSubjectDN;
			ReloadTableView(RedirectUriTableView, OidcClientDto.OidcClientMetadataDTO.RedirectUris);
			ReloadTableView(PostLogoutRedirectUriTableView, OidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris);

			BtnBrowseCertificate.Activated +=	(object sender, EventArgs e) => {
				var openPanel = new NSOpenPanel();
				openPanel.ReleasedWhenClosed = true;
				openPanel.Prompt = "Select file";

				var result = openPanel.RunModal();
				if (result == 1)
				{
					var filePath = openPanel.Url.AbsoluteString.Replace("file://",string.Empty);
					var cert = new X509Certificate2 ();
					ActionHelper.Execute (delegate() {
						cert.Import (filePath);
						TxtCertificateDN.StringValue = cert.Subject;
					});
				}
			};

			BtnAddRedirectUri.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtRedirectUri.StringValue))
				{
					UIErrorHelper.ShowAlert ("Redirect Uri cannot be empty", "Alert");
					return;
				}
				OidcClientDto.OidcClientMetadataDTO.RedirectUris.Add(TxtRedirectUri.StringValue);
				ReloadTableView(RedirectUriTableView, OidcClientDto.OidcClientMetadataDTO.RedirectUris);
				TxtRedirectUri.StringValue = (NSString)string.Empty;
			};

			BtnAddPostLogoutRedirectUri.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtPostLogoutRedirectUri.StringValue))
				{
					UIErrorHelper.ShowAlert ("Post logout redirect Uri cannot be empty", "Alert");
					return;
				}
				OidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris.Add(TxtPostLogoutRedirectUri.StringValue);
				ReloadTableView(PostLogoutRedirectUriTableView, OidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris);
				TxtPostLogoutRedirectUri.StringValue = (NSString)string.Empty;
			};

			BtnRemoveRedirectUri.Activated += (object sender, EventArgs e) => {
				if (RedirectUriTableView.SelectedRows.Count > 0) {
					foreach (var row in RedirectUriTableView.SelectedRows) {

						OidcClientDto.OidcClientMetadataDTO.RedirectUris.RemoveAt((int)row);
					}
					ReloadTableView(RedirectUriTableView, OidcClientDto.OidcClientMetadataDTO.RedirectUris);
				}
			};

			BtnRemovePostLogoutRedirectUri.Activated += (object sender, EventArgs e) => {
				if (PostLogoutRedirectUriTableView.SelectedRows.Count > 0) {
					foreach (var row in PostLogoutRedirectUriTableView.SelectedRows) {

						OidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris.RemoveAt((int)row);
					}
					ReloadTableView(PostLogoutRedirectUriTableView, OidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris);
				}
			};

			this.BtnApply.Activated += (object sender, EventArgs e) => {

				ActionHelper.Execute (delegate() {
					var value = (NSString)CbTokenAuthMethod.SelectedValue;
					if (value == "private_key_jwt" && string.IsNullOrEmpty (TxtCertificateDN.StringValue)) {
						UIErrorHelper.ShowAlert ("Please choose a valid certificate", "Alert");
					} else if (string.IsNullOrEmpty (TxtLogoutUrl.StringValue)) {
						UIErrorHelper.ShowAlert ("Please enter valid logout uri", "Alert");
					} else if (OidcClientDto.OidcClientMetadataDTO.RedirectUris.Count == 0) {
						UIErrorHelper.ShowAlert ("Please enter a valid redirect URI", "Alert");
					} else if (OidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris.Count == 0) {
						UIErrorHelper.ShowAlert ("Please enter a valid post logout redirect URI", "Alert");
					}
						else {
						OidcClientDto.OidcClientMetadataDTO.LogoutUri = TxtLogoutUrl.StringValue;
						OidcClientDto.OidcClientMetadataDTO.TokenEndpointAuthMethod = (NSString)CbTokenAuthMethod.SelectedValue;
						OidcClientDto.OidcClientMetadataDTO.CertSubjectDN = TxtCertificateDN.StringValue;
						var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
						OidcClientDto = SnapInContext.Instance.ServiceGateway.OidcClient.Update (ServerDto, TenantName, OidcClientDto.ClientId, OidcClientDto.OidcClientMetadataDTO, auth.Token);
						NSNotificationCenter.DefaultCenter.PostNotificationName ("RefreshTableView", this);
					}
				});
			};
		}
				
		public void ReloadTableView(NSTableView tableView, List<string> datasource)
		{
			tableView.Delegate = new TableDelegate ();
			var listView = new DefaultDataSource { Entries = datasource };
			tableView.DataSource = listView;
			tableView.ReloadData ();
		}
			
		public class TableDelegate : NSTableViewDelegate
		{
			public TableDelegate ()
			{
			}

			public override void WillDisplayCell (NSTableView tableView, NSObject cell,
				NSTableColumn tableColumn, nint row)
			{
				ActionHelper.Execute (delegate() {
					NSBrowserCell browserCell = cell as NSBrowserCell;
					if (browserCell != null) {
						browserCell.Leaf = true;
					}
				});
			}
		}
	}
}

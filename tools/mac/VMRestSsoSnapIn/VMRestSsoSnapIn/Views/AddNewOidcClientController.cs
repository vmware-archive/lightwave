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
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;

namespace RestSsoAdminSnapIn
{
	public partial class AddNewOidcClientController : NSWindowController
	{
		public OidcClientDto OidcClientDto {get;set;}
			
		public AddNewOidcClientController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public AddNewOidcClientController (NSCoder coder) : base (coder)
		{
		}

		public AddNewOidcClientController () : base ("AddNewOidcClient")
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			OidcClientDto = new OidcClientDto { 
				OidcClientMetadataDTO = new OidcClientMetadataDto {
					RedirectUris=new List<string>(), 
					PostLogoutRedirectUris = new List<string>()
				} 
			};
			BtnSelectCertificate.Activated +=	(object sender, EventArgs e) => {
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
				} else if(!WebUtil.IsValidHttpUrl(TxtRedirectUri.StringValue))
				{
					UIErrorHelper.ShowAlert ("Redirect Uri is invalid", "Alert");
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
				} else if(!WebUtil.IsValidHttpUrl(TxtPostLogoutRedirectUri.StringValue))
				{
					UIErrorHelper.ShowAlert ("Post logout is invalid", "Alert");
					return;
				}
				OidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris.Add(TxtPostLogoutRedirectUri.StringValue);
				ReloadTableView(PostLogoutUtiTableView, OidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris);
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
				if (PostLogoutUtiTableView.SelectedRows.Count > 0) {
					foreach (var row in PostLogoutUtiTableView.SelectedRows) {

						OidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris.RemoveAt((int)row);
					}
					ReloadTableView(PostLogoutUtiTableView, OidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris);
				}
			};

			BtnClose.Activated += (object sender, EventArgs e) => {
				OidcClientDto = null;
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};

			this.BtnSave.Activated += (object sender, EventArgs e) => {
				var value = (NSString)CbAuthTokenMethod.SelectedValue;
				if (value == "private_key_jwt" && string.IsNullOrEmpty (TxtCertificateDN.StringValue)) {
					UIErrorHelper.ShowAlert ("Please choose a valid certificate", "Alert");
				} else if (string.IsNullOrEmpty (TxtLogoutUri.StringValue) || !WebUtil.IsValidHttpUrl(TxtLogoutUri.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid logout uri", "Alert");
				} else if (OidcClientDto.OidcClientMetadataDTO.RedirectUris.Count == 0) {
					UIErrorHelper.ShowAlert ("Please enter a valid redirect URI", "Alert");
				} else if (OidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris.Count == 0) {
					UIErrorHelper.ShowAlert ("Please enter a valid post logout redirect URI", "Alert");
				}else {
					OidcClientDto.OidcClientMetadataDTO.LogoutUri = TxtLogoutUri.StringValue;
					OidcClientDto.OidcClientMetadataDTO.TokenEndpointAuthMethod = (NSString)CbAuthTokenMethod.SelectedValue;
					OidcClientDto.OidcClientMetadataDTO.CertSubjectDN = TxtCertificateDN.StringValue;
					this.Close ();
					NSApplication.SharedApplication.StopModalWithCode (1);
				}
			};

			CbAuthTokenMethod.SelectItem (0);
		}

		public void ReloadTableView(NSTableView tableView, List<string> datasource)
		{
			tableView.Delegate = new TableDelegate ();
			var listView = new DefaultDataSource { Entries = datasource };
			tableView.DataSource = listView;
			tableView.ReloadData ();
		}

		public new AddNewOidcClient Window {
			get { return (AddNewOidcClient)base.Window; }
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

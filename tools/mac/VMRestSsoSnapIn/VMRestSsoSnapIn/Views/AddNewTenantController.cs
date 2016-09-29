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
using System.Collections.Generic;
using System.Linq;
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System.Security.Cryptography.X509Certificates;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Crypto;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers;
using VmIdentity.CommonUtils.Utilities;

namespace RestSsoAdminSnapIn
{
	public partial class AddNewTenantController : AppKit.NSWindowController
	{
		public TenantDto TenantDto { get; set;}
		public bool UpdateCredentials { get; set; }
		private List<string> _certs;


		#region Constructors

		// Called when created from unmanaged code
		public AddNewTenantController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public AddNewTenantController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public AddNewTenantController () : base ("AddNewTenant")
		{
			Initialize ();
		}
		
		// Shared initialization code
		void Initialize ()
		{
			
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			_certs = new List<string> ();
			TxtTenantName.Enabled = !UpdateCredentials;
			txtUsername.Enabled = !UpdateCredentials;
			TxtPassword.Enabled = !UpdateCredentials;
			if (UpdateCredentials)
				TxtTenantName.StringValue = TenantDto.Name;
			else
				TenantDto = new TenantDto ();
			TenantDto.Credentials = new TenantCredentialsDto(){Certificates = new List<CertificateDto>()};
			
			BtnAddCertificate.Activated +=	(object sender, EventArgs e) => {
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
						_certs.Add(filePath);
						var certfificateDto = new CertificateDto { Encoded = cert.ExportToPem(), };
						TenantDto.Credentials.Certificates.Add(certfificateDto);
						ReloadCertificates();
					});
				}
			};

			BtnRemoveCertificate.Activated += (object sender, EventArgs e) => {
				if (CertificateChainTableView.SelectedRows.Count > 0) {
					foreach (var row in CertificateChainTableView.SelectedRows) {
						_certs.RemoveAt ((int)row);
						TenantDto.Credentials.Certificates.RemoveAt ((int)row);
					}
					ReloadCertificates();
				}
			};

			BtnBrowsePrivateKey.Activated += (object sender, EventArgs e) => {
				var openPanel = new NSOpenPanel();
				openPanel.ReleasedWhenClosed = true;
				openPanel.Prompt = "Select file";
				var result = openPanel.RunModal();
				if (result == 1)
				{
					var filePath = openPanel.Url.AbsoluteString.Replace("file://",string.Empty);

					ActionHelper.Execute (delegate() {
						if(ShaWithRsaSigner.IsPrivateKeyValid(filePath))
						{
							var text = System.IO.File.ReadAllText(filePath);
							var privateKey = PrivateKeyHelper.ExtractBase64EncodedPayload(text);
							TxtPrivateKeyPath.StringValue = filePath;
							TenantDto.Credentials.PrivateKey = new PrivateKeyDto(){ Algorithm = EncrptionAlgorithm.RSA, Encoded = privateKey };
						}
						else
						{
							UIErrorHelper.ShowAlert ("Selected private key is not valid", "Alert");
						}
					});
				}
			};

			BtnClose.Activated += (object sender, EventArgs e) => {
				TenantDto = null;
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};
			this.BtnSave.Activated += (object sender, EventArgs e) => {
				if (!UpdateCredentials && string.IsNullOrEmpty (TxtTenantName.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid tenant name", "Alert");
				} else if (!UpdateCredentials && string.IsNullOrEmpty (txtUsername.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid username", "Alert");
				} else if (!UpdateCredentials && string.IsNullOrEmpty (TxtPassword.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid password", "Alert");
				} else if (string.IsNullOrEmpty (TxtPrivateKeyPath.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid private key", "Alert");
				} else if (_certs.Count < 2) {
					UIErrorHelper.ShowAlert ("Please enter atleast 2 valid Certificates", "Alert");
				} else {
					TenantDto.Name = TxtTenantName.StringValue;
					TenantDto.Username = txtUsername.StringValue;
					TenantDto.Password = TxtPassword.StringValue;
					this.Close ();
					NSApplication.SharedApplication.StopModalWithCode (1);
				}
			};
		}
		#endregion

		public void ReloadCertificates()
		{
			foreach(NSTableColumn column in CertificateChainTableView.TableColumns())
			{
				CertificateChainTableView.RemoveColumn (column);
			}
			CertificateChainTableView.Delegate = new TableDelegate ();
			var listView = new DefaultDataSource { Entries = _certs };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Certificate", DisplayOrder = 1, Width = 400 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				CertificateChainTableView.AddColumn (column);
			}
			CertificateChainTableView.DataSource = listView;
			CertificateChainTableView.ReloadData ();
		}

		//strongly typed window accessor
		public new AddNewTenant Window {
			get {
				return (AddNewTenant)base.Window;
			}
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
						if (tableColumn.Identifier == "Name") {
							var image = NSImage.ImageNamed ("certificate.png");
							browserCell.Image = image;
							browserCell.Image.Size = new CoreGraphics.CGSize{ Width = (float)16.0, Height = (float)16.0 };
						}
					}
				});
			}
		}
	}
}


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
using System.Security.Cryptography.X509Certificates;
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn;

namespace RestSsoAdminSnapIn
{
	public partial class ExtenalIdpDetailsViewController : AppKit.NSViewController
	{
		public ExternalIdentityProviderDto ExternalIdentityProviderDto { get; set; }
		public ServerDto ServerDto { get; set; }
		public string TenantName { get; set; }

		#region Constructors

		// Called when created from unmanaged code
		public ExtenalIdpDetailsViewController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}

		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public ExtenalIdpDetailsViewController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}

		// Call to load from the XIB/NIB file
		public ExtenalIdpDetailsViewController () : base ("ExtenalIdpDetailsView", NSBundle.MainBundle)
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
			if (ExternalIdentityProviderDto.NameIDFormats == null)
				ExternalIdentityProviderDto.NameIDFormats = new List<string> ();

			if (ExternalIdentityProviderDto.SubjectFormats == null)
				ExternalIdentityProviderDto.SubjectFormats = new Dictionary<string, string> ();
			if (ExternalIdentityProviderDto.SsoServices == null)
				ExternalIdentityProviderDto.SsoServices = new List<ServiceEndpointDto> ();
			if (ExternalIdentityProviderDto.SloServices == null)
				ExternalIdentityProviderDto.SloServices = new List<ServiceEndpointDto> ();
			if (ExternalIdentityProviderDto.SigningCertificates == null)
				ExternalIdentityProviderDto.SigningCertificates = new CertificateChainDto{
					Certificates = new List<CertificateDto>()
			};
			TxtEntityName.StringValue = (NSString)(string.IsNullOrEmpty (ExternalIdentityProviderDto.EntityID) ? string.Empty : ExternalIdentityProviderDto.EntityID);
			TxtAlias.StringValue = (NSString) (string.IsNullOrEmpty (ExternalIdentityProviderDto.Alias) ? string.Empty : ExternalIdentityProviderDto.Alias);
			BtnJit.StringValue = ExternalIdentityProviderDto.JitEnabled ? "1" : "0";
			ReloadTableView(NameFormatTableView, ExternalIdentityProviderDto.NameIDFormats);
			ReloadTableView(SubjectFormatTableView, ExternalIdentityProviderDto.SubjectFormats);
			ReloadCertificates ();
			InitializeSsoServices ();
			InitializeSloServices ();

			BtnViewCertificate.Activated += (object sender, EventArgs e) => 
			{
				if (CertificateTableView.SelectedRows.Count > 0) {
					var row = CertificateTableView.SelectedRows.First();
					var encoded = ExternalIdentityProviderDto.SigningCertificates.Certificates[(int)row].Encoded;
					var bytes = System.Text.Encoding.ASCII.GetBytes (encoded);
					var certificate = new X509Certificate2(bytes);
					CertificateService.DisplayX509Certificate2(this, certificate);
				}
			};

			EditButton.Activated += (object sender, EventArgs e) => {
				ActionHelper.Execute (delegate() {
					var form = new AddNewExternalIdentityProviderController
					{
						ServerDto = ServerDto,
						TenantName = TenantName,
						ExternalIdentityProviderDto = ExternalIdentityProviderDto
					};
					var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
					if (result == VMIdentityConstants.DIALOGOK) {
						if (form.ExternalIdentityProviderDto != null) {
							UIErrorHelper.ShowAlert ("External IDP " + form.ExternalIdentityProviderDto.EntityID + " updated successfully", "Information");
							Refresh (sender, e);
						}
					}
				});
			};
		}
		private void ReloadTableView(NSTableView tableView, Dictionary<string,string> datasource)
		{
			foreach(NSTableColumn column in tableView.TableColumns())
			{
				tableView.RemoveColumn (column);
			}
			tableView.Delegate = new TableDelegate ();
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 1, Width = 80 },
				new ColumnOptions{ Id = "Value", DisplayName = "Value", DisplayOrder = 1, Width = 200 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				tableView.AddColumn (column);
			}
			var listView = new DictionaryDataSource { Entries = datasource.Keys.ToList(), Datasource = datasource };
			tableView.DataSource = listView;
			tableView.ReloadData ();
		}
		private void ReloadCertificates()
		{
			foreach(NSTableColumn column in CertificateTableView.TableColumns())
			{
				CertificateTableView.RemoveColumn (column);
			}
			CertificateTableView.Delegate = new CertTableDelegate ();
			var listView = new TrustedCertificatesDataSource { Entries = ExternalIdentityProviderDto.SigningCertificates.Certificates };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "SubjectDn", DisplayName = "Subject DN", DisplayOrder = 1, Width = 120 },
				new ColumnOptions{ Id = "IssuedBy", DisplayName = "Issuer", DisplayOrder = 1, Width = 150 },
				new ColumnOptions{ Id = "IssuedOn", DisplayName = "Valid From", DisplayOrder = 1, Width = 80 },
				new ColumnOptions{ Id = "Expiration", DisplayName = "Valid To", DisplayOrder = 1, Width = 80 },
				new ColumnOptions{ Id = "Fingerprint", DisplayName = "FingerPrint", DisplayOrder = 1, Width = 150 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				CertificateTableView.AddColumn (column);
			}
			CertificateTableView.DataSource = listView;
			CertificateTableView.ReloadData ();
		}
		private void InitializeSsoServices()
		{
			foreach(NSTableColumn column in SsoTableView.TableColumns())
			{
				SsoTableView.RemoveColumn (column);
			}
			SsoTableView.Delegate = new TableDelegate ();
			var listView = new ServiceEndpointDataSource { Entries = ExternalIdentityProviderDto.SsoServices };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 1, Width = 150 },
				new ColumnOptions{ Id = "Endpoint", DisplayName = "Endpoint", DisplayOrder = 4, Width = 200 },
				new ColumnOptions{ Id = "Binding", DisplayName = "Binding", DisplayOrder = 5, Width = 200 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				SsoTableView.AddColumn (column);
			}
			SsoTableView.DataSource = listView;
			SsoTableView.ReloadData ();
		}
		private void InitializeSloServices()
		{
			foreach(NSTableColumn column in SloTableView.TableColumns())
			{
				SloTableView.RemoveColumn (column);
			}
			SloTableView.Delegate = new TableDelegate ();
			var listView = new ServiceEndpointDataSource { Entries = ExternalIdentityProviderDto.SloServices };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 1, Width = 150 },
				new ColumnOptions{ Id = "Endpoint", DisplayName = "Endpoint", DisplayOrder = 4, Width = 200 },
				new ColumnOptions{ Id = "Binding", DisplayName = "Binding", DisplayOrder = 5, Width = 200 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				SloTableView.AddColumn (column);
			}
			SloTableView.DataSource = listView;
			SloTableView.ReloadData ();
		}
		private void ReloadTableView(NSTableView tableView, List<string> datasource)
		{
			tableView.Delegate = new TableDelegate ();
			var listView = new DefaultDataSource { Entries = datasource };
			tableView.DataSource = listView;
			tableView.ReloadData ();
		}
		#endregion

		//strongly typed view accessor
		public new ExtenalIdpDetailsView View {
			get {
				return (ExtenalIdpDetailsView)base.View;
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
					}
				});
			}
		}
		public class CertTableDelegate : NSTableViewDelegate
		{
			public CertTableDelegate ()
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

		public virtual void Refresh (object sender, EventArgs e)
		{
			NSNotificationCenter.DefaultCenter.PostNotificationName ("RefreshTableView", this);
		}
	}
}

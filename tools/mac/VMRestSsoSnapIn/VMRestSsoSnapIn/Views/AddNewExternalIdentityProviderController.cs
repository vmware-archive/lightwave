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
using VmIdentity.CommonUtils.Utilities;

namespace RestSsoAdminSnapIn
{
	public partial class AddNewExternalIdentityProviderController : NSWindowController
	{
		public ExternalIdentityProviderDto ExternalIdentityProviderDto{ get; set; }
		public ServerDto ServerDto { get; set; }
		public string TenantName { get; set; }

		public AddNewExternalIdentityProviderController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public AddNewExternalIdentityProviderController (NSCoder coder) : base (coder)
		{
		}

		public AddNewExternalIdentityProviderController () : base ("AddNewExternalIdentityProvider")
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();

			if (ExternalIdentityProviderDto == null) {
				ExternalIdentityProviderDto = new ExternalIdentityProviderDto () {
					NameIDFormats = new List<string> (),
					SubjectFormats = new Dictionary<string, string> (),
					SsoServices = new List<ServiceEndpointDto> (),
					SloServices = new List<ServiceEndpointDto> (),
					SigningCertificates = new CertificateChainDto {
						Certificates = new List<CertificateDto> ()
					}
				};
			} else {
				DtoToView ();
			}

			// Name Id formats
			BtnAddNameIdFormat.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtNameIdFormat.StringValue))
				{
					UIErrorHelper.ShowAlert ("Name Id format cannot be empty", "Alert");
					return;
				}
				ExternalIdentityProviderDto.NameIDFormats.Add(TxtNameIdFormat.StringValue);
				ReloadTableView(LstNameIdFormat, ExternalIdentityProviderDto.NameIDFormats);
				TxtNameIdFormat.StringValue = (NSString)string.Empty;
			};

			BtnRemoveNameIdFormat.Activated += (object sender, EventArgs e) => {
				if (LstNameIdFormat.SelectedRows.Count > 0) {
					foreach (var row in LstNameIdFormat.SelectedRows) {

						ExternalIdentityProviderDto.NameIDFormats.RemoveAt((int)row);
					}
					ReloadTableView(LstNameIdFormat, ExternalIdentityProviderDto.NameIDFormats);
				}
			};
			ReloadTableView(LstNameIdFormat, ExternalIdentityProviderDto.NameIDFormats);

			// Subject formats
			BtnAddSubjectFormat.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtSubjectFormatName.StringValue))
				{
					UIErrorHelper.ShowAlert ("Subject format name cannot be empty", "Alert");
					return;
				}
				if(string.IsNullOrEmpty(TxtSubjectFormatValue.StringValue))
				{
					UIErrorHelper.ShowAlert ("Subject format value cannot be empty", "Alert");
					return;
				}
				if(ExternalIdentityProviderDto.SubjectFormats.ContainsKey(TxtSubjectFormatName.StringValue))
				{
					UIErrorHelper.ShowAlert ("Subject format name already exists", "Alert");
					return;
				}
				ExternalIdentityProviderDto.SubjectFormats.Add(TxtSubjectFormatName.StringValue, TxtSubjectFormatValue.StringValue);
				ReloadTableView(LstSubjectFormat, ExternalIdentityProviderDto.SubjectFormats);
				TxtSubjectFormatName.StringValue = (NSString)string.Empty;
				TxtSubjectFormatValue.StringValue = (NSString)string.Empty;
			};

			BtnRemoveSubjectFormat.Activated += (object sender, EventArgs e) => {
				if (LstSubjectFormat.SelectedRows.Count > 0) {
					foreach (var row in LstSubjectFormat.SelectedRows) {
						var source = LstSubjectFormat.DataSource as DictionaryDataSource;
						var name = source.Entries[(int)row];
						ExternalIdentityProviderDto.SubjectFormats.Remove(name);
					}
					ReloadTableView(LstSubjectFormat, ExternalIdentityProviderDto.SubjectFormats);
				}
			};
			ReloadTableView(LstSubjectFormat, ExternalIdentityProviderDto.SubjectFormats);

			// Certificates
			BtnAddCertificate.Activated +=	(object sender, EventArgs e) => {
				var openPanel = new NSOpenPanel();
				openPanel.ReleasedWhenClosed = true;
				openPanel.Prompt = "Select file";

				var result = openPanel.RunModal();
				if (result == 1)
				{
					var filePath = Uri.UnescapeDataString (openPanel.Url.AbsoluteString.Replace("file://",string.Empty));
					var cert = new X509Certificate2 ();
					ActionHelper.Execute (delegate() {
						cert.Import (filePath);
						var certfificateDto = new CertificateDto { Encoded = cert.ExportToPem(), };
						ExternalIdentityProviderDto.SigningCertificates.Certificates.Add(certfificateDto);
						ReloadCertificates();
					});
				}
			};

			BtnRemoveCertificate.Activated += (object sender, EventArgs e) => {
				if (LstCertificates.SelectedRows.Count > 0) {
					foreach (var row in LstCertificates.SelectedRows) {
						ExternalIdentityProviderDto.SigningCertificates.Certificates.RemoveAt ((int)row);
					}
					ReloadCertificates();
				}
			};
			ReloadCertificates ();

			// Sso Services
			BtnAddSso.Activated += OnAddSsoServices;
			BtnRemoveSso.Activated += OnRemoveSsoServices;	
			InitializeSsoServices ();

			// Slo Services
			BtnAddSlo.Activated += OnAddSloServices;
			BtnRemoveSlo.Activated += OnRemoveSloServices;	
			InitializeSloServices ();

			this.BtnSave.Activated += (object sender, EventArgs e) => {
				if (string.IsNullOrEmpty (TxtUniqueId.StringValue)) {
					UIErrorHelper.ShowAlert ("Please choose a Unique Id", "Alert");
				} else if(string.IsNullOrEmpty(TxtAlias.StringValue))
				{
					UIErrorHelper.ShowAlert ("Alias cannot be empty", "Alert");
				} else if (ExternalIdentityProviderDto.NameIDFormats.Count() < 1) {
					UIErrorHelper.ShowAlert ("Please choose a Name Id format", "Alert");
				} else if (ExternalIdentityProviderDto.SubjectFormats.Count() < 1) {
					UIErrorHelper.ShowAlert ("Please choose a Subject Id format", "Alert");
				} else if (ExternalIdentityProviderDto.SsoServices.Count() < 1) {
					UIErrorHelper.ShowAlert ("Please choose a Sso Service", "Alert");
				} else if (ExternalIdentityProviderDto.SloServices.Count() < 1) {
					UIErrorHelper.ShowAlert ("Please choose a Slo service", "Alert");
				} else if (ExternalIdentityProviderDto.SigningCertificates.Certificates.Count() < 1) {
					UIErrorHelper.ShowAlert ("Please choose a certificate", "Alert");
				} else {
					ExternalIdentityProviderDto.EntityID = TxtUniqueId.StringValue;
					ExternalIdentityProviderDto.Alias = TxtAlias.StringValue;
					ExternalIdentityProviderDto.JitEnabled = ChkJit.StringValue == "1";

					ActionHelper.Execute(delegate {
						var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto.ServerName);
						SnapInContext.Instance.ServiceGateway.MacExternalIdentityProviderService.Create(ServerDto,TenantName,ExternalIdentityProviderDto,auth.Token);
						this.Close ();
						NSApplication.SharedApplication.StopModalWithCode (1);
					});
				}
			};

			BtnClose.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};
			BtnViewCertificate.Activated += (object sender, EventArgs e) => 
			{
				if (LstCertificates.SelectedRows.Count > 0) {
					var row = LstCertificates.SelectedRows.First();
					var encoded = ExternalIdentityProviderDto.SigningCertificates.Certificates[(int)row].Encoded;
					var bytes = System.Text.Encoding.ASCII.GetBytes (encoded);
					var certificate = new X509Certificate2(bytes);
					CertificateService.DisplayX509Certificate2(this, certificate);
				}
			};
		}

		private void DtoToView(){
			TxtUniqueId.StringValue = ExternalIdentityProviderDto.EntityID;
			TxtUniqueId.Enabled = false;
			TxtAlias.StringValue = ExternalIdentityProviderDto.Alias;
			ChkJit.StringValue = ExternalIdentityProviderDto.JitEnabled ? "1" : "0";
		}

		private void ReloadTableView(NSTableView tableView, List<string> datasource)
		{
			tableView.Delegate = new TableDelegate ();
			var listView = new DefaultDataSource { Entries = datasource };
			tableView.DataSource = listView;
			tableView.ReloadData ();
		}
		private void ReloadTableView(NSTableView tableView, Dictionary<string,string> datasource)
		{
			foreach(NSTableColumn column in tableView.TableColumns())
			{
				tableView.RemoveColumn (column);
			}
			tableView.Delegate = new TableDelegate ();
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 0, Width = 80 },
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
			foreach(NSTableColumn column in LstCertificates.TableColumns())
			{
				LstCertificates.RemoveColumn (column);
			}
			LstCertificates.Delegate = new CertTableDelegate ();
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
				LstCertificates.AddColumn (column);
			}
			LstCertificates.DataSource = listView;
			LstCertificates.ReloadData ();
		}
		private void InitializeSsoServices()
		{
			foreach(NSTableColumn column in LstSso.TableColumns())
			{
				LstSso.RemoveColumn (column);
			}
			LstSso.Delegate = new TableDelegate ();
			var listView = new ServiceEndpointDataSource { Entries = ExternalIdentityProviderDto.SsoServices };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 1, Width = 150 },
				new ColumnOptions{ Id = "Endpoint", DisplayName = "Endpoint", DisplayOrder = 4, Width = 200 },
				new ColumnOptions{ Id = "Binding", DisplayName = "Binding", DisplayOrder = 5, Width = 200 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				LstSso.AddColumn (column);
			}
			LstSso.DataSource = listView;
			LstSso.ReloadData ();
		}
		private void OnAddSsoServices (object sender, EventArgs e)
		{
			if (IsSsoServiceValid ()) {
				var endpointDto = new ServiceEndpointDto {
					Name = TxtSsoName.StringValue,
					Endpoint = TxtSsoEndpoint.StringValue,
					Binding = TxtSsoBinding.StringValue
				};
				ExternalIdentityProviderDto.SsoServices.Add(endpointDto);
				var datasource = new ServiceEndpointDataSource { Entries = ExternalIdentityProviderDto.SsoServices };
				LstSso.DataSource = datasource;
				LstSso.ReloadData ();
				TxtSsoName.StringValue = (NSString)string.Empty;
				TxtSsoEndpoint.StringValue = (NSString)string.Empty;
				TxtSsoBinding.StringValue = (NSString)string.Empty;
			}
		}
		private bool IsSsoServiceValid()
		{
			if(string.IsNullOrEmpty(TxtSsoName.StringValue))
			{
				UIErrorHelper.ShowAlert ("Sso service name cannot be empty", "Alert");
				return false;
			} else if(string.IsNullOrEmpty(TxtSsoBinding.StringValue))
			{
				UIErrorHelper.ShowAlert ("Sso service binding cannot be empty", "Alert");
				return false;
			} else if(string.IsNullOrEmpty(TxtSsoEndpoint.StringValue))
			{
				UIErrorHelper.ShowAlert ("Sso service endpoint cannot be empty", "Alert");
				return false;
			}
			return true;
		}
		private void OnRemoveSsoServices (object sender, EventArgs e)
		{
			if (LstSso.SelectedRows.Count > 0) {
				foreach (var row in LstSso.SelectedRows) {
					ExternalIdentityProviderDto.SsoServices.RemoveAt ((int)row);
				}
				var datasource = new ServiceEndpointDataSource { Entries = ExternalIdentityProviderDto.SsoServices };
				LstSso.DataSource = datasource;
				LstSso.ReloadData ();
			}
		}

		private void InitializeSloServices()
		{
			foreach(NSTableColumn column in LstSlo.TableColumns())
			{
				LstSlo.RemoveColumn (column);
			}
			LstSlo.Delegate = new TableDelegate ();
			var listView = new ServiceEndpointDataSource { Entries = ExternalIdentityProviderDto.SloServices };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 1, Width = 150 },
				new ColumnOptions{ Id = "Endpoint", DisplayName = "Endpoint", DisplayOrder = 4, Width = 200 },
				new ColumnOptions{ Id = "Binding", DisplayName = "Binding", DisplayOrder = 5, Width = 200 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				LstSlo.AddColumn (column);
			}
			LstSlo.DataSource = listView;
			LstSlo.ReloadData ();
		}
		private void OnAddSloServices (object sender, EventArgs e)
		{
			if (IsSloServiceValid ()) {
				var endpointDto = new ServiceEndpointDto {
					Name = TxtSloName.StringValue,
					Endpoint = TxtSloEndpoint.StringValue,
					Binding = TxtSloBinding.StringValue
				};
				ExternalIdentityProviderDto.SloServices.Add(endpointDto);
				var datasource = new ServiceEndpointDataSource { Entries = ExternalIdentityProviderDto.SloServices };
				LstSlo.DataSource = datasource;
				LstSlo.ReloadData ();
				TxtSloName.StringValue = (NSString)string.Empty;
				TxtSloEndpoint.StringValue = (NSString)string.Empty;
				TxtSloBinding.StringValue = (NSString)string.Empty;
			}
		}
		private bool IsSloServiceValid()
		{
			if(string.IsNullOrEmpty(TxtSloName.StringValue))
			{
				UIErrorHelper.ShowAlert ("Slo service name cannot be empty", "Alert");
				return false;
			} else if(string.IsNullOrEmpty(TxtSloBinding.StringValue))
			{
				UIErrorHelper.ShowAlert ("Slo service binding cannot be empty", "Alert");
				return false;
			} else if(string.IsNullOrEmpty(TxtSloEndpoint.StringValue))
			{
				UIErrorHelper.ShowAlert ("Slo service endpoint cannot be empty", "Alert");
				return false;
			}
			return true;
		}
		private void OnRemoveSloServices (object sender, EventArgs e)
		{
			if (LstSlo.SelectedRows.Count > 0) {
				foreach (var row in LstSlo.SelectedRows) {
					ExternalIdentityProviderDto.SloServices.RemoveAt ((int)row);
				}
				var datasource = new ServiceEndpointDataSource { Entries = ExternalIdentityProviderDto.SloServices };
				LstSlo.DataSource = datasource;
				LstSlo.ReloadData ();
			}
		}

		public new AddNewExternalIdentityProvider Window {
			get { return (AddNewExternalIdentityProvider)base.Window; }
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
	}
}

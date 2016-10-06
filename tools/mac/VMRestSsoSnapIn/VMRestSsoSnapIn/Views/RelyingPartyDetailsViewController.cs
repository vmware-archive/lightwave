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
using System.Text;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn;
using VmIdentity.CommonUtils.Utilities;

namespace RestSsoAdminSnapIn
{
	public partial class RelyingPartyDetailsViewController : AppKit.NSViewController
	{
		public RelyingPartyDto RelyingPartyDtoOriginal;
		public ServerDto ServerDto;
		public string TenantName;
		private RelyingPartyDto RelyingPartyDto;
		private X509Certificate2 _certificate;
		#region Constructors

		// Called when created from unmanaged code
		public RelyingPartyDetailsViewController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}

		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public RelyingPartyDetailsViewController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}

		// Call to load from the XIB/NIB file
		public RelyingPartyDetailsViewController () : base ("RelyingPartyDetailsView", NSBundle.MainBundle)
		{
			Initialize ();
		}

		// Shared initialization code
		void Initialize ()
		{
		}

		#endregion

		//strongly typed view accessor
		public new RelyingPartyDetailsView View {
			get {
				return (RelyingPartyDetailsView)base.View;
			}
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			RelyingPartyDto = RelyingPartyDtoOriginal.DeepCopy ();
			TxtRpName.StringValue = string.IsNullOrEmpty(RelyingPartyDto.Name) ? string.Empty : 
				RelyingPartyDto.Name.Length > 10 ? RelyingPartyDto.Name.Substring(0,10) + "..." : 
				RelyingPartyDto.Name;
			TxtUrl.StringValue = string.IsNullOrEmpty (RelyingPartyDto.Url) ? string.Empty : RelyingPartyDto.Url;
			TxtCertificate.StringValue = "Certificate";
			_certificate = new X509Certificate2 (Encoding.ASCII.GetBytes(RelyingPartyDtoOriginal.Certificate.Encoded));
			this.BtnAddSignAlgo.Activated += OnAddSignatureAlgorithm;
			this.BtnAddAssertServices.Activated += OnAddAssertServices;
			this.BtnAddAttributeService.Activated += OnAddAttributeServices;
			this.BtnAddSloService.Activated += OnAddSloServices;

			this.BtnRemoveSignAlgo.Activated += OnRemoveSignatureAlgorithm;
			this.BtnRemoveAssertService.Activated += OnRemoveAssertServices;
			this.BtnRemoveAttributeService.Activated += OnRemoveAttributeServices;
			this.BtnRemoveSloService.Activated += OnRemoveSloServices;

			this.SignAlgorithmTableView.DoubleClick += OnSignatureAlgorithmUpdate;
			this.SloServicesTableView.DoubleClick += OnSloUpdate;
			this.AttributeTableView.DoubleClick += OnAttributeUpdate;
			this.AssertTableView.DoubleClick += OnAssertUpdate;

			this.BtnApply.Activated += OnClickSaveButton;
			this.BtnBrowseCertificate.Activated += OnBrowseCertificate;
			this.BtnViewCertificate.Activated += (object sender, EventArgs e) => 
			{
				ActionHelper.Execute (delegate() {
					var cert = new X509Certificate2 (Encoding.ASCII.GetBytes(RelyingPartyDtoOriginal.Certificate.Encoded));
					CertificateService.DisplayX509Certificate2(this, cert);
				});
			};

			InitializeSignatureAlgorithm ();
			InitializeAssertionConsumerServices ();
			InitializeAttributeConsumerServices ();
			InitializeSloServices ();

		}

		public void OnSignatureAlgorithmUpdate (object sender, EventArgs e)
		{
			if(SignAlgorithmTableView.SelectedRows != null && (int)SignAlgorithmTableView.SelectedRows.Count > 0)
			{
				var row = (int)SignAlgorithmTableView.SelectedRows.FirstIndex;
				var dto = RelyingPartyDto.SignatureAlgorithms [row];
				NSApplication.SharedApplication.StopModal ();
				var form = new AddNewSignatureAlgorithmController (){SignatureAlgorithmDto = dto};
				NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (form.IsUpdated != null) {
					RelyingPartyDto.SignatureAlgorithms.RemoveAt (row);
					RelyingPartyDto.SignatureAlgorithms.Add (form.SignatureAlgorithmDto);
					var datasource = new SignatureAlgorithmDataSource { Entries = RelyingPartyDto.SignatureAlgorithms };
					SignAlgorithmTableView.DataSource = datasource;
					SignAlgorithmTableView.ReloadData ();
				}
			}
		}
		public void OnSloUpdate (object sender, EventArgs e)
		{
			if (SloServicesTableView.SelectedRows != null && (int)SloServicesTableView.SelectedRows.Count > 0) {
				var row = (int)SloServicesTableView.SelectedRows.FirstIndex;
				var dto = RelyingPartyDto.SingleLogoutServices [row];
				NSApplication.SharedApplication.StopModal ();
				var form = new AddNewServiceEndpointController (){ServiceEndpointDto = dto };
				NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (form.IsUpdated != null) {
					RelyingPartyDto.SingleLogoutServices.RemoveAt (row);
					RelyingPartyDto.SingleLogoutServices.Add (form.ServiceEndpointDto);
					var datasource = new ServiceEndpointDataSource { Entries = RelyingPartyDto.SingleLogoutServices };
					SloServicesTableView.DataSource = datasource;
					SloServicesTableView.ReloadData ();
				}
			}
		}
		public void OnAttributeUpdate (object sender, EventArgs e)
		{
			if (AttributeTableView.SelectedRows != null && (int)AttributeTableView.SelectedRows.Count > 0) {
				var row = (int)AttributeTableView.SelectedRows.FirstIndex;
				var dto = RelyingPartyDto.AttributeConsumerServices [row];
				NSApplication.SharedApplication.StopModal ();
				var defaultExists = RelyingPartyDto.AttributeConsumerServices.Exists (x => x.IsDefault);
				var form = new AddNewAttributeConsumerServiceController (){ DefaultSet = defaultExists, AttributeConsumerServiceDto = dto };
				NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (form.IsUpdated != null) {
					RelyingPartyDto.AttributeConsumerServices.RemoveAt (row);
					RelyingPartyDto.AttributeConsumerServices.Add (form.AttributeConsumerServiceDto);
					var datasource = new AttributeConsumerServiceDataSource { Entries = RelyingPartyDto.AttributeConsumerServices };
					AttributeTableView.DataSource = datasource;
					AttributeTableView.ReloadData ();
				}
			}
		}
		public void OnAssertUpdate (object sender, EventArgs e)
		{
			if (AssertTableView.SelectedRows != null && (int)AssertTableView.SelectedRows.Count > 0) {
				var row = (int)AssertTableView.SelectedRows.FirstIndex;
				var dto = RelyingPartyDto.AssertionConsumerServices [row];
				NSApplication.SharedApplication.StopModal ();
				var defaultExists = RelyingPartyDto.AssertionConsumerServices.Exists (x => x.IsDefault);
				var form = new AddNewAssertionConsumerServiceController (){ DefaultSet = defaultExists, AssertionConsumerServiceDto = dto };
				NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (form.IsUpdated != null) {
					RelyingPartyDto.AssertionConsumerServices.RemoveAt (row);
					RelyingPartyDto.AssertionConsumerServices.Add (form.AssertionConsumerServiceDto);
					var datasource = new AssertionConsumerServiceDataSource { Entries = RelyingPartyDto.AssertionConsumerServices };
					AssertTableView.DataSource = datasource;
					AssertTableView.ReloadData ();
				}
			}
		}

		public void OnRemoveSignatureAlgorithm (object sender, EventArgs e)
		{
			if (RelyingPartyDto.SignatureAlgorithms!= null && RelyingPartyDto.SignatureAlgorithms.Count > 0) {
				if (SignAlgorithmTableView.SelectedRows != null && SignAlgorithmTableView.SelectedRows.Count > 0) {
					foreach (var row in SignAlgorithmTableView.SelectedRows) {
						if (row > 0 && (int)row < RelyingPartyDto.SignatureAlgorithms.Count)
							RelyingPartyDto.SignatureAlgorithms.RemoveAt ((int)row);
					}
					var datasource = new SignatureAlgorithmDataSource { Entries = RelyingPartyDto.SignatureAlgorithms };
					SignAlgorithmTableView.DataSource = datasource;
					SignAlgorithmTableView.ReloadData ();
				}
			}
		}

		void OnBrowseCertificate (object sender, EventArgs e)
		{
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
					_certificate = cert;
				});
				TxtCertificate.StringValue = filePath;
			}
		}
		public void OnRemoveAssertServices (object sender, EventArgs e)
		{
			if (RelyingPartyDto.AssertionConsumerServices != null &&
			   RelyingPartyDto.AssertionConsumerServices.Count > 0) {
				if (AssertTableView.SelectedRows != null && AssertTableView.SelectedRows.Count > 0) {
					foreach (var row in AssertTableView.SelectedRows) {
						if (row > 0 && (int)row < RelyingPartyDto.AssertionConsumerServices.Count)
							RelyingPartyDto.AssertionConsumerServices.RemoveAt ((int)row);
					}
					var datasource = new AssertionConsumerServiceDataSource { Entries = RelyingPartyDto.AssertionConsumerServices };
					AssertTableView.DataSource = datasource;
					AssertTableView.ReloadData ();
				}
			}
		}
		public void OnRemoveAttributeServices (object sender, EventArgs e)
		{
			if (RelyingPartyDto.AttributeConsumerServices != null &&
			   RelyingPartyDto.AttributeConsumerServices.Count > 0) {
				if (AttributeTableView.SelectedRows != null && AttributeTableView.SelectedRows.Count > 0) {
					foreach (var row in AttributeTableView.SelectedRows) {
						if (row > 0 && (int)row < RelyingPartyDto.AttributeConsumerServices.Count)
							RelyingPartyDto.AttributeConsumerServices.RemoveAt ((int)row);
					}
					var datasource = new AttributeConsumerServiceDataSource { Entries = RelyingPartyDto.AttributeConsumerServices };
					AttributeTableView.DataSource = datasource;
					AttributeTableView.ReloadData ();
				}
			}
		}
		public void OnRemoveSloServices (object sender, EventArgs e)
		{
			if (RelyingPartyDto.SingleLogoutServices != null &&
			   RelyingPartyDto.SingleLogoutServices.Count > 0) {
				if (SloServicesTableView.SelectedRows != null && SloServicesTableView.SelectedRows.Count > 0) {
					foreach (var row in SloServicesTableView.SelectedRows) {
						if (row > 0 && (int)row < RelyingPartyDto.SingleLogoutServices.Count)
							RelyingPartyDto.SingleLogoutServices.RemoveAt ((int)row);
					}
					var datasource = new ServiceEndpointDataSource { Entries = RelyingPartyDto.SingleLogoutServices };
					SloServicesTableView.DataSource = datasource;
					SloServicesTableView.ReloadData ();
				}
			}
		}
		public void OnAddSignatureAlgorithm (object sender, EventArgs e)
		{	
			NSApplication.SharedApplication.StopModal ();
			var form = new AddNewSignatureAlgorithmController ();
			NSApplication.SharedApplication.RunModalForWindow (form.Window);
			if (form.SignatureAlgorithmDto != null) {
				RelyingPartyDto.SignatureAlgorithms.Add (form.SignatureAlgorithmDto);
				var datasource = new SignatureAlgorithmDataSource { Entries = RelyingPartyDto.SignatureAlgorithms };
				SignAlgorithmTableView.DataSource = datasource;
				SignAlgorithmTableView.ReloadData ();
			}
		}
		public void OnAddAssertServices (object sender, EventArgs e)
		{
			NSApplication.SharedApplication.StopModal ();
			var defaultExists = RelyingPartyDto.AssertionConsumerServices.Exists (x => x.IsDefault);
			var form = new AddNewAssertionConsumerServiceController (){ DefaultSet = defaultExists };
			NSApplication.SharedApplication.RunModalForWindow (form.Window);
			if (form.AssertionConsumerServiceDto != null) {
				RelyingPartyDto.AssertionConsumerServices.Add (form.AssertionConsumerServiceDto);
				var datasource = new AssertionConsumerServiceDataSource { Entries = RelyingPartyDto.AssertionConsumerServices };
				AssertTableView.DataSource = datasource;
				AssertTableView.ReloadData ();
			}
		}
		public void OnAddAttributeServices (object sender, EventArgs e)
		{
			NSApplication.SharedApplication.StopModal ();
			var defaultExists = RelyingPartyDto.AttributeConsumerServices.Exists (x => x.IsDefault);
			var form = new AddNewAttributeConsumerServiceController (){ DefaultSet = defaultExists };
			NSApplication.SharedApplication.RunModalForWindow (form.Window);
			if (form.AttributeConsumerServiceDto != null) {
				RelyingPartyDto.AttributeConsumerServices.Add (form.AttributeConsumerServiceDto);
				var datasource = new AttributeConsumerServiceDataSource { Entries = RelyingPartyDto.AttributeConsumerServices };
				AttributeTableView.DataSource = datasource;
				AttributeTableView.ReloadData ();
			}
		}
		public void OnAddSloServices (object sender, EventArgs e)
		{
			NSApplication.SharedApplication.StopModal ();
			var form = new AddNewServiceEndpointController ();
			NSApplication.SharedApplication.RunModalForWindow (form.Window);
			if (form.ServiceEndpointDto != null) {
				RelyingPartyDto.SingleLogoutServices.Add (form.ServiceEndpointDto);
				var datasource = new ServiceEndpointDataSource { Entries = RelyingPartyDto.SingleLogoutServices };
				SloServicesTableView.DataSource = datasource;
				SloServicesTableView.ReloadData ();
			}
		}

		public void OnClickSaveButton (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				if (string.IsNullOrEmpty (TxtUrl.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid Url", "Alert");
				} else if (string.IsNullOrEmpty (TxtCertificate.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid Certificate path", "Alert");
				} else if (!string.IsNullOrEmpty (TxtCertificate.StringValue) && TxtCertificate.StringValue != "Certificate"  
					&& !System.IO.File.Exists (TxtCertificate.StringValue.Replace ("file://", string.Empty))) {
					UIErrorHelper.ShowAlert ("Certificate path is not valid", "Alert");
				} else {
					var encoded = string.Empty;
					var cert = new X509Certificate2 ();
					ActionHelper.Execute (delegate() {
						if(TxtCertificate.StringValue == "Certificate")
						{
							encoded = _certificate.ExportToPem ();
						}
						else 
						{	
							cert.Import (TxtCertificate.StringValue.Replace ("file://", string.Empty));
							encoded = cert.ExportToPem ();
						}
					
						RelyingPartyDto.Name = RelyingPartyDtoOriginal.Name;
						RelyingPartyDto.Certificate = new CertificateDto { Encoded =  encoded};
						RelyingPartyDto.Url = TxtUrl.StringValue;

						var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
						RelyingPartyDto = SnapInContext.Instance.ServiceGateway.RelyingParty.Update (ServerDto, TenantName, RelyingPartyDto, auth.Token);

						NSNotificationCenter.DefaultCenter.PostNotificationName ("RefreshTableView", this);
					});
				}
			});
		}

		private void InitializeSignatureAlgorithm()
		{
			foreach(NSTableColumn column in SignAlgorithmTableView.TableColumns())
			{
				SignAlgorithmTableView.RemoveColumn (column);
			}
			SignAlgorithmTableView.Delegate = new TableDelegate ();
			var listView = new SignatureAlgorithmDataSource { Entries = RelyingPartyDto.SignatureAlgorithms };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "MaxKeySize", DisplayName = "Max Key Size", DisplayOrder = 1, Width = 80 },
				new ColumnOptions{ Id = "MinKeySize", DisplayName = "Min Key Size", DisplayOrder = 2, Width = 80 },
				new ColumnOptions{ Id = "Priority", DisplayName = "Priority", DisplayOrder = 3, Width = 80 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				SignAlgorithmTableView.AddColumn (column);
			}
			SignAlgorithmTableView.DataSource = listView;
			SignAlgorithmTableView.ReloadData ();
		}

		private void InitializeAssertionConsumerServices()
		{
			foreach(NSTableColumn column in AssertTableView.TableColumns())
			{
				AssertTableView.RemoveColumn (column);
			}
			AssertTableView.Delegate = new TableDelegate ();
			var listView = new AssertionConsumerServiceDataSource { Entries = RelyingPartyDto.AssertionConsumerServices };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 1, Width = 80 },
				new ColumnOptions{ Id = "Index", DisplayName = "Index", DisplayOrder = 2, Width = 80 },
				new ColumnOptions{ Id = "IsDefault", DisplayName = "Default", DisplayOrder = 3, Width = 80 },
				new ColumnOptions{ Id = "Endpoint", DisplayName = "Endpoint", DisplayOrder = 4, Width = 150 },
				new ColumnOptions{ Id = "Binding", DisplayName = "Binding", DisplayOrder = 5, Width = 150 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				AssertTableView.AddColumn (column);
			}
			AssertTableView.DataSource = listView;
			AssertTableView.ReloadData ();
		}

		private void InitializeSloServices()
		{
			foreach(NSTableColumn column in SloServicesTableView.TableColumns())
			{
				SloServicesTableView.RemoveColumn (column);
			}
			SloServicesTableView.Delegate = new TableDelegate ();
			var listView = new ServiceEndpointDataSource { Entries = RelyingPartyDto.SingleLogoutServices };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 1, Width = 80 },
				new ColumnOptions{ Id = "Endpoint", DisplayName = "Endpoint", DisplayOrder = 4, Width = 150 },
				new ColumnOptions{ Id = "Binding", DisplayName = "Binding", DisplayOrder = 5, Width = 150 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				SloServicesTableView.AddColumn (column);
			}
			SloServicesTableView.DataSource = listView;
			SloServicesTableView.ReloadData ();
		}

		private void InitializeAttributeConsumerServices()
		{
			foreach(NSTableColumn column in AttributeTableView.TableColumns())
			{
				AttributeTableView.RemoveColumn (column);
			}
			AttributeTableView.Delegate = new TableDelegate ();
			var listView = new AttributeConsumerServiceDataSource { Entries = RelyingPartyDto.AttributeConsumerServices };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 1, Width = 80 },
				new ColumnOptions{ Id = "Index", DisplayName = "Index", DisplayOrder = 2, Width = 80 },
				new ColumnOptions{ Id = "IsDefault", DisplayName = "Default", DisplayOrder = 3, Width = 80 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				AttributeTableView.AddColumn (column);
			}
			AttributeTableView.DataSource = listView;
			AttributeTableView.ReloadData ();
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

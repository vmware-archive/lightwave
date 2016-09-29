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
using VmIdentity.CommonUtils.Utilities;
using Vmware.Tools.RestSsoAdminSnapIn;

namespace RestSsoAdminSnapIn
{
	public partial class AddRelyingPartyController : AppKit.NSWindowController
	{
		public RelyingPartyDto RelyingPartyDto{ get; set;}
		public ServerDto ServerDto{ get; set;}
		public string Tenant{ get; set;}
		public bool SaveSuccessful;

		#region Constructors

		// Called when created from unmanaged code
		public AddRelyingPartyController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public AddRelyingPartyController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public AddRelyingPartyController () : base ("AddRelyingParty")
		{
			Initialize ();
		}
		
		// Shared initialization code
		void Initialize ()
		{
			SaveSuccessful = false;
			if (RelyingPartyDto == null)
				RelyingPartyDto = new RelyingPartyDto ();
			RelyingPartyDto.SignatureAlgorithms = new List<SignatureAlgorithmDto> ();
			RelyingPartyDto.AssertionConsumerServices = new List<AssertionConsumerServiceDto> ();
			RelyingPartyDto.AttributeConsumerServices = new List<AttributeConsumerServiceDto> ();
			RelyingPartyDto.SingleLogoutServices = new List<ServiceEndpointDto> ();
		}

		#endregion

		//strongly typed window accessor
		public new AddRelyingParty Window {
			get {
				return (AddRelyingParty)base.Window;
			}
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();

			//Events
			this.BtnSave.Activated += OnClickAddButton;
			this.BtnBrowseCertificate.Activated += OnClickBrowseButton;
			this.BtnClose.Activated += (object sender, EventArgs e) => {
				RelyingPartyDto = null;
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};

			this.BtnAddSignatureAlgorithm.Activated += OnAddSignatureAlgorithm;
			this.BtnAddAssertServices.Activated += OnAddAssertServices;
			this.BtnAddAttributeServices.Activated += OnAddAttributeServices;
			this.BtnAddSlo.Activated += OnAddSloServices;

			this.BtnRemoveSignatureAlgorithm.Activated += OnRemoveSignatureAlgorithm;
			this.BtnRemoveAssertServices.Activated += OnRemoveAssertServices;
			this.BtnRemoveAttributeServices.Activated += OnRemoveAttributeServices;
			this.BtnRemoveSlo.Activated += OnRemoveSloServices;

			this.SignAlgoTableView.DoubleClick += OnSignatureAlgorithmUpdate;
			this.SloTableView.DoubleClick += OnSloUpdate;
			this.AttributeTableView.DoubleClick += OnAttributeUpdate;
			this.AssertionTableView.DoubleClick += OnAssertUpdate;

			InitializeSignatureAlgorithm ();
			InitializeAssertionConsumerServices ();
			InitializeAttributeConsumerServices ();
			InitializeSloServices ();
		}

		public void OnSignatureAlgorithmUpdate (object sender, EventArgs e)
		{
			if(SignAlgoTableView.SelectedRows != null && (int)SignAlgoTableView.SelectedRows.Count > -1)
			{
				var row = (int)SignAlgoTableView.SelectedRows.FirstIndex;
				var dto = RelyingPartyDto.SignatureAlgorithms [row];
				NSApplication.SharedApplication.StopModal ();
				var form = new AddNewSignatureAlgorithmController (){SignatureAlgorithmDto = dto};
				NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (form.IsUpdated != null) {
					RelyingPartyDto.SignatureAlgorithms.RemoveAt (row);
					RelyingPartyDto.SignatureAlgorithms.Add (form.SignatureAlgorithmDto);
					var datasource = new SignatureAlgorithmDataSource { Entries = RelyingPartyDto.SignatureAlgorithms };
					SignAlgoTableView.DataSource = datasource;
					SignAlgoTableView.ReloadData ();
				}
			}
		}
		public void OnSloUpdate (object sender, EventArgs e)
		{
			if (SloTableView.SelectedRows != null && (int)SloTableView.SelectedRows.Count > -1) {
				var row = (int)SloTableView.SelectedRows.FirstIndex;
				var dto = RelyingPartyDto.SingleLogoutServices [row];
				NSApplication.SharedApplication.StopModal ();
				var form = new AddNewServiceEndpointController (){ServiceEndpointDto = dto };
				NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (form.IsUpdated != null) {
					RelyingPartyDto.SingleLogoutServices.RemoveAt (row);
					RelyingPartyDto.SingleLogoutServices.Add (form.ServiceEndpointDto);
					var datasource = new ServiceEndpointDataSource { Entries = RelyingPartyDto.SingleLogoutServices };
					SloTableView.DataSource = datasource;
					SloTableView.ReloadData ();
				}
			}
		}
		public void OnAttributeUpdate (object sender, EventArgs e)
		{
			if (AttributeTableView.SelectedRows != null && (int)AttributeTableView.SelectedRows.Count > -1) {
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
			if (AssertionTableView.SelectedRows != null && (int)AssertionTableView.SelectedRows.Count > -1) {
				var row = (int)AssertionTableView.SelectedRows.FirstIndex;
				var dto = RelyingPartyDto.AssertionConsumerServices [row];
				NSApplication.SharedApplication.StopModal ();
				var defaultExists = RelyingPartyDto.AssertionConsumerServices.Exists (x => x.IsDefault);
				var form = new AddNewAssertionConsumerServiceController (){ DefaultSet = defaultExists, AssertionConsumerServiceDto = dto };
				NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (form.IsUpdated != null) {
					RelyingPartyDto.AssertionConsumerServices.RemoveAt (row);
					RelyingPartyDto.AssertionConsumerServices.Add (form.AssertionConsumerServiceDto);
					var datasource = new AssertionConsumerServiceDataSource { Entries = RelyingPartyDto.AssertionConsumerServices };
					AssertionTableView.DataSource = datasource;
					AssertionTableView.ReloadData ();
				}
			}
		}

		public void OnRemoveSignatureAlgorithm (object sender, EventArgs e)
		{
			if (SignAlgoTableView.SelectedRows != null && SignAlgoTableView.SelectedRows.Count > 0) {
				foreach (var row in SignAlgoTableView.SelectedRows) {
					RelyingPartyDto.SignatureAlgorithms.RemoveAt ((int)row);
				}
				var datasource = new SignatureAlgorithmDataSource { Entries = RelyingPartyDto.SignatureAlgorithms };
				SignAlgoTableView.DataSource = datasource;
				SignAlgoTableView.ReloadData ();
			}
		}
		public void OnRemoveAssertServices (object sender, EventArgs e)
		{
			if (AssertionTableView.SelectedRows != null && AssertionTableView.SelectedRows.Count > 0) {
				foreach (var row in AssertionTableView.SelectedRows) {
					RelyingPartyDto.AssertionConsumerServices.RemoveAt ((int)row);
				}
				var datasource = new AssertionConsumerServiceDataSource { Entries = RelyingPartyDto.AssertionConsumerServices };
				AssertionTableView.DataSource = datasource;
				AssertionTableView.ReloadData ();
			}
		}
		public void OnRemoveAttributeServices (object sender, EventArgs e)
		{
			if (AttributeTableView.SelectedRows != null && AttributeTableView.SelectedRows.Count > 0) {
				foreach (var row in AttributeTableView.SelectedRows) {
					RelyingPartyDto.AttributeConsumerServices.RemoveAt ((int)row);
				}
				var datasource = new AttributeConsumerServiceDataSource { Entries = RelyingPartyDto.AttributeConsumerServices };
				AttributeTableView.DataSource = datasource;
				AttributeTableView.ReloadData ();
			}
		}
		public void OnRemoveSloServices (object sender, EventArgs e)
		{
			if (SloTableView.SelectedRows != null && SloTableView.SelectedRows.Count > 0) {
				foreach (var row in SloTableView.SelectedRows) {
					RelyingPartyDto.SingleLogoutServices.RemoveAt ((int)row);
				}
				var datasource = new ServiceEndpointDataSource { Entries = RelyingPartyDto.SingleLogoutServices };
				SloTableView.DataSource = datasource;
				SloTableView.ReloadData ();
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
				SignAlgoTableView.DataSource = datasource;
				SignAlgoTableView.ReloadData ();
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
				AssertionTableView.DataSource = datasource;
				AssertionTableView.ReloadData ();
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
				SloTableView.DataSource = datasource;
				SloTableView.ReloadData ();
			}
		}
		private void InitializeSignatureAlgorithm()
		{
			foreach(NSTableColumn column in SignAlgoTableView.TableColumns())
			{
				SignAlgoTableView.RemoveColumn (column);
			}
			SignAlgoTableView.Delegate = new TableDelegate ();
			var listView = new SignatureAlgorithmDataSource { Entries = RelyingPartyDto.SignatureAlgorithms };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "MaxKeySize", DisplayName = "Max Key Size", DisplayOrder = 1, Width = 80 },
				new ColumnOptions{ Id = "MinKeySize", DisplayName = "Min Key Size", DisplayOrder = 2, Width = 80 },
				new ColumnOptions{ Id = "Priority", DisplayName = "Priority", DisplayOrder = 3, Width = 80 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				SignAlgoTableView.AddColumn (column);
			}
			SignAlgoTableView.DataSource = listView;
			SignAlgoTableView.ReloadData ();
		}

		private void InitializeAssertionConsumerServices()
		{
			foreach(NSTableColumn column in AssertionTableView.TableColumns())
			{
				AssertionTableView.RemoveColumn (column);
			}
			AssertionTableView.Delegate = new TableDelegate ();
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
				AssertionTableView.AddColumn (column);
			}
			AssertionTableView.DataSource = listView;
			AssertionTableView.ReloadData ();
		}

		private void InitializeSloServices()
		{
			foreach(NSTableColumn column in SloTableView.TableColumns())
			{
				SloTableView.RemoveColumn (column);
			}
			SloTableView.Delegate = new TableDelegate ();
			var listView = new AssertionConsumerServiceDataSource { Entries = RelyingPartyDto.AssertionConsumerServices };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 1, Width = 80 },
				new ColumnOptions{ Id = "Endpoint", DisplayName = "Endpoint", DisplayOrder = 4, Width = 150 },
				new ColumnOptions{ Id = "Binding", DisplayName = "Binding", DisplayOrder = 5, Width = 150 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				SloTableView.AddColumn (column);
			}
			SloTableView.DataSource = listView;
			SloTableView.ReloadData ();
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

		public void OnClickAddButton (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				if (string.IsNullOrEmpty (TxtName.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid name", "Alert");
				} else if(!string.IsNullOrEmpty (TxtName.StringValue) && TxtName.StringValue.Contains("/"))
					{
						UIErrorHelper.ShowAlert ("Relying party name contains and invalid character.", "Alert");
					}
					else if (string.IsNullOrEmpty (TxtUrl.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid Url", "Alert");
				} else if (!string.IsNullOrEmpty (TxtCertificate.StringValue))
				if (!System.IO.File.Exists (TxtCertificate.StringValue.Replace ("file://", string.Empty))) {
					UIErrorHelper.ShowAlert ("Certificate path is not valid", "Alert");
				} else {
					var cert = new X509Certificate2 ();
					ActionHelper.Execute (delegate() {
						cert.Import (TxtCertificate.StringValue.Replace ("file://", string.Empty));
					});
					RelyingPartyDto.Name = TxtName.StringValue;
					RelyingPartyDto.Certificate = new CertificateDto { Encoded = cert.ExportToPem () };
					RelyingPartyDto.Url = TxtUrl.StringValue;

					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
					RelyingPartyDto = SnapInContext.Instance.ServiceGateway.RelyingParty.Create (ServerDto, Tenant, RelyingPartyDto, auth.Token);
					SaveSuccessful = true;
					this.Close ();
					NSApplication.SharedApplication.StopModalWithCode (1);
				}
			});
		}

		public void OnClickBrowseButton (object sender, EventArgs e)
		{
			var openPanel = new NSOpenPanel ();
			openPanel.ReleasedWhenClosed = true;
			openPanel.Prompt = "Select file";

			var result = openPanel.RunModal ();
			if (result == 1) {
				var filePath = Uri.UnescapeDataString (openPanel.Url.AbsoluteString.Replace ("file://", string.Empty));
				var cert = new X509Certificate2 ();
				try {

					cert.Import (filePath);
				} catch (Exception) {
					UIErrorHelper.ShowAlert ("Invalid X509 certificate", "Alert");
				}
				TxtCertificate.StringValue = filePath;
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
	}
}


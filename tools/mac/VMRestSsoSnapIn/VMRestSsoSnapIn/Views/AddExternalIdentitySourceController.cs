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
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Service.IdentityProvider;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Dto.Attributes;
using VmIdentity.CommonUtils.Utilities;

namespace RestSsoAdminSnapIn
{
	public partial class AddExternalIdentitySourceController : AppKit.NSWindowController
	{
		public IdentityProviderDto IdentityProviderDto;
		private WizardSteps _currentStep;
		private List<CertificateDto> _certificates;
		public ServerDto ServerDto { get; set; }
		public string TenantName { get; set; }
		private string _domainName;
		private bool isUpdate ;

		#region Constructors

		// Called when created from unmanaged code
		public AddExternalIdentitySourceController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public AddExternalIdentitySourceController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public AddExternalIdentitySourceController () : base ("AddExternalIdentitySource")
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

			_certificates = new List<CertificateDto> ();
			var title = string.Empty;
			if (IdentityProviderDto != null) {
				var tag = GetIdFromIdentitySourceType (IdentityProviderDto.Type);

				if (tag == 1) {
					title = VMIdentityConstants.AD_WIN_AUTH_TITLE;
					_currentStep = WizardSteps.Four;
				} else {
					if (tag == 2) {
						title = VMIdentityConstants.AD_AS_LDAP_TITLE;
					}
					else{
						title = VMIdentityConstants.OPEN_LDAP_TITLE;
					}
					_currentStep = WizardSteps.Two;
				}

				isUpdate = true;
			} else {
				title = VMIdentityConstants.NEW_EXTERNAL_DOMAIN_TITLE;
				_currentStep = WizardSteps.One;
				isUpdate = false;
			}
			this.Window.Title = (NSString)title;
			SetWizardStep ();
			ReloadCertificates ();

			//Events
			this.BtnTestConnection.Activated += TestConnection;
			this.BtnNext.Activated += OnClickNextButton;
			this.BtnBack.Activated += OnClickBackButton;
			this.BtnAddCertificate.Activated +=	(object sender, EventArgs e) => {
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
						var certfificateDto = new CertificateDto { Encoded = cert.ExportToPem(), Chain = cert.GetFormattedThumbPrint()};
						_certificates.Add(certfificateDto);
						ReloadCertificates();
					});
				}
			};

			this.RdoIdentitySource.Activated += (object sender, EventArgs e) => 
			{
				SetSpnControls();
			};
			this.RdoDomainController.Activated += (object sender, EventArgs e) => 
			{
				var anyDc = RdoDomainController.SelectedTag == 1;
				if(anyDc)
				{
					SetConnectionString();
				}
				else
				{
					TxtLdapConnection.StringValue = (NSString) string.Empty;
				}
				ChkProtect.Enabled = anyDc;
				EnableDisableConnectionString(!anyDc);
			};
			this.BtnRemoveCertificate.Activated += (object sender, EventArgs e) => {
				if (LstCertificates.SelectedRows.Count > 0) {
					foreach (var row in LstCertificates.SelectedRows) {
						_certificates.RemoveAt ((int)row);
					}
					ReloadCertificates();
				}
			};
			this.BtnPrimaryImport.Activated += (object sender, EventArgs e) => {
				
			};

			this.BtnSecondaryImport.Activated += (object sender, EventArgs e) => {

			};
			this.TxtDomainName.Changed += (object sender, EventArgs e) => {
				SetConnectionString();
			};

			this.ChkProtect.Activated += (object sender, EventArgs e) => {
				SetConnectionString();
			};
			this.RdoSpn.Activated += (object sender, EventArgs e) => {
				SetSpnControls();
			};
			BtnPrimaryImport.Enabled = false;
			BtnSecondaryImport.Enabled = false;
			this.TxtPrimaryUrl.Activated += (object sender, EventArgs e) => 
			{
				BtnPrimaryImport.Enabled = this.TxtPrimaryUrl.StringValue!= null && this.TxtPrimaryUrl.StringValue.StartsWith("ldaps://");
			};
			this.TxtSecondaryConnection.Activated += (object sender, EventArgs e) => 
			{
				BtnSecondaryImport.Enabled = this.TxtSecondaryConnection.StringValue!= null && this.TxtSecondaryConnection.StringValue.StartsWith("ldaps://");
			};
			BtnPrimaryImport.Activated += (object sender, EventArgs e) => 
			{
				ImportCertificates(TxtPrimaryUrl.StringValue);
			};
			BtnSecondaryImport.Activated += (object sender, EventArgs e) => 
			{
				ImportCertificates(TxtSecondaryConnection.StringValue);
			};
			if (IdentityProviderDto != null)
				DtoToView ();
			else
				IdentityProviderDto = new IdentityProviderDto ();
			this.BtnAdvanced.Activated += (object sender, EventArgs e) => 
			{
				var form = new ExternalDomainAdvancedSettingsController ()
				{
					IdentityProviderDto = new IdentityProviderDto
					{
						Schema = IdentityProviderDto.Schema == null ? new Dictionary<string, SchemaObjectMappingDto>() :new Dictionary<string, SchemaObjectMappingDto>(IdentityProviderDto.Schema),
						AttributesMap = IdentityProviderDto.AttributesMap == null ?  new Dictionary<string, string>() : new Dictionary<string, string>(IdentityProviderDto.AttributesMap),
						BaseDnForNestedGroupsEnabled = IdentityProviderDto.BaseDnForNestedGroupsEnabled,
						MatchingRuleInChainEnabled = IdentityProviderDto.MatchingRuleInChainEnabled,
						DirectGroupsSearchEnabled = IdentityProviderDto.DirectGroupsSearchEnabled
					}
				};
				var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);

				if(result == 1)
				{
					IdentityProviderDto.Schema = GetSchema(form.IdentityProviderDto.Schema);
					IdentityProviderDto.AttributesMap = new Dictionary<string, string>(form.IdentityProviderDto.AttributesMap);
					IdentityProviderDto.BaseDnForNestedGroupsEnabled = form.IdentityProviderDto.BaseDnForNestedGroupsEnabled;
					IdentityProviderDto.MatchingRuleInChainEnabled = form.IdentityProviderDto.MatchingRuleInChainEnabled;
					IdentityProviderDto.DirectGroupsSearchEnabled = form.IdentityProviderDto.DirectGroupsSearchEnabled;
				}
			};
			SetSpnControls ();
		}
		private void EnableDisableConnectionString(bool value)
		{
			TxtPrimaryConnection.Enabled = value;
			TxtSecondaryConnection.Enabled = value;
			BtnPrimaryImport.Enabled = value && TxtPrimaryConnection.StringValue.StartsWith("ldaps://");
			BtnSecondaryImport.Enabled = value && TxtSecondaryConnection.StringValue.StartsWith("ldaps://");
		}
		private void SetSpnControls()
		{
			var isSpn = (RdoIdentitySource.SelectedTag == 1 &&  RdoSpn.SelectedRow == 1);
			TxtSpn.Enabled = isSpn;
			TxtUpn.Enabled = RdoIdentitySource.SelectedTag > 1 || isSpn;
			TxtPassword.Enabled = RdoIdentitySource.SelectedTag > 1 || isSpn;
		}
		private void SetConnectionString()
		{
			if (_currentStep == WizardSteps.Two && RdoDomainController.SelectedTag == 1) {
				var ldaps = ChkProtect.StringValue == "1" ? "ldaps://" : "ldap://";
				TxtLdapConnection.StringValue = ldaps + TxtDomainName.StringValue;
			}
		}
		void ImportCertificates(string connection)
		{
			try
			{
				var xcert = LdapSecureConnectionCertificateFetcher.FetchServerCertificate(connection);
				var cert = new X509Certificate2(xcert);
				var thumbprint = cert.GetFormattedThumbPrint();
				var certfificateDto = new CertificateDto { Encoded = cert.ExportToPem(),Chain = thumbprint };

				var exists = _certificates.Exists(x=>x.Chain == thumbprint);
				if(exists)
				{
					UIErrorHelper.ShowAlert("Certificate with the same fingerprint already exists", "Error");
					return;
				}
				_certificates.Add(certfificateDto);
				ReloadCertificates ();
				UIErrorHelper.ShowAlert(string.Format("Certificate with subject {0} imported successfully", cert.Subject), "Information");
			}
			catch (Exception exception)
			{
				UIErrorHelper.ShowAlert (exception.Message, "Error");
			}
		}
		void TestConnection (object sender, EventArgs e)
		{
			ActionHelper.Execute(delegate {
				var dto = ViewToDto ();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto.ServerName);
				var returnDto = SnapInContext.Instance.ServiceGateway.IdentityProvider.Probe(ServerDto,TenantName,dto, auth.Token);
				if(returnDto != null)
				{
					var message = string.Format("Test connection successful.", returnDto.Name);
					UIErrorHelper.ShowAlert (message, "Information");
				}
			});
		}
		private void ReloadCertificates()
		{
			foreach(NSTableColumn column in LstCertificates.TableColumns())
			{
				LstCertificates.RemoveColumn (column);
			}
			LstCertificates.Delegate = new TableDelegate ();
			var listView = new TrustedCertificatesDataSource { Entries = _certificates };
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
		public void OnClickBackButton (object sender, EventArgs e)
		{	
			SetPreviousStep ();
			SetWizardStep ();
		}
		public void OnClickNextButton (object sender, EventArgs e)
		{	
			if (CurrentStepIsValid ()) {
				
				SetNextStep ();
				SetWizardStep ();

				if (_currentStep == WizardSteps.Save) {
					if (!Save ()) {
						_currentStep--;
						SetWizardStep ();
					}
				}
			}
		}

		private bool Save(){
			var dto = ViewToDto ();
			var result = false;
			ActionHelper.Execute(delegate {

				if(RdoIdentitySource.Enabled)
				{
					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto.ServerName);
					var returnDto = SnapInContext.Instance.ServiceGateway.IdentityProvider.Create(ServerDto,TenantName,dto, auth.Token);
					if(returnDto != null)
						{
							var message = string.Format("External domain {0} created successfully.", returnDto.Name);
							UIErrorHelper.ShowAlert (message, "Information");
							this.Close();
							NSApplication.SharedApplication.StopModalWithCode (0);
						}
				}
				else
				{
					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto.ServerName);
					var returnDto = SnapInContext.Instance.ServiceGateway.IdentityProvider.Update(ServerDto,TenantName,dto, auth.Token);
					if(returnDto != null)
					{
						var message = string.Format("External domain {0} updated successfully.", returnDto.Name);
						UIErrorHelper.ShowAlert (message, "Information");
						this.Close();
						NSApplication.SharedApplication.StopModalWithCode (0);
					}
				}
				result = true;
			});
			return result;
		}

		private bool CurrentStepIsValid()
		{
			var isAd = IsAdWinSelected ();
			if (_currentStep == WizardSteps.Four) {
				if (isAd) {
					if (RdoSpn.SelectedRow == 1) {
						if (string.IsNullOrEmpty (TxtSpn.StringValue)) {
							UIErrorHelper.ShowAlert ("Please enter valid Service Principal Name", "Alert");
							return false;
						} else if (string.IsNullOrEmpty (TxtUpn.StringValue)) {
							UIErrorHelper.ShowAlert ("Please enter valid User Principal Name", "Alert");
							return false;
						} else if (string.IsNullOrEmpty (TxtPassword.StringValue)) {
							UIErrorHelper.ShowAlert ("Please enter valid password", "Alert");
							return false;
						}
					}
				} else if (string.IsNullOrEmpty (TxtUpn.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid Username ", "Alert");
					return false;
				} else if (string.IsNullOrEmpty (TxtPassword.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid Password", "Alert");
					return false;
				}
			} else if (_currentStep == WizardSteps.Two) {
				if (string.IsNullOrEmpty (TxtDomainName.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid Domain Name", "Alert");
					return false;
				} else if (string.IsNullOrEmpty (TxtFriendlyName.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid Friendly Name", "Alert");
					return false;
				} else if (string.IsNullOrEmpty (TxtDomainAlias.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid Domain Alias", "Alert");
					return false;
				} else if (string.IsNullOrEmpty (TxtBaseDnUser.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid User base DN", "Alert");
					return false;
				} else if (string.IsNullOrEmpty (TxtBaseDnGroups.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid Group base DN", "Alert");
					return false;
				} else if (RdoDomainController.SelectedRow == 1 && string.IsNullOrEmpty (TxtPrimaryConnection.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid LDAP Primary URL", "Alert");
					return false;
				}
			} else if (_currentStep == WizardSteps.Three) {
				if (_certificates.Count () < 1) {
					UIErrorHelper.ShowAlert ("Please choose atleast one certificate", "Alert");
					return false;
				}
			} else if (_currentStep == WizardSteps.One) {
				if (RdoIdentitySource.SelectedTag == 1) {
					_domainName = GetDomainName ();
					if (string.IsNullOrEmpty (_domainName)) {
						UIErrorHelper.ShowAlert ("Node is not joined to AD", "Alert");
						return false;
					}
				}
			}

			return true;
		}

		private IdentityProviderDto ViewToDto()
		{
			var isAd = IsAdWinSelected();
			var useMachineAccount = (isAd && RdoSpn.SelectedRow != 1);
			//var useSpn = (isAd && RdoSpn.SelectedRow == 1);
			var providerDto = new IdentityProviderDto
			{
				DomainType = DomainType.EXTERNAL_DOMAIN.ToString(),
				Type = GetIdentitySourceType(RdoIdentitySource.SelectedTag),
				Name = isAd ? _domainName : TxtDomainName.StringValue,
				Alias = isAd ? null : TxtDomainAlias.StringValue,
				FriendlyName = isAd ? null : TxtFriendlyName.StringValue,
				UserBaseDN =  isAd ? null :  TxtBaseDnUser.StringValue,
				GroupBaseDN =  isAd ? null :  TxtBaseDnGroups.StringValue,
				ConnectionStrings =  isAd ? null : GetConnectionStrings(),
				SiteAffinityEnabled = RdoDomainController.SelectedTag == 1 && ChkProtect.StringValue == "1",
				Certificates = _certificates,
				UserMachineAccount = useMachineAccount,
				Username = useMachineAccount ? null : TxtUpn.StringValue,
				Password = useMachineAccount ? null : TxtPassword.StringValue,
				ServicePrincipalName = useMachineAccount ? null : TxtSpn.StringValue,
				AuthenticationType = isAd ? "USE_KERBEROS" : "PASSWORD",
				BaseDnForNestedGroupsEnabled = IdentityProviderDto.BaseDnForNestedGroupsEnabled,
				DirectGroupsSearchEnabled = IdentityProviderDto.DirectGroupsSearchEnabled,
				MatchingRuleInChainEnabled = IdentityProviderDto.MatchingRuleInChainEnabled,
				AttributesMap = IdentityProviderDto.AttributesMap != null && IdentityProviderDto.AttributesMap.Count > 0 ? IdentityProviderDto.AttributesMap : null,
				Schema = IdentityProviderDto.Schema // != null && IdentityProviderDto.Schema.Count > 0 ? GetSchema(IdentityProviderDto.Schema) : null
			};
			return providerDto;
		}

		private Dictionary<string,SchemaObjectMappingDto> GetSchema(Dictionary<string,SchemaObjectMappingDto> input)
		{
			var schema = new Dictionary<string,SchemaObjectMappingDto>();

			var user = ObjectId.ObjectIdUser.ToString ();
			var pass = ObjectId.ObjectIdPasswordSettings.ToString();
			var grp = ObjectId.ObjectIdGroup.ToString();
			var dmn = ObjectId.ObjectIdDomain.ToString();

			if (input.ContainsKey(user) && input [user].AttributeMappings.Count > 0) {
				schema.Add (ObjectId.ObjectIdUser.ToString(), new SchemaObjectMappingDto {
					ObjectClass = input [user].ObjectClass,
					AttributeMappings = new Dictionary<string, string> ()
				});

				UserAttributeId attributeEnum = UserAttributeId.UserAttributeAccountName;
				foreach (var attribute in input [user].AttributeMappings.Keys) {
					var e = attributeEnum.GetByDescription (attribute);
					schema [ObjectId.ObjectIdUser.ToString ()].AttributeMappings.Add (e.ToString (), input [user].AttributeMappings [attribute]);
				}
			}

			if (input.ContainsKey(pass) && input [pass].AttributeMappings.Count > 0) {
				schema.Add (ObjectId.ObjectIdPasswordSettings.ToString(), new SchemaObjectMappingDto {
					ObjectClass = input [pass].ObjectClass,
					AttributeMappings = new Dictionary<string, string> ()
				});

				PasswordAttributeId attributeEnum = PasswordAttributeId.PasswordSettingsAttributeMaximumPwdAge;
				foreach (var attribute in input [pass].AttributeMappings.Keys) {
					var e = attributeEnum.GetByDescription (attribute);
					schema [ObjectId.ObjectIdPasswordSettings.ToString ()].AttributeMappings.Add (e.ToString (), input [pass].AttributeMappings [attribute]);
				}
			}

			if (input.ContainsKey(grp) && input [grp].AttributeMappings.Count > 0) {
				schema.Add (ObjectId.ObjectIdGroup.ToString(), new SchemaObjectMappingDto {
					ObjectClass = input [grp].ObjectClass,
					AttributeMappings = new Dictionary<string, string> ()
				});

				GroupAttributeId attributeEnum = GroupAttributeId.GroupAttributeAccountName;
				foreach (var attribute in input [grp].AttributeMappings.Keys) {
					var e = attributeEnum.GetByDescription (attribute);
					schema [ObjectId.ObjectIdGroup.ToString ()].AttributeMappings.Add (e.ToString (), input [grp].AttributeMappings [attribute]);
				}
			}
			if (input.ContainsKey(dmn) && input [dmn].AttributeMappings.Count > 0) {
				schema.Add (ObjectId.ObjectIdDomain.ToString(), new SchemaObjectMappingDto {
					ObjectClass = input [dmn].ObjectClass,
					AttributeMappings = new Dictionary<string, string> ()
				});

				DomainAttributeId attributeEnum = DomainAttributeId.DomainAttributeMaxPwdAge;
				foreach (var attribute in input [dmn].AttributeMappings.Keys) {
					var e = attributeEnum.GetByDescription (attribute);
					schema [ObjectId.ObjectIdDomain.ToString ()].AttributeMappings.Add (e.ToString (), input [dmn].AttributeMappings [attribute]);
				}
			}

			return schema;
		}

		private void DtoToView()
		{
			var tag = GetIdFromIdentitySourceType (IdentityProviderDto.Type);
			RdoIdentitySource.SelectCellWithTag (tag);
			RdoIdentitySource.Enabled = false;
			TxtDomainName.Enabled = false;
			TxtDomainAlias.StringValue = string.IsNullOrEmpty (IdentityProviderDto.Alias) ? string.Empty : IdentityProviderDto.Alias;
			TxtFriendlyName.StringValue = string.IsNullOrEmpty (IdentityProviderDto.FriendlyName) ? string.Empty : IdentityProviderDto.FriendlyName;
			TxtDomainName.StringValue = string.IsNullOrEmpty (IdentityProviderDto.Name) ? string.Empty : IdentityProviderDto.Name;
			TxtBaseDnUser.StringValue = string.IsNullOrEmpty (IdentityProviderDto.UserBaseDN) ? string.Empty : IdentityProviderDto.UserBaseDN;
			TxtBaseDnGroups.StringValue = string.IsNullOrEmpty (IdentityProviderDto.GroupBaseDN) ? string.Empty : IdentityProviderDto.GroupBaseDN;
			if (tag == 2 && IdentityProviderDto.SiteAffinityEnabled) {
				RdoDomainController.SelectCellWithTag (1);
				ChkProtect.StringValue = "1";
				EnableDisableConnectionString (false);
			} else {
				EnableDisableConnectionString (true);
				ChkProtect.Enabled = false;
				RdoDomainController.SelectCellWithTag (2);
				if (IdentityProviderDto.ConnectionStrings.Count > 0) {
					TxtPrimaryConnection.StringValue = IdentityProviderDto.ConnectionStrings [0];
				}
				if (IdentityProviderDto.ConnectionStrings.Count > 1) {
					TxtSecondaryConnection.StringValue = IdentityProviderDto.ConnectionStrings [1];
				}
			}
			_certificates = IdentityProviderDto.Certificates == null ? new List<CertificateDto>() : IdentityProviderDto.Certificates;
			ReloadCertificates ();
			var machineAccount = IdentityProviderDto.UserMachineAccount ? 1 : 2;
			RdoSpn.SelectCellWithTag (machineAccount);
			TxtSpn.StringValue =  string.IsNullOrEmpty (IdentityProviderDto.ServicePrincipalName) ? string.Empty : IdentityProviderDto.ServicePrincipalName;
			TxtUpn.StringValue =  string.IsNullOrEmpty (IdentityProviderDto.Username) ? string.Empty : IdentityProviderDto.Username;
		}

		private List<string> GetConnectionStrings()
		{
			return RdoIdentitySource.SelectedTag == 2 && RdoDomainController.SelectedTag == 1 ?
				new List<string> { TxtLdapConnection.StringValue } :
			(!string.IsNullOrEmpty(TxtSecondaryConnection.StringValue) 
				? new List<string> { TxtPrimaryUrl.StringValue, TxtSecondaryConnection.StringValue }
				: new List<string> { TxtPrimaryUrl.StringValue });
		}

		private string GetIdentitySourceType(nint id)
		{
			var identitySource = string.Empty;
			switch (id) {
			case 1:
				identitySource = "IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY";
				break;
			case 2:
				identitySource = "IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING";
				break;
			case 3:
				identitySource = "IDENTITY_STORE_TYPE_LDAP";
				break;
			}
			return identitySource;
		}

		private int GetIdFromIdentitySourceType(string sourceType)
		{
			int type = 0;
			switch (sourceType) {
			case "IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY":
				type = 1;
				break;
			case "IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING":
				type = 2;
				break;
			case "IDENTITY_STORE_TYPE_LDAP":
				type = 3;
				break;
			}
			return type;
		}

		#endregion

		//strongly typed window accessor
		public new AddExternalIdentitySource Window {
			get {
				return (AddExternalIdentitySource)base.Window;
			}
		}

		private void SetNextStep()
		{
			if (_currentStep == WizardSteps.One && RdoIdentitySource.SelectedRow == 0) {
				_currentStep = WizardSteps.Four;
			} else {	
				if (_currentStep == WizardSteps.Two && IsUnsecuredConnection ()) {
					_currentStep = WizardSteps.Four;
				} else {
					_currentStep++;
				}
			}
		}
		private bool IsUnsecuredConnection()
		{
			var connectionStrings = GetConnectionStrings();
			return !connectionStrings.Exists(x=>x.StartsWith("ldaps"));
		}
		private void SetPreviousStep()
		{
			if (_currentStep == WizardSteps.Four) {
				if (RdoIdentitySource.SelectedRow == 0) {
					_currentStep = WizardSteps.One;
				} else {
						_currentStep = IsUnsecuredConnection() ? WizardSteps.Two : _currentStep - 1;
				}
			} else {

				if (!isUpdate || (isUpdate && _currentStep > WizardSteps.Two)) {
					_currentStep--;
				}
			}
		}

		private string GetDomainName()
		{
			var domainName = string.Empty;
			ActionHelper.Execute (delegate() {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
				var adJoinInfoDto = SnapInContext.Instance.ServiceGateway.Adf.GetActiveDirectory (auth.ServerDto, auth.Token);
				if (adJoinInfoDto != null && adJoinInfoDto.JoinStatus == "DOMAIN") {
					domainName = adJoinInfoDto.Name;
				}
			});
			return domainName;
		}

		private void SetWizardStep()
		{
			PnlStep1.Hidden = _currentStep != WizardSteps.One;
			PnlStep2.Hidden = _currentStep != WizardSteps.Two;
			PnlStep3.Hidden = _currentStep != WizardSteps.Three;
			PnlStep4.Hidden = _currentStep != WizardSteps.Four;

			if (!PnlStep2.Hidden) {
				PnlProtect.Hidden = RdoIdentitySource.SelectedRow != 1;
				PnlSpecificDomainController.Hidden = false;
				SetConnectionString ();
				var anyDc = RdoIdentitySource.SelectedTag == 2 && RdoDomainController.SelectedTag == 1;
				EnableDisableConnectionString (!anyDc);
			}
			var adWin = IsAdWinSelected ();

			if (!PnlStep4.Hidden) {
				RdoSpn.Hidden = !adWin;
				LblSpn.Hidden = !adWin;
				TxtSpn.Hidden = !adWin;
				LblUsername.StringValue = adWin ? "User Principal Name" : "Username";
			}

			BtnBack.Enabled = CheckBackEnabled (adWin);
			BtnNext.Title = (_currentStep == WizardSteps.Four) ? "Save" : "Next";
			BtnTestConnection.Hidden = RdoIdentitySource.SelectedTag == 1;
			BtnAdvanced.Hidden = RdoIdentitySource.SelectedTag == 1;
			StepIndicator.IntValue = (int)(_currentStep - 1);
		}

		private bool CheckBackEnabled(bool adWin){
			return ((isUpdate && !adWin && _currentStep > WizardSteps.Two) ||
				(!isUpdate && _currentStep != WizardSteps.One));
		}
		private bool IsAdWinSelected()
		{
			return RdoIdentitySource.SelectedTag == 1;
		}

		public enum WizardSteps
		{
			One = 1,
			Two = 2,
			Three = 3,
			Four = 4,
			Save = 5
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


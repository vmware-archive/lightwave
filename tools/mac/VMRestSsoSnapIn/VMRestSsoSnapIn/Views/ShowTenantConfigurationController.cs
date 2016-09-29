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
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn.Nodes;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;	
using System.Security.Cryptography.X509Certificates;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using VmIdentity.CommonUtils.Utilities;

namespace RestSsoAdminSnapIn
{
	public partial class ShowTenantConfigurationController : NSWindowController
	{
		public TenantConfigurationDto TenantConfigurationDto { get; set; }
		public ServerDto ServerDto{ get; set; }
		public string TenantName { get; set; }

		public ShowTenantConfigurationController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public ShowTenantConfigurationController (NSCoder coder) : base (coder)
		{
		}

		public ShowTenantConfigurationController () : base ("ShowTenantConfiguration")
		{
		}
		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();

			DtoToView ();

			this.BtnAuthenticationAddCertificate.Activated +=	(object sender, EventArgs e) => {
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
						var certfificateDto = new CertificateDto { Encoded = cert.ExportToPem(), };
						TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates.Add(certfificateDto);
						ReloadCertificates();
					});
				}
			};

			this.BtnAuthenticationRemoveCertificate.Activated += (object sender, EventArgs e) => {
				if (CertificateTableView.SelectedRows.Count > 0) {
					foreach (var row in CertificateTableView.SelectedRows) {
						TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates.RemoveAt ((int)row);
					}
					ReloadCertificates();
				}
			};

			BtnAuthenticationPolicyAddPolicyOid.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtAuthenticationPolicyOid.StringValue))
				{
					UIErrorHelper.ShowAlert ("Policy OID cannot be empty", "Alert");
					return;
				}
				TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs.Add(TxtAuthenticationPolicyOid.StringValue);
				ReloadTableView(AuthenticationPolicyOidTableView, TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs);
				TxtAuthenticationPolicyOid.StringValue = (NSString)string.Empty;
				BtnAuthenticationRemovePolicyOid.Enabled = TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs != null &&
					TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs.Count > 0;
			};

			BtnAuthenticationRemovePolicyOid.Activated += (object sender, EventArgs e) => {
				if (AuthenticationPolicyOidTableView.SelectedRows.Count > 0) {
					foreach (var row in AuthenticationPolicyOidTableView.SelectedRows) {
						TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs.RemoveAt((int)row);
					}
					ReloadTableView(AuthenticationPolicyOidTableView, TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs);
					BtnAuthenticationRemovePolicyOid.Enabled = TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs != null &&
						TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs.Count > 0;
				}
			};

			BtnClose.Activated += (object sender, EventArgs e) => {
				TenantConfigurationDto = null;
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};

			this.BtnSave.Activated += (object sender, EventArgs e) => {

				ActionHelper.Execute (delegate() {
				ViewToDto();
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto.ServerName);
				SnapInContext.Instance.ServiceGateway.Tenant.UpdateConfig(ServerDto,TenantName,TenantConfigurationDto,auth.Token);
				SnapInContext.Instance.ServiceGateway.Tenant.UpdatePasswordAndLockoutConfig(ServerDto,TenantName,TenantConfigurationDto,auth.Token);
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (1);
				});
			};

			this.ChkDisableLogonBanner.Activated += (object sender, EventArgs e) => {
				CheckLogonBanner();
			};
			this.BtnUploadContent.Activated += (object sender, EventArgs e) => {
				var openPanel = new NSOpenPanel();
				openPanel.ReleasedWhenClosed = true;
				openPanel.Prompt = "Select file";
				var result = openPanel.RunModal();
				if (result == 1)
				{
					var filePath = openPanel.Url.AbsoluteString.Replace("file://",string.Empty);

					ActionHelper.Execute (delegate() {
						var text = System.IO.File.ReadAllText(filePath);
						if(!string.IsNullOrEmpty(text))
							TxtBrandLogonBanner.StringValue = text;
					});
				}
			};
			CheckLogonBanner ();
		}

		private void CheckLogonBanner()
		{
			var enableLogonBanner = ChkDisableLogonBanner.StringValue != "1";
			TxtBrandLogonBanner.Enabled = enableLogonBanner;
			TxtLogonBannerTitle.Enabled = enableLogonBanner;
			BtnUploadContent.Enabled = enableLogonBanner;
		}

		private void ViewToDto()
		{
			TenantConfigurationDto.LockoutPolicy.Description = TxtLockoutDescription.StringValue;
			TenantConfigurationDto.LockoutPolicy.AutoUnlockIntervalSec = (long)TxtLockoutAutoUnlockINterval.DoubleValue;
			TenantConfigurationDto.LockoutPolicy.FailedAttemptIntervalSec = (long)TxtLockoutFailedAttemptInterval.DoubleValue;
			TenantConfigurationDto.LockoutPolicy.MaxFailedAttempts = (int)TxtLockoutMaxFailedAttempts.DoubleValue;

			TenantConfigurationDto.TokenPolicy.ClockToleranceMillis = (long)TxtTokenClockTolerence.DoubleValue;
			TenantConfigurationDto.TokenPolicy.DelegationCount = TxtTokenDelegateCount.IntValue;
			TenantConfigurationDto.TokenPolicy.RenewCount = TxtTokenRenewCount.IntValue;
			TenantConfigurationDto.TokenPolicy.MaxBearerTokenLifeTimeMillis = (long)TxtTokenBearerMaxLifetime.DoubleValue;
			TenantConfigurationDto.TokenPolicy.MaxBearerRefreshTokenLifeTimeMillis = (long)TxtTokenBearerMaxRefreshTime.DoubleValue;
			TenantConfigurationDto.TokenPolicy.MaxHOKTokenLifeTimeMillis = (long)TxtHokMaxRefreshTime.DoubleValue;
			TenantConfigurationDto.TokenPolicy.MaxHOKRefreshTokenLifeTimeMillis = (long)TxtHokMaxRefreshTime.DoubleValue;

			TenantConfigurationDto.AuthenticationPolicy.PasswordBasedAuthentication = 
				ChkAuthenticationPolicyPassword.StringValue == "1";
			TenantConfigurationDto.AuthenticationPolicy.WindowsBasedAuthentication = 
				ChkAuthentiationPolicyWindows.StringValue == "1";
			TenantConfigurationDto.AuthenticationPolicy.CertificateBasedAuthentication = 
				ChkAuthenticationPolicyCertificate.StringValue == "1";
			//if (TenantConfigurationDto.AuthenticationPolicy.CertificateBasedAuthentication) {
				if (TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy == null)
					TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy = new ClientCertificatePolicyDto ();
				TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.OcspEnabled = ChkAuthenticationOcsp.StringValue == "1";
				TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.FailOverToCrlEnabled = ChkAuthenticationFailoverToCrl.StringValue == "1";
				TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.RevocationCheckEnabled = ChkAuthetnicationRevocationCheck.StringValue == "1";
				TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CrlDistributionPointUsageEnabled = ChkAuthenticationCrlDistribution.StringValue == "1";
			if (!string.IsNullOrEmpty (TxtAuthenticationOcspUrlOverride.StringValue))
				TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.OcspUrlOverride = TxtAuthenticationOcspUrlOverride.StringValue;
			else
				TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.OcspUrlOverride = null;
			if (!string.IsNullOrEmpty (TxtCrlDistributionPointOverride.StringValue))
				TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CrlDistributionPointOverride = TxtCrlDistributionPointOverride.StringValue;
			else
				TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CrlDistributionPointOverride = null;
			if (TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs == null)
				TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs = new List<string> ();
			if (TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates == null)
				TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates = new List<CertificateDto> ();

			TenantConfigurationDto.ProviderPolicy.DefaultProvider = TxtProviderDefault.StringValue;
			TenantConfigurationDto.ProviderPolicy.DefaultProviderAlias = DefaultProviderAlias.StringValue;
			TenantConfigurationDto.ProviderPolicy.ProviderSelectionEnabled = CbEnablePoviderSelection.StringValue == "1";

			TenantConfigurationDto.BrandPolicy.Name = TxtBrandName.StringValue;
			TenantConfigurationDto.BrandPolicy.LogonBannerContent = TxtBrandLogonBanner.StringValue;
			TenantConfigurationDto.BrandPolicy.LogonBannerTitle = TxtLogonBannerTitle.StringValue;
			TenantConfigurationDto.BrandPolicy.LogonBannerDisabled = ChkDisableLogonBanner.StringValue  == "1";
			TenantConfigurationDto.BrandPolicy.LogonBannerCheckboxEnabled = ChkLogonBannerCheckbox.StringValue == "1";

			TenantConfigurationDto.PasswordPolicy.Description = TxtPasswordDescription.StringValue;
			TenantConfigurationDto.PasswordPolicy.MaxIdenticalAdjacentCharacters = TxtPasswordMaxIdenticalAdjChars.IntValue;
			TenantConfigurationDto.PasswordPolicy.MaxLength = TxtPasswordMaxLength.IntValue;
			TenantConfigurationDto.PasswordPolicy.MinAlphabeticCount = TxtPasswordMinAlphaCount.IntValue;
			TenantConfigurationDto.PasswordPolicy.MinLength = TxtPasswordMinLength.IntValue;
			TenantConfigurationDto.PasswordPolicy.MinLowercaseCount = TxtPasswordMinLowercaseCount.IntValue;
			TenantConfigurationDto.PasswordPolicy.MinNumericCount = TxtPasswordMinNumericCount.IntValue;
			TenantConfigurationDto.PasswordPolicy.MinSpecialCharCount = TxtPasswordMinSpecialCharCount.IntValue;
			TenantConfigurationDto.PasswordPolicy.MinUppercaseCount = TxtPasswordMinUpperCaseCount.IntValue;
			TenantConfigurationDto.PasswordPolicy.PasswordLifetimeDays = TxtPasswordLifetime.IntValue;
			TenantConfigurationDto.PasswordPolicy.ProhibitedPreviousPasswordCount = TxtPasswordProhibitedPreviousPasswordCount.IntValue;
		}

		private void DtoToView()
		{
			if (TenantConfigurationDto.LockoutPolicy != null) {
				TxtLockoutDescription.StringValue = (NSString)(string.IsNullOrEmpty (TenantConfigurationDto.LockoutPolicy.Description) 
					? string.Empty 
					: TenantConfigurationDto.LockoutPolicy.Description);
				TxtLockoutAutoUnlockINterval.DoubleValue = (double)TenantConfigurationDto.LockoutPolicy.AutoUnlockIntervalSec;
				TxtLockoutFailedAttemptInterval.DoubleValue = (double)TenantConfigurationDto.LockoutPolicy.FailedAttemptIntervalSec;
				TxtLockoutMaxFailedAttempts.IntValue = TenantConfigurationDto.LockoutPolicy.MaxFailedAttempts;
			} else
				TenantConfigurationDto.LockoutPolicy = new LockoutPolicyDto ();

			if (TenantConfigurationDto.TokenPolicy != null) {
				TxtTokenClockTolerence.DoubleValue = (double)TenantConfigurationDto.TokenPolicy.ClockToleranceMillis;
				TxtTokenDelegateCount.IntValue = TenantConfigurationDto.TokenPolicy.DelegationCount;
				TxtTokenRenewCount.IntValue = TenantConfigurationDto.TokenPolicy.RenewCount;
				TxtTokenBearerMaxLifetime.DoubleValue = (double)TenantConfigurationDto.TokenPolicy.MaxBearerTokenLifeTimeMillis;
				TxtTokenBearerMaxRefreshTime.DoubleValue = (double)TenantConfigurationDto.TokenPolicy.MaxBearerRefreshTokenLifeTimeMillis;
				TxtHokMaxRefreshTime.DoubleValue = (double)TenantConfigurationDto.TokenPolicy.MaxHOKTokenLifeTimeMillis;
				TxtHokMaxRefreshTime.DoubleValue = (double)TenantConfigurationDto.TokenPolicy.MaxHOKRefreshTokenLifeTimeMillis;
			} else
				TenantConfigurationDto.TokenPolicy = new TokenPolicyDto ();

			ChkAuthenticationPolicyPassword.StringValue = "0";
			ChkAuthentiationPolicyWindows.StringValue = "0";
			ChkAuthenticationPolicyCertificate.StringValue = "0";
			ChkAuthenticationOcsp.StringValue = "0";
			ChkAuthenticationFailoverToCrl.StringValue = "0";
			ChkAuthetnicationRevocationCheck.StringValue = "0";
			ChkAuthenticationCrlDistribution.StringValue = "0";

			if (TenantConfigurationDto.AuthenticationPolicy != null) {
				ChkAuthenticationPolicyPassword.StringValue = (TenantConfigurationDto.AuthenticationPolicy.PasswordBasedAuthentication) ? "1" : "0";
				ChkAuthentiationPolicyWindows.StringValue = (TenantConfigurationDto.AuthenticationPolicy.WindowsBasedAuthentication) ? "1" : "0";
				ChkAuthenticationPolicyCertificate.StringValue = (TenantConfigurationDto.AuthenticationPolicy.CertificateBasedAuthentication) ? "1" : "0";

				if (TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy != null) {
					ChkAuthenticationOcsp.StringValue = TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.OcspEnabled ? "1" : "0";
					ChkAuthenticationFailoverToCrl.StringValue = TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.FailOverToCrlEnabled ? "1" : "0";
					ChkAuthetnicationRevocationCheck.StringValue = TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.RevocationCheckEnabled ? "1" : "0";
					ChkAuthenticationCrlDistribution.StringValue = TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CrlDistributionPointUsageEnabled ? "1" : "0";

					TxtAuthenticationOcspUrlOverride.StringValue = (NSString)
						(string.IsNullOrEmpty (TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.OcspUrlOverride)
							? string.Empty
							: TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.OcspUrlOverride);

					TxtCrlDistributionPointOverride.StringValue = (NSString)
						(string.IsNullOrEmpty (TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CrlDistributionPointOverride)
							? string.Empty
							: TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CrlDistributionPointOverride);
				} else {
					TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy = new ClientCertificatePolicyDto {
						CertPolicyOIDs = new List<string> (),
						TrustedCACertificates = new List<CertificateDto> ()
					};
				}
				if (TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs == null)
					TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs = new List<string> ();
				if (TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates == null)
					TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates = new List<CertificateDto> ();
				
				ReloadCertificates ();
				ReloadTableView (AuthenticationPolicyOidTableView, TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs);
				BtnAuthenticationRemovePolicyOid.Enabled = TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs != null &&
				TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs.Count > 0;

			} else
				TenantConfigurationDto.AuthenticationPolicy = new AuthenticationPolicyDto {
					ClientCertificatePolicy = new ClientCertificatePolicyDto {
						CertPolicyOIDs = new List<string> (),
						TrustedCACertificates = new List<CertificateDto> ()
					}
				};

			if (TenantConfigurationDto.PasswordPolicy != null) {
				TxtPasswordDescription.StringValue = (NSString)
					(string.IsNullOrEmpty (TenantConfigurationDto.PasswordPolicy.Description)
						? string.Empty
						: TenantConfigurationDto.PasswordPolicy.Description);
				TxtPasswordMaxIdenticalAdjChars.IntValue = TenantConfigurationDto.PasswordPolicy.MaxIdenticalAdjacentCharacters;
				TxtPasswordMaxLength.IntValue = TenantConfigurationDto.PasswordPolicy.MaxLength;
				TxtPasswordMinAlphaCount.IntValue = TenantConfigurationDto.PasswordPolicy.MinAlphabeticCount;
				TxtPasswordMinLength.IntValue = TenantConfigurationDto.PasswordPolicy.MinLength;
				TxtPasswordMinLowercaseCount.IntValue = TenantConfigurationDto.PasswordPolicy.MinLowercaseCount;
				TxtPasswordMinNumericCount.IntValue = TenantConfigurationDto.PasswordPolicy.MinNumericCount;
				TxtPasswordMinSpecialCharCount.IntValue = TenantConfigurationDto.PasswordPolicy.MinSpecialCharCount;
				TxtPasswordMinUpperCaseCount.IntValue = TenantConfigurationDto.PasswordPolicy.MinUppercaseCount;
				TxtPasswordLifetime.IntValue = TenantConfigurationDto.PasswordPolicy.PasswordLifetimeDays;
				TxtPasswordProhibitedPreviousPasswordCount.IntValue = TenantConfigurationDto.PasswordPolicy.ProhibitedPreviousPasswordCount;
			} else
				TenantConfigurationDto.PasswordPolicy = new PasswordPolicyDto ();
			
			if (TenantConfigurationDto.ProviderPolicy != null) {

				TxtProviderDefault.StringValue = (NSString)(string.IsNullOrEmpty (TenantConfigurationDto.ProviderPolicy.DefaultProvider)
					? string.Empty
					: TenantConfigurationDto.ProviderPolicy.DefaultProvider);
				DefaultProviderAlias.StringValue = (NSString)(string.IsNullOrEmpty (TenantConfigurationDto.ProviderPolicy.DefaultProviderAlias)
					? string.Empty
					: TenantConfigurationDto.ProviderPolicy.DefaultProviderAlias);
				CbEnablePoviderSelection.StringValue = TenantConfigurationDto.ProviderPolicy.ProviderSelectionEnabled ? "1" : "0";
			} else
				TenantConfigurationDto.ProviderPolicy = new ProviderPolicyDto ();

			if (TenantConfigurationDto.BrandPolicy != null) {
				TxtBrandName.StringValue = (NSString)(string.IsNullOrEmpty (TenantConfigurationDto.BrandPolicy.Name)
					? string.Empty
					: TenantConfigurationDto.BrandPolicy.Name);
				TxtBrandLogonBanner.StringValue = (NSString)(string.IsNullOrEmpty (TenantConfigurationDto.BrandPolicy.LogonBannerContent)
					? string.Empty
					: TenantConfigurationDto.BrandPolicy.LogonBannerContent);
				TxtLogonBannerTitle.StringValue = (NSString)(string.IsNullOrEmpty (TenantConfigurationDto.BrandPolicy.LogonBannerTitle)
					? string.Empty
					: TenantConfigurationDto.BrandPolicy.LogonBannerTitle);
				ChkDisableLogonBanner.StringValue = TenantConfigurationDto.BrandPolicy.LogonBannerDisabled ? "1" : "0";
				ChkLogonBannerCheckbox.StringValue = TenantConfigurationDto.BrandPolicy.LogonBannerCheckboxEnabled ? "1" : "0";
			} else
				TenantConfigurationDto.BrandPolicy = new BrandPolicyDto ();
		}

		public void ReloadTableView(NSTableView tableView, List<string> datasource)
		{
			tableView.Delegate = new TableDelegate ();
			var listView = new DefaultDataSource { Entries = datasource };
			tableView.DataSource = listView;
			tableView.ReloadData ();
		}

		public new ShowTenantConfiguration Window {
			get { return (ShowTenantConfiguration)base.Window; }
		}
		private void ReloadCertificates()
		{
			foreach(NSTableColumn column in CertificateTableView.TableColumns())
			{
				CertificateTableView.RemoveColumn (column);
			}
			CertificateTableView.Delegate = new CertTableDelegate ();
			var listView = new TrustedCertificatesDataSource { Entries = TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates };
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
			BtnAuthenticationRemoveCertificate.Enabled = TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates != null &&
			TenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates.Count > 0;
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

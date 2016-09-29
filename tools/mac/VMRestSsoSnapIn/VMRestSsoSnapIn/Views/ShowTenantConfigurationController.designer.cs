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

using Foundation;
using System.CodeDom.Compiler;

namespace RestSsoAdminSnapIn
{
	[Register ("ShowTenantConfigurationController")]
	partial class ShowTenantConfigurationController
	{
		[Outlet]
		AppKit.NSTableView AuthenticationPolicyOidTableView { get; set; }

		[Outlet]
		AppKit.NSButton BtnAuthenticationAddCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnAuthenticationPolicyAddPolicyOid { get; set; }

		[Outlet]
		AppKit.NSButton BtnAuthenticationRemoveCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnAuthenticationRemovePolicyOid { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnSave { get; set; }

		[Outlet]
		AppKit.NSButton BtnUploadContent { get; set; }

		[Outlet]
		AppKit.NSButton CbEnablePoviderSelection { get; set; }

		[Outlet]
		AppKit.NSTableView CertificateTableView { get; set; }

		[Outlet]
		AppKit.NSButton ChkAuthentiationPolicyWindows { get; set; }

		[Outlet]
		AppKit.NSButton ChkAuthenticationCrlDistribution { get; set; }

		[Outlet]
		AppKit.NSButton ChkAuthenticationFailoverToCrl { get; set; }

		[Outlet]
		AppKit.NSButton ChkAuthenticationOcsp { get; set; }

		[Outlet]
		AppKit.NSButton ChkAuthenticationPolicyCertificate { get; set; }

		[Outlet]
		AppKit.NSButton ChkAuthenticationPolicyPassword { get; set; }

		[Outlet]
		AppKit.NSButton ChkAuthetnicationRevocationCheck { get; set; }

		[Outlet]
		AppKit.NSButton ChkDisableLogonBanner { get; set; }

		[Outlet]
		AppKit.NSButton ChkLogonBannerCheckbox { get; set; }

		[Outlet]
		AppKit.NSTextField DefaultProviderAlias { get; set; }

		[Outlet]
		AppKit.NSTextField TxtAuthenticationOcspUrlOverride { get; set; }

		[Outlet]
		AppKit.NSTextField TxtAuthenticationPolicyOid { get; set; }

		[Outlet]
		AppKit.NSTextField TxtBrandLogonBanner { get; set; }

		[Outlet]
		AppKit.NSTextField TxtBrandName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtCrlDistributionPointOverride { get; set; }

		[Outlet]
		AppKit.NSTextField TxtHokMaxRefreshTime { get; set; }

		[Outlet]
		AppKit.NSTextField TxtLockoutAutoUnlockINterval { get; set; }

		[Outlet]
		AppKit.NSTextField TxtLockoutDescription { get; set; }

		[Outlet]
		AppKit.NSTextField TxtLockoutFailedAttemptInterval { get; set; }

		[Outlet]
		AppKit.NSTextField TxtLockoutMaxFailedAttempts { get; set; }

		[Outlet]
		AppKit.NSTextField TxtLogonBannerTitle { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordDescription { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordLifetime { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordMaxIdenticalAdjChars { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordMaxLength { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordMinAlphaCount { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordMinLength { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordMinLowercaseCount { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordMinNumericCount { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordMinSpecialCharCount { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordMinUpperCaseCount { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordProhibitedPreviousPasswordCount { get; set; }

		[Outlet]
		AppKit.NSTextField TxtProviderDefault { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTokenBearerMaxLifetime { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTokenBearerMaxRefreshTime { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTokenClockTolerence { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTokenDelegateCount { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTokenHokMaxLifetime { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTokenRenewCount { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (AuthenticationPolicyOidTableView != null) {
				AuthenticationPolicyOidTableView.Dispose ();
				AuthenticationPolicyOidTableView = null;
			}

			if (BtnAuthenticationAddCertificate != null) {
				BtnAuthenticationAddCertificate.Dispose ();
				BtnAuthenticationAddCertificate = null;
			}

			if (BtnAuthenticationPolicyAddPolicyOid != null) {
				BtnAuthenticationPolicyAddPolicyOid.Dispose ();
				BtnAuthenticationPolicyAddPolicyOid = null;
			}

			if (BtnAuthenticationRemoveCertificate != null) {
				BtnAuthenticationRemoveCertificate.Dispose ();
				BtnAuthenticationRemoveCertificate = null;
			}

			if (BtnAuthenticationRemovePolicyOid != null) {
				BtnAuthenticationRemovePolicyOid.Dispose ();
				BtnAuthenticationRemovePolicyOid = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnSave != null) {
				BtnSave.Dispose ();
				BtnSave = null;
			}

			if (BtnUploadContent != null) {
				BtnUploadContent.Dispose ();
				BtnUploadContent = null;
			}

			if (CertificateTableView != null) {
				CertificateTableView.Dispose ();
				CertificateTableView = null;
			}

			if (ChkAuthentiationPolicyWindows != null) {
				ChkAuthentiationPolicyWindows.Dispose ();
				ChkAuthentiationPolicyWindows = null;
			}

			if (ChkAuthenticationCrlDistribution != null) {
				ChkAuthenticationCrlDistribution.Dispose ();
				ChkAuthenticationCrlDistribution = null;
			}

			if (ChkAuthenticationFailoverToCrl != null) {
				ChkAuthenticationFailoverToCrl.Dispose ();
				ChkAuthenticationFailoverToCrl = null;
			}

			if (ChkAuthenticationOcsp != null) {
				ChkAuthenticationOcsp.Dispose ();
				ChkAuthenticationOcsp = null;
			}

			if (ChkAuthenticationPolicyCertificate != null) {
				ChkAuthenticationPolicyCertificate.Dispose ();
				ChkAuthenticationPolicyCertificate = null;
			}

			if (ChkAuthenticationPolicyPassword != null) {
				ChkAuthenticationPolicyPassword.Dispose ();
				ChkAuthenticationPolicyPassword = null;
			}

			if (ChkAuthetnicationRevocationCheck != null) {
				ChkAuthetnicationRevocationCheck.Dispose ();
				ChkAuthetnicationRevocationCheck = null;
			}

			if (ChkDisableLogonBanner != null) {
				ChkDisableLogonBanner.Dispose ();
				ChkDisableLogonBanner = null;
			}

			if (ChkLogonBannerCheckbox != null) {
				ChkLogonBannerCheckbox.Dispose ();
				ChkLogonBannerCheckbox = null;
			}

			if (TxtAuthenticationOcspUrlOverride != null) {
				TxtAuthenticationOcspUrlOverride.Dispose ();
				TxtAuthenticationOcspUrlOverride = null;
			}

			if (TxtAuthenticationPolicyOid != null) {
				TxtAuthenticationPolicyOid.Dispose ();
				TxtAuthenticationPolicyOid = null;
			}

			if (TxtBrandLogonBanner != null) {
				TxtBrandLogonBanner.Dispose ();
				TxtBrandLogonBanner = null;
			}

			if (TxtBrandName != null) {
				TxtBrandName.Dispose ();
				TxtBrandName = null;
			}

			if (TxtCrlDistributionPointOverride != null) {
				TxtCrlDistributionPointOverride.Dispose ();
				TxtCrlDistributionPointOverride = null;
			}

			if (TxtHokMaxRefreshTime != null) {
				TxtHokMaxRefreshTime.Dispose ();
				TxtHokMaxRefreshTime = null;
			}

			if (TxtLockoutAutoUnlockINterval != null) {
				TxtLockoutAutoUnlockINterval.Dispose ();
				TxtLockoutAutoUnlockINterval = null;
			}

			if (TxtLockoutDescription != null) {
				TxtLockoutDescription.Dispose ();
				TxtLockoutDescription = null;
			}

			if (TxtLockoutFailedAttemptInterval != null) {
				TxtLockoutFailedAttemptInterval.Dispose ();
				TxtLockoutFailedAttemptInterval = null;
			}

			if (TxtLockoutMaxFailedAttempts != null) {
				TxtLockoutMaxFailedAttempts.Dispose ();
				TxtLockoutMaxFailedAttempts = null;
			}

			if (TxtLogonBannerTitle != null) {
				TxtLogonBannerTitle.Dispose ();
				TxtLogonBannerTitle = null;
			}

			if (TxtPasswordDescription != null) {
				TxtPasswordDescription.Dispose ();
				TxtPasswordDescription = null;
			}

			if (TxtPasswordLifetime != null) {
				TxtPasswordLifetime.Dispose ();
				TxtPasswordLifetime = null;
			}

			if (TxtPasswordMaxIdenticalAdjChars != null) {
				TxtPasswordMaxIdenticalAdjChars.Dispose ();
				TxtPasswordMaxIdenticalAdjChars = null;
			}

			if (TxtPasswordMaxLength != null) {
				TxtPasswordMaxLength.Dispose ();
				TxtPasswordMaxLength = null;
			}

			if (TxtPasswordMinAlphaCount != null) {
				TxtPasswordMinAlphaCount.Dispose ();
				TxtPasswordMinAlphaCount = null;
			}

			if (TxtPasswordMinLength != null) {
				TxtPasswordMinLength.Dispose ();
				TxtPasswordMinLength = null;
			}

			if (TxtPasswordMinLowercaseCount != null) {
				TxtPasswordMinLowercaseCount.Dispose ();
				TxtPasswordMinLowercaseCount = null;
			}

			if (TxtPasswordMinNumericCount != null) {
				TxtPasswordMinNumericCount.Dispose ();
				TxtPasswordMinNumericCount = null;
			}

			if (TxtPasswordMinSpecialCharCount != null) {
				TxtPasswordMinSpecialCharCount.Dispose ();
				TxtPasswordMinSpecialCharCount = null;
			}

			if (TxtPasswordMinUpperCaseCount != null) {
				TxtPasswordMinUpperCaseCount.Dispose ();
				TxtPasswordMinUpperCaseCount = null;
			}

			if (TxtPasswordProhibitedPreviousPasswordCount != null) {
				TxtPasswordProhibitedPreviousPasswordCount.Dispose ();
				TxtPasswordProhibitedPreviousPasswordCount = null;
			}

			if (TxtProviderDefault != null) {
				TxtProviderDefault.Dispose ();
				TxtProviderDefault = null;
			}

			if (TxtTokenBearerMaxLifetime != null) {
				TxtTokenBearerMaxLifetime.Dispose ();
				TxtTokenBearerMaxLifetime = null;
			}

			if (TxtTokenBearerMaxRefreshTime != null) {
				TxtTokenBearerMaxRefreshTime.Dispose ();
				TxtTokenBearerMaxRefreshTime = null;
			}

			if (TxtTokenClockTolerence != null) {
				TxtTokenClockTolerence.Dispose ();
				TxtTokenClockTolerence = null;
			}

			if (TxtTokenDelegateCount != null) {
				TxtTokenDelegateCount.Dispose ();
				TxtTokenDelegateCount = null;
			}

			if (TxtTokenHokMaxLifetime != null) {
				TxtTokenHokMaxLifetime.Dispose ();
				TxtTokenHokMaxLifetime = null;
			}

			if (TxtTokenRenewCount != null) {
				TxtTokenRenewCount.Dispose ();
				TxtTokenRenewCount = null;
			}

			if (DefaultProviderAlias != null) {
				DefaultProviderAlias.Dispose ();
				DefaultProviderAlias = null;
			}

			if (CbEnablePoviderSelection != null) {
				CbEnablePoviderSelection.Dispose ();
				CbEnablePoviderSelection = null;
			}
		}
	}
}

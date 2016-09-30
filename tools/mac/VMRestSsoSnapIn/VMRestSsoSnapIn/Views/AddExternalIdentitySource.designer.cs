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
	[Register ("AddExternalIdentitySourceController")]
	partial class AddExternalIdentitySourceController
	{
		[Outlet]
		AppKit.NSButton BtnAddCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnAdvanced { get; set; }

		[Outlet]
		AppKit.NSButton BtnBack { get; set; }

		[Outlet]
		AppKit.NSButton BtnHelpADWindowsIntegrated { get; set; }

		[Outlet]
		AppKit.NSButton BtnHelpCredentials { get; set; }

		[Outlet]
		AppKit.NSButton BtnHelpDomainName { get; set; }

		[Outlet]
		AppKit.NSButton BtnNext { get; set; }

		[Outlet]
		AppKit.NSButton BtnPrimaryImport { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnSecondaryImport { get; set; }

		[Outlet]
		AppKit.NSButton BtnTestConnection { get; set; }

		[Outlet]
		AppKit.NSButton ChkProtect { get; set; }

		[Outlet]
		AppKit.NSTextField LblPassword { get; set; }

		[Outlet]
		AppKit.NSTextField LblSpn { get; set; }

		[Outlet]
		AppKit.NSTextField LblUsername { get; set; }

		[Outlet]
		AppKit.NSTableView LstCertificates { get; set; }

		[Outlet]
		AppKit.NSView PnlProtect { get; set; }

		[Outlet]
		AppKit.NSView PnlSpecificDomainController { get; set; }

		[Outlet]
		AppKit.NSView PnlStep1 { get; set; }

		[Outlet]
		AppKit.NSView PnlStep2 { get; set; }

		[Outlet]
		AppKit.NSView PnlStep3 { get; set; }

		[Outlet]
		AppKit.NSView PnlStep4 { get; set; }

		[Outlet]
		AppKit.NSMatrix RdoDomainController { get; set; }

		[Outlet]
		AppKit.NSMatrix RdoIdentitySource { get; set; }

		[Outlet]
		AppKit.NSMatrix RdoSpn { get; set; }

		[Outlet]
		AppKit.NSLevelIndicator StepIndicator { get; set; }

		[Outlet]
		AppKit.NSTextField TxtBaseDnGroups { get; set; }

		[Outlet]
		AppKit.NSTextField TxtBaseDnUser { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDomainAlias { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDomainName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtFriendlyName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtGroupbaseDN { get; set; }

		[Outlet]
		AppKit.NSTextField TxtLdapConnection { get; set; }

		[Outlet]
		AppKit.NSSecureTextField TxtPassword { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPrimaryConnection { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPrimaryUrl { get; set; }

		[Outlet]
		AppKit.NSTextField TxtSecondaryConnection { get; set; }

		[Outlet]
		AppKit.NSTextField TxtSpn { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUpn { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (TxtFriendlyName != null) {
				TxtFriendlyName.Dispose ();
				TxtFriendlyName = null;
			}

			if (BtnAddCertificate != null) {
				BtnAddCertificate.Dispose ();
				BtnAddCertificate = null;
			}

			if (BtnAdvanced != null) {
				BtnAdvanced.Dispose ();
				BtnAdvanced = null;
			}

			if (BtnBack != null) {
				BtnBack.Dispose ();
				BtnBack = null;
			}

			if (BtnHelpADWindowsIntegrated != null) {
				BtnHelpADWindowsIntegrated.Dispose ();
				BtnHelpADWindowsIntegrated = null;
			}

			if (BtnHelpCredentials != null) {
				BtnHelpCredentials.Dispose ();
				BtnHelpCredentials = null;
			}

			if (BtnHelpDomainName != null) {
				BtnHelpDomainName.Dispose ();
				BtnHelpDomainName = null;
			}

			if (BtnNext != null) {
				BtnNext.Dispose ();
				BtnNext = null;
			}

			if (BtnPrimaryImport != null) {
				BtnPrimaryImport.Dispose ();
				BtnPrimaryImport = null;
			}

			if (BtnRemoveCertificate != null) {
				BtnRemoveCertificate.Dispose ();
				BtnRemoveCertificate = null;
			}

			if (BtnSecondaryImport != null) {
				BtnSecondaryImport.Dispose ();
				BtnSecondaryImport = null;
			}

			if (BtnTestConnection != null) {
				BtnTestConnection.Dispose ();
				BtnTestConnection = null;
			}

			if (ChkProtect != null) {
				ChkProtect.Dispose ();
				ChkProtect = null;
			}

			if (LblPassword != null) {
				LblPassword.Dispose ();
				LblPassword = null;
			}

			if (LblSpn != null) {
				LblSpn.Dispose ();
				LblSpn = null;
			}

			if (LblUsername != null) {
				LblUsername.Dispose ();
				LblUsername = null;
			}

			if (LstCertificates != null) {
				LstCertificates.Dispose ();
				LstCertificates = null;
			}

			if (PnlProtect != null) {
				PnlProtect.Dispose ();
				PnlProtect = null;
			}

			if (PnlSpecificDomainController != null) {
				PnlSpecificDomainController.Dispose ();
				PnlSpecificDomainController = null;
			}

			if (PnlStep1 != null) {
				PnlStep1.Dispose ();
				PnlStep1 = null;
			}

			if (PnlStep2 != null) {
				PnlStep2.Dispose ();
				PnlStep2 = null;
			}

			if (PnlStep3 != null) {
				PnlStep3.Dispose ();
				PnlStep3 = null;
			}

			if (PnlStep4 != null) {
				PnlStep4.Dispose ();
				PnlStep4 = null;
			}

			if (RdoDomainController != null) {
				RdoDomainController.Dispose ();
				RdoDomainController = null;
			}

			if (RdoIdentitySource != null) {
				RdoIdentitySource.Dispose ();
				RdoIdentitySource = null;
			}

			if (RdoSpn != null) {
				RdoSpn.Dispose ();
				RdoSpn = null;
			}

			if (StepIndicator != null) {
				StepIndicator.Dispose ();
				StepIndicator = null;
			}

			if (TxtBaseDnGroups != null) {
				TxtBaseDnGroups.Dispose ();
				TxtBaseDnGroups = null;
			}

			if (TxtBaseDnUser != null) {
				TxtBaseDnUser.Dispose ();
				TxtBaseDnUser = null;
			}

			if (TxtDomainAlias != null) {
				TxtDomainAlias.Dispose ();
				TxtDomainAlias = null;
			}

			if (TxtDomainName != null) {
				TxtDomainName.Dispose ();
				TxtDomainName = null;
			}

			if (TxtGroupbaseDN != null) {
				TxtGroupbaseDN.Dispose ();
				TxtGroupbaseDN = null;
			}

			if (TxtLdapConnection != null) {
				TxtLdapConnection.Dispose ();
				TxtLdapConnection = null;
			}

			if (TxtPassword != null) {
				TxtPassword.Dispose ();
				TxtPassword = null;
			}

			if (TxtPrimaryConnection != null) {
				TxtPrimaryConnection.Dispose ();
				TxtPrimaryConnection = null;
			}

			if (TxtPrimaryUrl != null) {
				TxtPrimaryUrl.Dispose ();
				TxtPrimaryUrl = null;
			}

			if (TxtSecondaryConnection != null) {
				TxtSecondaryConnection.Dispose ();
				TxtSecondaryConnection = null;
			}

			if (TxtSpn != null) {
				TxtSpn.Dispose ();
				TxtSpn = null;
			}

			if (TxtUpn != null) {
				TxtUpn.Dispose ();
				TxtUpn = null;
			}
		}
	}

	[Register ("AddExternalIdentitySource")]
	partial class AddExternalIdentitySource
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

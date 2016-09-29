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
	[Register ("TokenWizardController")]
	partial class TokenWizardController
	{
		[Outlet]
		AppKit.NSButton BtnAcquireToken { get; set; }

		[Outlet]
		AppKit.NSButton BtnBrowseTokenFile { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnSelectCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnSelectPrivateKey { get; set; }

		[Outlet]
		AppKit.NSButton CbSaml { get; set; }

		[Outlet]
		AppKit.NSButton CbSsl { get; set; }

		[Outlet]
		AppKit.NSTextField LblCertificate { get; set; }

		[Outlet]
		AppKit.NSTextField LblPrivateKey { get; set; }

		[Outlet]
		AppKit.NSTextField LblServerUrl { get; set; }

		[Outlet]
		AppKit.NSTextField LblSnp { get; set; }

		[Outlet]
		AppKit.NSTextField LblStsUrl { get; set; }

		[Outlet]
		AppKit.NSBox PnlJwt { get; set; }

		[Outlet]
		AppKit.NSBox PnlSaml { get; set; }

		[Outlet]
		AppKit.NSMatrix RdoTypeGroup { get; set; }

		[Outlet]
		AppKit.NSTextField TxtAccessTokenString { get; set; }

		[Outlet]
		AppKit.NSTextField TxtCertificate { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDomain { get; set; }

		[Outlet]
		AppKit.NSTextField TxtIDTokenString { get; set; }

		[Outlet]
		AppKit.NSSecureTextField TxtPassword { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPort { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPrivateKey { get; set; }

		[Outlet]
		AppKit.NSTextField TxtRefreshTokenString { get; set; }

		[Outlet]
		AppKit.NSTextField TxtSamlToken { get; set; }

		[Outlet]
		AppKit.NSTextField TxtServer { get; set; }

		[Outlet]
		AppKit.NSTextField TxtSpn { get; set; }

		[Outlet]
		AppKit.NSTextField TxtStsUrl { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTenant { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTokenFile { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUsername { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (TxtPrivateKey != null) {
				TxtPrivateKey.Dispose ();
				TxtPrivateKey = null;
			}

			if (TxtCertificate != null) {
				TxtCertificate.Dispose ();
				TxtCertificate = null;
			}

			if (BtnAcquireToken != null) {
				BtnAcquireToken.Dispose ();
				BtnAcquireToken = null;
			}

			if (BtnBrowseTokenFile != null) {
				BtnBrowseTokenFile.Dispose ();
				BtnBrowseTokenFile = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnSelectCertificate != null) {
				BtnSelectCertificate.Dispose ();
				BtnSelectCertificate = null;
			}

			if (BtnSelectPrivateKey != null) {
				BtnSelectPrivateKey.Dispose ();
				BtnSelectPrivateKey = null;
			}

			if (CbSaml != null) {
				CbSaml.Dispose ();
				CbSaml = null;
			}

			if (CbSsl != null) {
				CbSsl.Dispose ();
				CbSsl = null;
			}

			if (LblCertificate != null) {
				LblCertificate.Dispose ();
				LblCertificate = null;
			}

			if (LblPrivateKey != null) {
				LblPrivateKey.Dispose ();
				LblPrivateKey = null;
			}

			if (LblServerUrl != null) {
				LblServerUrl.Dispose ();
				LblServerUrl = null;
			}

			if (LblSnp != null) {
				LblSnp.Dispose ();
				LblSnp = null;
			}

			if (LblStsUrl != null) {
				LblStsUrl.Dispose ();
				LblStsUrl = null;
			}

			if (PnlJwt != null) {
				PnlJwt.Dispose ();
				PnlJwt = null;
			}

			if (PnlSaml != null) {
				PnlSaml.Dispose ();
				PnlSaml = null;
			}

			if (RdoTypeGroup != null) {
				RdoTypeGroup.Dispose ();
				RdoTypeGroup = null;
			}

			if (TxtAccessTokenString != null) {
				TxtAccessTokenString.Dispose ();
				TxtAccessTokenString = null;
			}

			if (TxtDomain != null) {
				TxtDomain.Dispose ();
				TxtDomain = null;
			}

			if (TxtIDTokenString != null) {
				TxtIDTokenString.Dispose ();
				TxtIDTokenString = null;
			}

			if (TxtPassword != null) {
				TxtPassword.Dispose ();
				TxtPassword = null;
			}

			if (TxtPort != null) {
				TxtPort.Dispose ();
				TxtPort = null;
			}

			if (TxtRefreshTokenString != null) {
				TxtRefreshTokenString.Dispose ();
				TxtRefreshTokenString = null;
			}

			if (TxtSamlToken != null) {
				TxtSamlToken.Dispose ();
				TxtSamlToken = null;
			}

			if (TxtServer != null) {
				TxtServer.Dispose ();
				TxtServer = null;
			}

			if (TxtSpn != null) {
				TxtSpn.Dispose ();
				TxtSpn = null;
			}

			if (TxtStsUrl != null) {
				TxtStsUrl.Dispose ();
				TxtStsUrl = null;
			}

			if (TxtTenant != null) {
				TxtTenant.Dispose ();
				TxtTenant = null;
			}

			if (TxtTokenFile != null) {
				TxtTokenFile.Dispose ();
				TxtTokenFile = null;
			}

			if (TxtUsername != null) {
				TxtUsername.Dispose ();
				TxtUsername = null;
			}
		}
	}

	[Register ("TokenWizard")]
	partial class TokenWizard
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

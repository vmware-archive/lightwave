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
	[Register ("OidcClientDetailsViewController")]
	partial class OidcClientDetailsViewController
	{
		[Outlet]
		AppKit.NSButton BtnAddPostLogoutRedirectUri { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddRedirectUri { get; set; }

		[Outlet]
		AppKit.NSButton BtnApply { get; set; }

		[Outlet]
		AppKit.NSButton BtnBrowseCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemovePostLogoutRedirectUri { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveRedirectUri { get; set; }

		[Outlet]
		AppKit.NSComboBox CbTokenAuthMethod { get; set; }

		[Outlet]
		AppKit.NSTableView PostLogoutRedirectUriTableView { get; set; }

		[Outlet]
		AppKit.NSTableView RedirectUriTableView { get; set; }

		[Outlet]
		AppKit.NSTextField TxtCertificateDN { get; set; }

		[Outlet]
		AppKit.NSTextField TxtLogoutUrl { get; set; }

		[Outlet]
		AppKit.NSTextField TxtName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPostLogoutRedirectUri { get; set; }

		[Outlet]
		AppKit.NSTextField TxtRedirectUri { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (CbTokenAuthMethod != null) {
				CbTokenAuthMethod.Dispose ();
				CbTokenAuthMethod = null;
			}

			if (BtnAddPostLogoutRedirectUri != null) {
				BtnAddPostLogoutRedirectUri.Dispose ();
				BtnAddPostLogoutRedirectUri = null;
			}

			if (BtnAddRedirectUri != null) {
				BtnAddRedirectUri.Dispose ();
				BtnAddRedirectUri = null;
			}

			if (BtnApply != null) {
				BtnApply.Dispose ();
				BtnApply = null;
			}

			if (BtnBrowseCertificate != null) {
				BtnBrowseCertificate.Dispose ();
				BtnBrowseCertificate = null;
			}

			if (BtnRemovePostLogoutRedirectUri != null) {
				BtnRemovePostLogoutRedirectUri.Dispose ();
				BtnRemovePostLogoutRedirectUri = null;
			}

			if (BtnRemoveRedirectUri != null) {
				BtnRemoveRedirectUri.Dispose ();
				BtnRemoveRedirectUri = null;
			}

			if (PostLogoutRedirectUriTableView != null) {
				PostLogoutRedirectUriTableView.Dispose ();
				PostLogoutRedirectUriTableView = null;
			}

			if (RedirectUriTableView != null) {
				RedirectUriTableView.Dispose ();
				RedirectUriTableView = null;
			}

			if (TxtCertificateDN != null) {
				TxtCertificateDN.Dispose ();
				TxtCertificateDN = null;
			}

			if (TxtLogoutUrl != null) {
				TxtLogoutUrl.Dispose ();
				TxtLogoutUrl = null;
			}

			if (TxtName != null) {
				TxtName.Dispose ();
				TxtName = null;
			}

			if (TxtPostLogoutRedirectUri != null) {
				TxtPostLogoutRedirectUri.Dispose ();
				TxtPostLogoutRedirectUri = null;
			}

			if (TxtRedirectUri != null) {
				TxtRedirectUri.Dispose ();
				TxtRedirectUri = null;
			}
		}
	}
}

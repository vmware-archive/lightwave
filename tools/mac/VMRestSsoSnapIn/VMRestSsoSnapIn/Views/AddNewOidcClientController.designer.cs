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
	[Register ("AddNewOidcClientController")]
	partial class AddNewOidcClientController
	{
		[Outlet]
		AppKit.NSButton BtnAddPostLogoutRedirectUri { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddRedirectUri { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemovePostLogoutRedirectUri { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveRedirectUri { get; set; }

		[Outlet]
		AppKit.NSButton BtnSave { get; set; }

		[Outlet]
		AppKit.NSButton BtnSelectCertificate { get; set; }

		[Outlet]
		AppKit.NSComboBox CbAuthTokenMethod { get; set; }

		[Outlet]
		AppKit.NSTableView PostLogoutUtiTableView { get; set; }

		[Outlet]
		AppKit.NSTableView RedirectUriTableView { get; set; }

		[Outlet]
		AppKit.NSTextField TxtCertificateDN { get; set; }

		[Outlet]
		AppKit.NSTextField TxtLogoutUri { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPostLogoutRedirectUri { get; set; }

		[Outlet]
		AppKit.NSTextField TxtRedirectUri { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (CbAuthTokenMethod != null) {
				CbAuthTokenMethod.Dispose ();
				CbAuthTokenMethod = null;
			}

			if (BtnAddPostLogoutRedirectUri != null) {
				BtnAddPostLogoutRedirectUri.Dispose ();
				BtnAddPostLogoutRedirectUri = null;
			}

			if (BtnAddRedirectUri != null) {
				BtnAddRedirectUri.Dispose ();
				BtnAddRedirectUri = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnRemovePostLogoutRedirectUri != null) {
				BtnRemovePostLogoutRedirectUri.Dispose ();
				BtnRemovePostLogoutRedirectUri = null;
			}

			if (BtnRemoveRedirectUri != null) {
				BtnRemoveRedirectUri.Dispose ();
				BtnRemoveRedirectUri = null;
			}

			if (BtnSave != null) {
				BtnSave.Dispose ();
				BtnSave = null;
			}

			if (BtnSelectCertificate != null) {
				BtnSelectCertificate.Dispose ();
				BtnSelectCertificate = null;
			}

			if (PostLogoutUtiTableView != null) {
				PostLogoutUtiTableView.Dispose ();
				PostLogoutUtiTableView = null;
			}

			if (RedirectUriTableView != null) {
				RedirectUriTableView.Dispose ();
				RedirectUriTableView = null;
			}

			if (TxtCertificateDN != null) {
				TxtCertificateDN.Dispose ();
				TxtCertificateDN = null;
			}

			if (TxtLogoutUri != null) {
				TxtLogoutUri.Dispose ();
				TxtLogoutUri = null;
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

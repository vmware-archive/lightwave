// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
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

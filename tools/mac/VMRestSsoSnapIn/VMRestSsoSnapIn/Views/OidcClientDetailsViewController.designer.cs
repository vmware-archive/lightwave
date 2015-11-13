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

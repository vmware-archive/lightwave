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
	[Register ("AddNewTenantController")]
	partial class AddNewTenantController
	{
		[Outlet]
		AppKit.NSButton BtnAddCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnBrowsePrivateKey { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnSave { get; set; }

		[Outlet]
		AppKit.NSTableView CertificateChainTableView { get; set; }

		[Outlet]
		AppKit.NSSecureTextField TxtPassword { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPrivateKeyPath { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTenantName { get; set; }

		[Outlet]
		AppKit.NSTextField txtUsername { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnAddCertificate != null) {
				BtnAddCertificate.Dispose ();
				BtnAddCertificate = null;
			}

			if (BtnBrowsePrivateKey != null) {
				BtnBrowsePrivateKey.Dispose ();
				BtnBrowsePrivateKey = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnRemoveCertificate != null) {
				BtnRemoveCertificate.Dispose ();
				BtnRemoveCertificate = null;
			}

			if (BtnSave != null) {
				BtnSave.Dispose ();
				BtnSave = null;
			}

			if (CertificateChainTableView != null) {
				CertificateChainTableView.Dispose ();
				CertificateChainTableView = null;
			}

			if (TxtPassword != null) {
				TxtPassword.Dispose ();
				TxtPassword = null;
			}

			if (TxtPrivateKeyPath != null) {
				TxtPrivateKeyPath.Dispose ();
				TxtPrivateKeyPath = null;
			}

			if (TxtTenantName != null) {
				TxtTenantName.Dispose ();
				TxtTenantName = null;
			}

			if (txtUsername != null) {
				txtUsername.Dispose ();
				txtUsername = null;
			}
		}
	}

	[Register ("AddNewTenant")]
	partial class AddNewTenant
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

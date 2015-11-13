// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMCertStoreSnapIn
{
	[Register ("AddPrivateKeyWindowController")]
	partial class AddPrivateKeyWindowController
	{
		[Outlet]
		AppKit.NSButton AddButton { get; set; }

		[Outlet]
		AppKit.NSTextField AliasField { get; set; }

		[Outlet]
		AppKit.NSButton CancelButton { get; set; }

		[Outlet]
		AppKit.NSTextField CertificateField { get; set; }

		[Outlet]
		AppKit.NSButton OpenCertificateFileButton { get; set; }

		[Outlet]
		AppKit.NSButton OpenPrivateKeyFileButton { get; set; }

		[Outlet]
		AppKit.NSSecureTextField PasswordField { get; set; }

		[Outlet]
		AppKit.NSTextField PrivateKeyField { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (AliasField != null) {
				AliasField.Dispose ();
				AliasField = null;
			}

			if (PrivateKeyField != null) {
				PrivateKeyField.Dispose ();
				PrivateKeyField = null;
			}

			if (CertificateField != null) {
				CertificateField.Dispose ();
				CertificateField = null;
			}

			if (OpenPrivateKeyFileButton != null) {
				OpenPrivateKeyFileButton.Dispose ();
				OpenPrivateKeyFileButton = null;
			}

			if (OpenCertificateFileButton != null) {
				OpenCertificateFileButton.Dispose ();
				OpenCertificateFileButton = null;
			}

			if (AddButton != null) {
				AddButton.Dispose ();
				AddButton = null;
			}

			if (CancelButton != null) {
				CancelButton.Dispose ();
				CancelButton = null;
			}

			if (PasswordField != null) {
				PasswordField.Dispose ();
				PasswordField = null;
			}
		}
	}

	[Register ("AddPrivateKeyWindow")]
	partial class AddPrivateKeyWindow
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

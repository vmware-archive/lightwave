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
	[Register ("AddCertificateWindowController")]
	partial class AddCertificateWindowController
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
		AppKit.NSButton OpenFileButton { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (AliasField != null) {
				AliasField.Dispose ();
				AliasField = null;
			}

			if (CertificateField != null) {
				CertificateField.Dispose ();
				CertificateField = null;
			}

			if (OpenFileButton != null) {
				OpenFileButton.Dispose ();
				OpenFileButton = null;
			}

			if (AddButton != null) {
				AddButton.Dispose ();
				AddButton = null;
			}

			if (CancelButton != null) {
				CancelButton.Dispose ();
				CancelButton = null;
			}
		}
	}

	[Register ("AddCertificateWindow")]
	partial class AddCertificateWindow
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

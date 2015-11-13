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
	[Register ("CertificateDetailsViewController")]
	partial class CertificateDetailsViewController
	{
		[Outlet]
		AppKit.NSButton BtnViewCertificate { get; set; }

		[Outlet]
		AppKit.NSButton ChkActive { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDn { get; set; }

		[Outlet]
		AppKit.NSTextField TxtIssuer { get; set; }

		[Outlet]
		AppKit.NSTextField TxtValidFrom { get; set; }

		[Outlet]
		AppKit.NSTextField TxtValidTo { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (TxtValidFrom != null) {
				TxtValidFrom.Dispose ();
				TxtValidFrom = null;
			}

			if (TxtValidTo != null) {
				TxtValidTo.Dispose ();
				TxtValidTo = null;
			}

			if (TxtIssuer != null) {
				TxtIssuer.Dispose ();
				TxtIssuer = null;
			}

			if (TxtDn != null) {
				TxtDn.Dispose ();
				TxtDn = null;
			}

			if (ChkActive != null) {
				ChkActive.Dispose ();
				ChkActive = null;
			}

			if (BtnViewCertificate != null) {
				BtnViewCertificate.Dispose ();
				BtnViewCertificate = null;
			}
		}
	}
}

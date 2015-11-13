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
	[Register ("SolutionUserDetailsViewController")]
	partial class SolutionUserDetailsViewController
	{
		[Outlet]
		AppKit.NSButton BtnChangeCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnSave { get; set; }

		[Outlet]
		AppKit.NSButton BtnViewCertificate { get; set; }

		[Outlet]
		AppKit.NSButton cbDisabled { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDescription { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDn { get; set; }

		[Outlet]
		AppKit.NSTextField TxtIssuer { get; set; }

		[Outlet]
		AppKit.NSTextField TxtName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtValidFrom { get; set; }

		[Outlet]
		AppKit.NSTextField TxtValidTo { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnChangeCertificate != null) {
				BtnChangeCertificate.Dispose ();
				BtnChangeCertificate = null;
			}

			if (BtnSave != null) {
				BtnSave.Dispose ();
				BtnSave = null;
			}

			if (BtnViewCertificate != null) {
				BtnViewCertificate.Dispose ();
				BtnViewCertificate = null;
			}

			if (cbDisabled != null) {
				cbDisabled.Dispose ();
				cbDisabled = null;
			}

			if (TxtDescription != null) {
				TxtDescription.Dispose ();
				TxtDescription = null;
			}

			if (TxtDn != null) {
				TxtDn.Dispose ();
				TxtDn = null;
			}

			if (TxtName != null) {
				TxtName.Dispose ();
				TxtName = null;
			}

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
		}
	}
}

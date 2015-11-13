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
	[Register ("ViewSolutionUserDetailsController")]
	partial class ViewSolutionUserDetailsController
	{
		[Outlet]
		AppKit.NSButton BtnChangeCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnSave { get; set; }

		[Outlet]
		AppKit.NSButton CbDisabled { get; set; }

		[Outlet]
		AppKit.NSView ImageView { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDC { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDescription { get; set; }

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

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnSave != null) {
				BtnSave.Dispose ();
				BtnSave = null;
			}

			if (CbDisabled != null) {
				CbDisabled.Dispose ();
				CbDisabled = null;
			}

			if (ImageView != null) {
				ImageView.Dispose ();
				ImageView = null;
			}

			if (TxtDescription != null) {
				TxtDescription.Dispose ();
				TxtDescription = null;
			}

			if (TxtName != null) {
				TxtName.Dispose ();
				TxtName = null;
			}

			if (TxtIssuer != null) {
				TxtIssuer.Dispose ();
				TxtIssuer = null;
			}

			if (TxtValidFrom != null) {
				TxtValidFrom.Dispose ();
				TxtValidFrom = null;
			}

			if (TxtValidTo != null) {
				TxtValidTo.Dispose ();
				TxtValidTo = null;
			}

			if (TxtDC != null) {
				TxtDC.Dispose ();
				TxtDC = null;
			}
		}
	}

	[Register ("ViewSolutionUserDetails")]
	partial class ViewSolutionUserDetails
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

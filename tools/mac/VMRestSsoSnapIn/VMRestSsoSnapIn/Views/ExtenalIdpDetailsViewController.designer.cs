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
	[Register ("ExtenalIdpDetailsViewController")]
	partial class ExtenalIdpDetailsViewController
	{
		[Outlet]
		AppKit.NSButton BtnJit { get; set; }

		[Outlet]
		AppKit.NSButton BtnViewCertificate { get; set; }

		[Outlet]
		AppKit.NSTableView CertificateTableView { get; set; }

		[Outlet]
		AppKit.NSTableView NameFormatTableView { get; set; }

		[Outlet]
		AppKit.NSTableView SloTableView { get; set; }

		[Outlet]
		AppKit.NSTableView SsoTableView { get; set; }

		[Outlet]
		AppKit.NSTableView SubjectFormatTableView { get; set; }

		[Outlet]
		AppKit.NSTextField TxtEntityName { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnJit != null) {
				BtnJit.Dispose ();
				BtnJit = null;
			}

			if (CertificateTableView != null) {
				CertificateTableView.Dispose ();
				CertificateTableView = null;
			}

			if (NameFormatTableView != null) {
				NameFormatTableView.Dispose ();
				NameFormatTableView = null;
			}

			if (SloTableView != null) {
				SloTableView.Dispose ();
				SloTableView = null;
			}

			if (SsoTableView != null) {
				SsoTableView.Dispose ();
				SsoTableView = null;
			}

			if (SubjectFormatTableView != null) {
				SubjectFormatTableView.Dispose ();
				SubjectFormatTableView = null;
			}

			if (TxtEntityName != null) {
				TxtEntityName.Dispose ();
				TxtEntityName = null;
			}

			if (BtnViewCertificate != null) {
				BtnViewCertificate.Dispose ();
				BtnViewCertificate = null;
			}
		}
	}
}

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
	[Register ("AddSolutionUserController")]
	partial class AddSolutionUserController
	{
		[Outlet]
		AppKit.NSButton BtnAddNew { get; set; }

		[Outlet]
		AppKit.NSButton BtnBrowseCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnCloseNew { get; set; }

		[Outlet]
		AppKit.NSTextField TxtCertificatePath { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDescription { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUsername { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnAddNew != null) {
				BtnAddNew.Dispose ();
				BtnAddNew = null;
			}

			if (BtnCloseNew != null) {
				BtnCloseNew.Dispose ();
				BtnCloseNew = null;
			}

			if (BtnBrowseCertificate != null) {
				BtnBrowseCertificate.Dispose ();
				BtnBrowseCertificate = null;
			}

			if (TxtCertificatePath != null) {
				TxtCertificatePath.Dispose ();
				TxtCertificatePath = null;
			}

			if (TxtDescription != null) {
				TxtDescription.Dispose ();
				TxtDescription = null;
			}

			if (TxtUsername != null) {
				TxtUsername.Dispose ();
				TxtUsername = null;
			}
		}
	}

	[Register ("AddSolutionUser")]
	partial class AddSolutionUser
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

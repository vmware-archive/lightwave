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
	[Register ("AddNewServerController")]
	partial class AddNewServerController
	{
		[Outlet]
		AppKit.NSButton BtnAdd { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton CbSaml { get; set; }

		[Outlet]
		AppKit.NSButton CbSsl { get; set; }

		[Outlet]
		AppKit.NSTextField LblStsEndpoint { get; set; }

		[Outlet]
		AppKit.NSTextField LblUrl { get; set; }

		[Outlet]
		AppKit.NSSecureTextField TxtPassword { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPort { get; set; }

		[Outlet]
		AppKit.NSTextField TxtServer { get; set; }

		[Outlet]
		AppKit.NSTextField TxtStsEndpoint { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTenant { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUsername { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnAdd != null) {
				BtnAdd.Dispose ();
				BtnAdd = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (CbSaml != null) {
				CbSaml.Dispose ();
				CbSaml = null;
			}

			if (CbSsl != null) {
				CbSsl.Dispose ();
				CbSsl = null;
			}

			if (LblStsEndpoint != null) {
				LblStsEndpoint.Dispose ();
				LblStsEndpoint = null;
			}

			if (LblUrl != null) {
				LblUrl.Dispose ();
				LblUrl = null;
			}

			if (TxtPort != null) {
				TxtPort.Dispose ();
				TxtPort = null;
			}

			if (TxtServer != null) {
				TxtServer.Dispose ();
				TxtServer = null;
			}

			if (TxtStsEndpoint != null) {
				TxtStsEndpoint.Dispose ();
				TxtStsEndpoint = null;
			}

			if (TxtTenant != null) {
				TxtTenant.Dispose ();
				TxtTenant = null;
			}

			if (TxtUsername != null) {
				TxtUsername.Dispose ();
				TxtUsername = null;
			}

			if (TxtPassword != null) {
				TxtPassword.Dispose ();
				TxtPassword = null;
			}
		}
	}

	[Register ("AddNewServer")]
	partial class AddNewServer
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

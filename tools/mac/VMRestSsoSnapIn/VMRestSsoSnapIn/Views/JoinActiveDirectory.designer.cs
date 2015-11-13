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
	[Register ("JoinActiveDirectoryController")]
	partial class JoinActiveDirectoryController
	{
		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnJoin { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDomain { get; set; }

		[Outlet]
		AppKit.NSTextField TxtOU { get; set; }

		[Outlet]
		AppKit.NSSecureTextField TxtPassword { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUsername { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (TxtUsername != null) {
				TxtUsername.Dispose ();
				TxtUsername = null;
			}

			if (TxtPassword != null) {
				TxtPassword.Dispose ();
				TxtPassword = null;
			}

			if (TxtDomain != null) {
				TxtDomain.Dispose ();
				TxtDomain = null;
			}

			if (TxtOU != null) {
				TxtOU.Dispose ();
				TxtOU = null;
			}

			if (BtnJoin != null) {
				BtnJoin.Dispose ();
				BtnJoin = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}
		}
	}

	[Register ("JoinActiveDirectory")]
	partial class JoinActiveDirectory
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

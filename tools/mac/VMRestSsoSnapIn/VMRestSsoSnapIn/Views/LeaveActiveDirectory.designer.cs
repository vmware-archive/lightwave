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
	[Register ("LeaveActiveDirectoryController")]
	partial class LeaveActiveDirectoryController
	{
		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnLeave { get; set; }

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

			if (BtnLeave != null) {
				BtnLeave.Dispose ();
				BtnLeave = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}
		}
	}

	[Register ("LeaveActiveDirectory")]
	partial class LeaveActiveDirectory
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VmIdentity.UI.Common
{
	[Register ("LoginWindowController")]
	partial class LoginWindowController
	{
		[Outlet]
		AppKit.NSButton CancelButton { get; set; }

		[Outlet]
		AppKit.NSButton OKButton { get; set; }

		[Outlet]
		AppKit.NSSecureTextField PasswordTxtField { get; set; }

		[Outlet]
		AppKit.NSTextField ServerTxtField { get; set; }

		[Outlet]
		AppKit.NSTextField UserNameTxtField { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (CancelButton != null) {
				CancelButton.Dispose ();
				CancelButton = null;
			}

			if (OKButton != null) {
				OKButton.Dispose ();
				OKButton = null;
			}

			if (PasswordTxtField != null) {
				PasswordTxtField.Dispose ();
				PasswordTxtField = null;
			}

			if (UserNameTxtField != null) {
				UserNameTxtField.Dispose ();
				UserNameTxtField = null;
			}

			if (ServerTxtField != null) {
				ServerTxtField.Dispose ();
				ServerTxtField = null;
			}
		}
	}

	[Register ("LoginWindow")]
	partial class LoginWindow
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

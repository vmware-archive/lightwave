// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace UI.Password
{
	[Register ("VerifyPasswordController")]
	partial class VerifyPasswordController
	{
		[Outlet]
		AppKit.NSSecureTextField PwdTextField { get; set; }

		[Outlet]
		AppKit.NSTextField UpnTextField { get; set; }

		[Action ("OnCancelButton:")]
		partial void OnCancelButton (Foundation.NSObject sender);

		[Action ("OnOkButton:")]
		partial void OnOkButton (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (UpnTextField != null) {
				UpnTextField.Dispose ();
				UpnTextField = null;
			}

			if (PwdTextField != null) {
				PwdTextField.Dispose ();
				PwdTextField = null;
			}
		}
	}
}

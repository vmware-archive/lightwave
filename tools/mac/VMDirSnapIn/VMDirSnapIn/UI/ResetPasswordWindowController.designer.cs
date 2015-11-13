// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMDirSnapIn.UI
{
    [Register ("ResetPasswordWindowController")]
    partial class ResetPasswordWindowController
    {
        [Outlet]
        AppKit.NSTextField ConfirmPasswordTextField { get; set; }

        [Outlet]
        AppKit.NSTextField NewPasswordTextField { get; set; }

        [Action ("OnCancelButton:")]
        partial void OnCancelButton (Foundation.NSObject sender);

        [Action ("OnOKButton:")]
        partial void OnOKButton (Foundation.NSObject sender);

        void ReleaseDesignerOutlets ()
        {
            if (NewPasswordTextField != null) {
                NewPasswordTextField.Dispose ();
                NewPasswordTextField = null;
            }

            if (ConfirmPasswordTextField != null) {
                ConfirmPasswordTextField.Dispose ();
                ConfirmPasswordTextField = null;
            }
        }
    }
}

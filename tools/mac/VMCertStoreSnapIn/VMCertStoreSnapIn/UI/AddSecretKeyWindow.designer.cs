// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMCertStoreSnapIn
{
    [Register ("AddSecretKeyWindowController")]
    partial class AddSecretKeyWindowController
    {
        [Outlet]
        public AppKit.NSButton AddButton { get; set; }

        [Outlet]
        public AppKit.NSTextField AliasTextField { get; set; }

        [Outlet]
        public AppKit.NSButton CancelButton { get; set; }

        [Outlet]
        public AppKit.NSSecureTextField PasswordField { get; set; }

        [Outlet]
        public AppKit.NSTextView SecretKeyView { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (AliasTextField != null) {
                AliasTextField.Dispose ();
                AliasTextField = null;
            }

            if (SecretKeyView != null) {
                SecretKeyView.Dispose ();
                SecretKeyView = null;
            }

            if (PasswordField != null) {
                PasswordField.Dispose ();
                PasswordField = null;
            }

            if (AddButton != null) {
                AddButton.Dispose ();
                AddButton = null;
            }

            if (CancelButton != null) {
                CancelButton.Dispose ();
                CancelButton = null;
            }
        }

    }

    [Register ("AddSecretKeyWindow")]
    partial class AddSecretKeyWindow
    {
		
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMCASnapIn.UI
{
    [Register ("PrivateKeyEditorWindowController")]
    partial class PrivateKeyEditorWindowController
    {
        [Outlet]
        AppKit.NSButton CancelButton { get; set; }

        [Outlet]
        AppKit.NSButton CreateKey { get; set; }

        [Outlet]
        AppKit.NSTextField FilePath { get; set; }

        [Outlet]
        AppKit.NSComboBox KeyLength { get; set; }

        [Outlet]
        AppKit.NSButton OkButton { get; set; }

        [Outlet]
        AppKit.NSButton OpenFileButton { get; set; }

        [Outlet]
        AppKit.NSMatrix PrivateKeyOptions { get; set; }

        [Outlet]
        AppKit.NSTextView PrivateKeyTextView { get; set; }

        [Action ("rowChanged:")]
        partial void rowChanged (Foundation.NSObject sender);

        void ReleaseDesignerOutlets ()
        {
            if (CancelButton != null) {
                CancelButton.Dispose ();
                CancelButton = null;
            }

            if (CreateKey != null) {
                CreateKey.Dispose ();
                CreateKey = null;
            }

            if (FilePath != null) {
                FilePath.Dispose ();
                FilePath = null;
            }

            if (KeyLength != null) {
                KeyLength.Dispose ();
                KeyLength = null;
            }

            if (OkButton != null) {
                OkButton.Dispose ();
                OkButton = null;
            }

            if (OpenFileButton != null) {
                OpenFileButton.Dispose ();
                OpenFileButton = null;
            }

            if (PrivateKeyOptions != null) {
                PrivateKeyOptions.Dispose ();
                PrivateKeyOptions = null;
            }

            if (PrivateKeyTextView != null) {
                PrivateKeyTextView.Dispose ();
                PrivateKeyTextView = null;
            }
        }
    }

    [Register ("PrivateKeyEditorWindow")]
    partial class PrivateKeyEditorWindow
    {
		
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

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
    [Register ("AddGroupByCNWindowController")]
    partial class AddGroupByCNWindowController
    {
        [Outlet]
        AppKit.NSTextField DnLabel { get; set; }

        [Outlet]
        AppKit.NSTextField DNTextField { get; set; }

        [Outlet]
        AppKit.NSButton FindDnButton { get; set; }

        [Action ("OnCancelButton:")]
        partial void OnCancelButton (Foundation.NSObject sender);

        [Action ("OnFindDnButton:")]
        partial void OnFindDnButton (Foundation.NSObject sender);

        [Action ("OnOKButton:")]
        partial void OnOKButton (Foundation.NSObject sender);

        void ReleaseDesignerOutlets ()
        {
            if (DNTextField != null) {
                DNTextField.Dispose ();
                DNTextField = null;
            }

            if (FindDnButton != null) {
                FindDnButton.Dispose ();
                FindDnButton = null;
            }

            if (DnLabel != null) {
                DnLabel.Dispose ();
                DnLabel = null;
            }
        }
    }
}

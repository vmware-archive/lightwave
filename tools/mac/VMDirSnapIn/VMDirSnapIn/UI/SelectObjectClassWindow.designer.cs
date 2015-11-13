// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;

namespace VMDirSnapIn.UI
{
    [Register ("SelectObjectClassWindowController")]
    partial class SelectObjectClassWindowController
    {
        [Outlet]
        AppKit.NSTableView AddObjectTableView { get; set; }

        [Outlet]
        AppKit.NSButton CancelButton { get; set; }

        [Outlet]
        AppKit.NSButton SelectButton { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (AddObjectTableView != null) {
                AddObjectTableView.Dispose ();
                AddObjectTableView = null;
            }

            if (SelectButton != null) {
                SelectButton.Dispose ();
                SelectButton = null;
            }

            if (CancelButton != null) {
                CancelButton.Dispose ();
                CancelButton = null;
            }
        }
    }

    [Register ("SelectObjectClassWindow")]
    partial class SelectObjectClassWindow
    {
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

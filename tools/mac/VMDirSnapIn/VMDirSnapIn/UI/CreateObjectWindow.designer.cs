// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;

namespace VMDirSnapIn.UI
{
    [Register ("CreateObjectWindowController")]
    partial class CreateObjectWindowController
    {
        [Outlet]
        AppKit.NSButton CancelButton { get; set; }

        [Outlet]
        AppKit.NSButton CreateButton { get; set; }

        [Outlet]
        AppKit.NSButton ManageAttributesButton { get; set; }

        [Outlet]
        AppKit.NSTableView PropertiesTableView { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (PropertiesTableView != null) {
                PropertiesTableView.Dispose ();
                PropertiesTableView = null;
            }

            if (ManageAttributesButton != null) {
                ManageAttributesButton.Dispose ();
                ManageAttributesButton = null;
            }

            if (CreateButton != null) {
                CreateButton.Dispose ();
                CreateButton = null;
            }

            if (CancelButton != null) {
                CancelButton.Dispose ();
                CancelButton = null;
            }
        }
    }

    [Register ("CreateObjectWindow")]
    partial class CreateObjectWindow
    {
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

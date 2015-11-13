// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;

namespace VMDirSnapIn.UI
{
    [Register ("LdapPropertiesWindowController")]
    partial class LdapPropertiesWindowController
    {
        [Outlet]
        AppKit.NSButton ApplyButton { get; set; }

        [Outlet]
        AppKit.NSButton CancelButton { get; set; }

        [Outlet]
        AppKit.NSTableView LdapAttributesTableView { get; set; }

        [Outlet]
        AppKit.NSButton ManageAttributesButton { get; set; }

        [Outlet]
        AppKit.NSButton OKButton { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (CancelButton != null) {
                CancelButton.Dispose ();
                CancelButton = null;
            }

            if (LdapAttributesTableView != null) {
                LdapAttributesTableView.Dispose ();
                LdapAttributesTableView = null;
            }

            if (ManageAttributesButton != null) {
                ManageAttributesButton.Dispose ();
                ManageAttributesButton = null;
            }

            if (OKButton != null) {
                OKButton.Dispose ();
                OKButton = null;
            }

            if (ApplyButton != null) {
                ApplyButton.Dispose ();
                ApplyButton = null;
            }
        }
    }

    [Register ("LdapPropertiesWindow")]
    partial class LdapPropertiesWindow
    {
		
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

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
    [Register ("ConfirmationDialogController")]
    partial class ConfirmationDialogController
    {
        [Outlet]
        public AppKit.NSTextField ConfirmLabel { get; set; }

        [Outlet]
        AppKit.NSButton NoButton { get; set; }

        [Outlet]
        AppKit.NSButton YesButton { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (ConfirmLabel != null) {
                ConfirmLabel.Dispose ();
                ConfirmLabel = null;
            }

            if (YesButton != null) {
                YesButton.Dispose ();
                YesButton = null;
            }

            if (NoButton != null) {
                NoButton.Dispose ();
                NoButton = null;
            }
        }
    }

    [Register ("ConfirmationDialog")]
    partial class ConfirmationDialog
    {
		
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

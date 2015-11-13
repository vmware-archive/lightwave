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
    [Register ("AddServerWindowController")]
    partial class AddServerWindowController
    {
        [Outlet]
        public AppKit.NSButton CancelButton { get; set; }

        [Outlet]
        public AppKit.NSTextField IPTxtField { get; set; }

        [Outlet]
        public AppKit.NSButton OKButton { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (OKButton != null) {
                OKButton.Dispose ();
                OKButton = null;
            }

            if (CancelButton != null) {
                CancelButton.Dispose ();
                CancelButton = null;
            }

            if (IPTxtField != null) {
                IPTxtField.Dispose ();
                IPTxtField = null;
            }
        }
    }

    [Register ("AddServerWindow")]
    partial class AddServerWindow
    {
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

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
    [Register ("WelcomeScreenCommonController")]
    partial class WelcomeScreenCommonController
    {
        [Outlet]
        public AppKit.NSPopUpButton ConnectToServer { get; private set; }

        [Outlet]
        AppKit.NSTextField DescriptionField1 { get; set; }

        [Outlet]
        AppKit.NSTextField DescriptionField2 { get; set; }

        [Outlet]
        AppKit.NSTextField DescriptionField3 { get; set; }

        [Outlet]
        AppKit.NSTextField TitleDescriptionField { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (ConnectToServer != null) {
                ConnectToServer.Dispose ();
                ConnectToServer = null;
            }

            if (DescriptionField1 != null) {
                DescriptionField1.Dispose ();
                DescriptionField1 = null;
            }

            if (DescriptionField2 != null) {
                DescriptionField2.Dispose ();
                DescriptionField2 = null;
            }

            if (DescriptionField3 != null) {
                DescriptionField3.Dispose ();
                DescriptionField3 = null;
            }

            if (TitleDescriptionField != null) {
                TitleDescriptionField.Dispose ();
                TitleDescriptionField = null;
            }
        }
    }
}

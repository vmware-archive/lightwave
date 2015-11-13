// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;

namespace VMCASnapIn.UI
{
    [Register ("AddCertificateWindowController")]
    partial class AddCertificateWindowController
    {
        [Outlet]
        AppKit.NSButton AddButton { get; set; }

        [Outlet]
        AppKit.NSButton BrowseButton { get; set; }

        [Outlet]
        AppKit.NSButton CancelButton { get; set; }

        [Outlet]
        AppKit.NSTextField CertPathTxtField { get; set; }

        [Outlet]
        AppKit.NSButton OpenFileButton { get; set; }

        [Outlet]
        AppKit.NSTextField PrivateKeyTxtField { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (AddButton != null) {
                AddButton.Dispose ();
                AddButton = null;
            }

            if (CancelButton != null) {
                CancelButton.Dispose ();
                CancelButton = null;
            }

            if (CertPathTxtField != null) {
                CertPathTxtField.Dispose ();
                CertPathTxtField = null;
            }

            if (PrivateKeyTxtField != null) {
                PrivateKeyTxtField.Dispose ();
                PrivateKeyTxtField = null;
            }

            if (BrowseButton != null) {
                BrowseButton.Dispose ();
                BrowseButton = null;
            }

            if (OpenFileButton != null) {
                OpenFileButton.Dispose ();
                OpenFileButton = null;
            }
        }
    }

    [Register ("AddCertificateWindow")]
    partial class AddCertificateWindow
    {
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

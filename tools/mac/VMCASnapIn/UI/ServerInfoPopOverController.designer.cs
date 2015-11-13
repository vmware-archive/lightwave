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
    [Register ("ServerInfoPopOverController")]
    partial class ServerInfoPopOverController
    {
        [Outlet]
        AppKit.NSTextField ActiveCertificatesLabel { get; set; }

        [Outlet]
        AppKit.NSTextField ExpiredCertificatesLabel { get; set; }

        [Outlet]
        AppKit.NSTextField ExpiringCertificatesLabel { get; set; }

        [Outlet]
        AppKit.NSTextField RevokedCertificatesLabel { get; set; }

        [Outlet]
        AppKit.NSButton ShowRootCertButton { get; set; }

        [Action ("AddRootCertificate:")]
        partial void AddRootCertificate (Foundation.NSObject sender);

        [Action ("GetServerVersion:")]
        partial void GetServerVersion (Foundation.NSObject sender);

        [Action ("ShowRootCertificate:")]
        partial void ShowRootCertificate (Foundation.NSObject sender);

        [Action ("ValidateCA:")]
        partial void ValidateCA (Foundation.NSObject sender);

        void ReleaseDesignerOutlets ()
        {
            if (ActiveCertificatesLabel != null) {
                ActiveCertificatesLabel.Dispose ();
                ActiveCertificatesLabel = null;
            }

            if (ExpiredCertificatesLabel != null) {
                ExpiredCertificatesLabel.Dispose ();
                ExpiredCertificatesLabel = null;
            }

            if (ExpiringCertificatesLabel != null) {
                ExpiringCertificatesLabel.Dispose ();
                ExpiringCertificatesLabel = null;
            }

            if (RevokedCertificatesLabel != null) {
                RevokedCertificatesLabel.Dispose ();
                RevokedCertificatesLabel = null;
            }

            if (ShowRootCertButton != null) {
                ShowRootCertButton.Dispose ();
                ShowRootCertButton = null;
            }
        }
    }
}

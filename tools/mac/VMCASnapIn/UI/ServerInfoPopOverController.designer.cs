/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

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

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

/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

namespace VMCertStoreSnapIn
{
    [Register ("AddSecretKeyWindowController")]
    partial class AddSecretKeyWindowController
    {
        [Outlet]
        public AppKit.NSButton AddButton { get; set; }

        [Outlet]
        public AppKit.NSTextField AliasTextField { get; set; }

        [Outlet]
        public AppKit.NSButton CancelButton { get; set; }

        [Outlet]
        public AppKit.NSSecureTextField PasswordField { get; set; }

        [Outlet]
        public AppKit.NSTextView SecretKeyView { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (AliasTextField != null) {
                AliasTextField.Dispose ();
                AliasTextField = null;
            }

            if (SecretKeyView != null) {
                SecretKeyView.Dispose ();
                SecretKeyView = null;
            }

            if (PasswordField != null) {
                PasswordField.Dispose ();
                PasswordField = null;
            }

            if (AddButton != null) {
                AddButton.Dispose ();
                AddButton = null;
            }

            if (CancelButton != null) {
                CancelButton.Dispose ();
                CancelButton = null;
            }
        }

    }

    [Register ("AddSecretKeyWindow")]
    partial class AddSecretKeyWindow
    {
		
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

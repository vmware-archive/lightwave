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
    [Register ("PrivateKeyEditorWindowController")]
    partial class PrivateKeyEditorWindowController
    {
        [Outlet]
        AppKit.NSButton CancelButton { get; set; }

        [Outlet]
        AppKit.NSButton CreateKey { get; set; }

        [Outlet]
        AppKit.NSTextField FilePath { get; set; }

        [Outlet]
        AppKit.NSComboBox KeyLength { get; set; }

        [Outlet]
        AppKit.NSButton OkButton { get; set; }

        [Outlet]
        AppKit.NSButton OpenFileButton { get; set; }

        [Outlet]
        AppKit.NSMatrix PrivateKeyOptions { get; set; }

        [Outlet]
        AppKit.NSTextView PrivateKeyTextView { get; set; }

        [Action ("rowChanged:")]
        partial void rowChanged (Foundation.NSObject sender);

        void ReleaseDesignerOutlets ()
        {
            if (CancelButton != null) {
                CancelButton.Dispose ();
                CancelButton = null;
            }

            if (CreateKey != null) {
                CreateKey.Dispose ();
                CreateKey = null;
            }

            if (FilePath != null) {
                FilePath.Dispose ();
                FilePath = null;
            }

            if (KeyLength != null) {
                KeyLength.Dispose ();
                KeyLength = null;
            }

            if (OkButton != null) {
                OkButton.Dispose ();
                OkButton = null;
            }

            if (OpenFileButton != null) {
                OpenFileButton.Dispose ();
                OpenFileButton = null;
            }

            if (PrivateKeyOptions != null) {
                PrivateKeyOptions.Dispose ();
                PrivateKeyOptions = null;
            }

            if (PrivateKeyTextView != null) {
                PrivateKeyTextView.Dispose ();
                PrivateKeyTextView = null;
            }
        }
    }

    [Register ("PrivateKeyEditorWindow")]
    partial class PrivateKeyEditorWindow
    {
		
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

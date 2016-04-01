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

/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;

namespace VMDirSnapIn.UI
{
    [Register ("ManagePropertiesWindowController")]
    partial class ManagePropertiesWindowController
    {
        [Outlet]
        AppKit.NSButton AddAllButton { get; set; }

        [Outlet]
        AppKit.NSButton AddButton { get; set; }

        [Outlet]
        AppKit.NSButton ApplyButton { get; set; }

        [Outlet]
        AppKit.NSButton CancelButton { get; set; }

        [Outlet]
        AppKit.NSTableView ExistingAttributesList { get; set; }

        [Outlet]
        AppKit.NSTableView NewAttributesList { get; set; }

        [Outlet]
        AppKit.NSButton RemoveAllButton { get; set; }

        [Outlet]
        AppKit.NSButton RemoveButton { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (AddAllButton != null) {
                AddAllButton.Dispose ();
                AddAllButton = null;
            }

            if (AddButton != null) {
                AddButton.Dispose ();
                AddButton = null;
            }

            if (ApplyButton != null) {
                ApplyButton.Dispose ();
                ApplyButton = null;
            }

            if (CancelButton != null) {
                CancelButton.Dispose ();
                CancelButton = null;
            }

            if (RemoveAllButton != null) {
                RemoveAllButton.Dispose ();
                RemoveAllButton = null;
            }

            if (RemoveButton != null) {
                RemoveButton.Dispose ();
                RemoveButton = null;
            }

            if (NewAttributesList != null) {
                NewAttributesList.Dispose ();
                NewAttributesList = null;
            }

            if (ExistingAttributesList != null) {
                ExistingAttributesList.Dispose ();
                ExistingAttributesList = null;
            }
        }
    }

    [Register ("ManagePropertiesWindow")]
    partial class ManagePropertiesWindow
    {
		
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

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
    [Register("WelcomeScreenCommonController")]
    partial class WelcomeScreenCommonController
    {
        [Outlet]
        public AppKit.NSButton ConnectToServer { get; set; }

        [Outlet]
        AppKit.NSTextField DescriptionField1 { get; set; }

        [Outlet]
        AppKit.NSTextField DescriptionField2 { get; set; }

        [Outlet]
        AppKit.NSTextField DescriptionField3 { get; set; }

        [Outlet]
        AppKit.NSTextField TitleDescriptionField { get; set; }

        void ReleaseDesignerOutlets()
        {
            if (ConnectToServer != null)
            {
                ConnectToServer.Dispose();
                ConnectToServer = null;
            }

            if (DescriptionField1 != null)
            {
                DescriptionField1.Dispose();
                DescriptionField1 = null;
            }

            if (DescriptionField2 != null)
            {
                DescriptionField2.Dispose();
                DescriptionField2 = null;
            }

            if (DescriptionField3 != null)
            {
                DescriptionField3.Dispose();
                DescriptionField3 = null;
            }

            if (TitleDescriptionField != null)
            {
                TitleDescriptionField.Dispose();
                TitleDescriptionField = null;
            }
        }
    }
}

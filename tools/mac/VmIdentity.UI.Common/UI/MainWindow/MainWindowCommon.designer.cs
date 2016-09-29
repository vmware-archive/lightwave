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
    [Register("MainWindowCommonController")]
    partial class MainWindowCommonController
    {
        [Outlet]
        public VmIdentity.UI.Common.ActivatableToolBarItem BackForwardToolBarItem { get; private set; }

        [Outlet]
        public AppKit.NSView ContainerView { get; private set; }

        [Outlet]
        public AppKit.NSTextField LoggedInLabel { get; private set; }

        [Outlet]
        AppKit.NSTextField NetworkStatus { get; set; }

        [Outlet]
        public AppKit.NSToolbar MainToolBar { get; private set; }

        [Outlet]
        public VmIdentity.UI.Common.ActivatableToolBarItem FederationViewToolBarItem { get; set; }

        [Outlet]
        public VmIdentity.UI.Common.ActivatableToolBarItem RefreshToolBarItem { get; private set; }

        [Outlet]
        public VmIdentity.UI.Common.ActivatableToolBarItem SchemaViewToolBarItem { get; set; }

        [Outlet]
        public AppKit.NSSearchFieldCell SearchFieldCell { get; private set; }

        [Outlet]
        public AppKit.NSSearchField SearchRecordsField { get; private set; }

        [Outlet]
        public VmIdentity.UI.Common.ActivatableToolBarItem SearchToolBarItem { get; private set; }

        [Outlet]
        public VmIdentity.UI.Common.ActivatableToolBarItem ServerToolBarItem { get; private set; }

        void ReleaseDesignerOutlets()
        {
            if (SchemaViewToolBarItem != null)
            {
                SchemaViewToolBarItem.Dispose();
                SchemaViewToolBarItem = null;
            }

            if (FederationViewToolBarItem != null)
            {
                FederationViewToolBarItem.Dispose();
                FederationViewToolBarItem = null;
            }

            if (BackForwardToolBarItem != null)
            {
                BackForwardToolBarItem.Dispose();
                BackForwardToolBarItem = null;
            }

            if (ContainerView != null)
            {
                ContainerView.Dispose();
                ContainerView = null;
            }

            if (LoggedInLabel != null)
            {
                LoggedInLabel.Dispose();
                LoggedInLabel = null;
            }

            if (RefreshToolBarItem != null)
            {
                RefreshToolBarItem.Dispose();
                RefreshToolBarItem = null;
            }

            if (SearchFieldCell != null)
            {
                SearchFieldCell.Dispose();
                SearchFieldCell = null;
            }

            if (SearchRecordsField != null)
            {
                SearchRecordsField.Dispose();
                SearchRecordsField = null;
            }

            if (SearchToolBarItem != null)
            {
                SearchToolBarItem.Dispose();
                SearchToolBarItem = null;
            }

            if (ServerToolBarItem != null)
            {
                ServerToolBarItem.Dispose();
                ServerToolBarItem = null;
            }

            if (MainToolBar != null)
            {
                MainToolBar.Dispose();
                MainToolBar = null;
            }

            if (NetworkStatus != null)
            {
                NetworkStatus.Dispose();
                NetworkStatus = null;
            }
        }
    }

    [Register("MainWindowCommon")]
    partial class MainWindowCommon
    {
		
        void ReleaseDesignerOutlets()
        {
        }
    }
}

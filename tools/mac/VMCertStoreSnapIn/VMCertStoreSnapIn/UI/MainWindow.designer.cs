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

namespace VMCertStoreSnapIn
{
    [Register("MainWindowController")]
    partial class MainWindowController
    {
        [Outlet]
        VmIdentity.UI.Common.ActivatableToolBarItem BackForwardToolBarItem { get; set; }

        [Outlet]
        VmIdentity.UI.Common.ActivatableToolBarItem ConnectionToolBarItem { get; set; }

        [Outlet]
        public AppKit.NSView CustomView { get; private set; }

        [Outlet]
        AppKit.NSTextField LoggedInLabel { get; set; }

        [Outlet]
        VmIdentity.UI.Common.ActivatableToolBarItem RefreshToolBarItem { get; set; }

        [Outlet]
        AppKit.NSSearchFieldCell SearchFieldCell { get; set; }

        [Outlet]
        public VmIdentity.UI.Common.ActivatableToolBarItem SearchToolBarItem { get; private set; }

        [Outlet]
        VmIdentity.UI.Common.ActivatableToolBarItem ShowServerInfoToolBarItem { get; set; }

        [Action("BackForwardAction:")]
        partial void BackForwardAction(Foundation.NSObject sender);

        [Action("CreateStore:")]
        partial void CreateStore(Foundation.NSObject sender);

        [Action("GotoNextAction:")]
        partial void GotoNextAction(Foundation.NSObject sender);

        [Action("GotoPreviousAction:")]
        partial void GotoPreviousAction(Foundation.NSObject sender);

        [Action("HandleConnection:")]
        partial void HandleConnection(Foundation.NSObject sender);

        [Action("OnRefresh:")]
        partial void OnRefresh(Foundation.NSObject sender);

        [Action("ShowServerInfo:")]
        partial void ShowServerInfo(Foundation.NSObject sender);

        [Action("StartSearch:")]
        partial void StartSearch(AppKit.NSSearchField sender);

        void ReleaseDesignerOutlets()
        {
            if (SearchFieldCell != null)
            {
                SearchFieldCell.Dispose();
                SearchFieldCell = null;
            }

            if (BackForwardToolBarItem != null)
            {
                BackForwardToolBarItem.Dispose();
                BackForwardToolBarItem = null;
            }

            if (ConnectionToolBarItem != null)
            {
                ConnectionToolBarItem.Dispose();
                ConnectionToolBarItem = null;
            }

            if (CustomView != null)
            {
                CustomView.Dispose();
                CustomView = null;
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

            if (SearchToolBarItem != null)
            {
                SearchToolBarItem.Dispose();
                SearchToolBarItem = null;
            }

            if (ShowServerInfoToolBarItem != null)
            {
                ShowServerInfoToolBarItem.Dispose();
                ShowServerInfoToolBarItem = null;
            }
        }
    }

    [Register("MainWindow")]
    partial class MainWindow
    {
		
        void ReleaseDesignerOutlets()
        {
        }
    }
}

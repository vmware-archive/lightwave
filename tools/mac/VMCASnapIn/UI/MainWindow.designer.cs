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

namespace VMCASnapIn.UI
{
    [Register ("MainWindowController")]
    partial class MainWindowController
    {
        [Outlet]
        VmIdentity.UI.Common.ActivatableToolBarItem AddRootCertificateToolBarItem { get; set; }

        [Outlet]
        VmIdentity.UI.Common.ActivatableToolBarItem BackForwardToolBarItem { get; set; }

        [Outlet]
        AppKit.NSView ContainerView { get; set; }

        [Outlet]
        VmIdentity.UI.Common.ActivatableToolBarItem HandleConnectionToolBarItem { get; set; }

        [Outlet]
        AppKit.NSTextField LoggedUserLabel { get; set; }

        [Outlet]
        VmIdentity.UI.Common.ActivatableToolBarItem RefreshToolBarItem { get; set; }

        [Outlet]
        VmIdentity.UI.Common.ActivatableToolBarItem SearchToolBarItem { get; set; }

        [Outlet]
        VmIdentity.UI.Common.ActivatableToolBarItem ServerInfoToolBarItem { get; set; }

        [Outlet]
        VmIdentity.UI.Common.ActivatableToolBarItem ShowRootCertificateToolBarItem { get; set; }

        [Outlet]
        AppKit.NSSplitView splitview { get; set; }

        [Outlet]
        VmIdentity.UI.Common.ActivatableToolBarItem ValidateRootCertificateToolBarItem { get; set; }

        [Action ("AddRootCertificate:")]
        partial void AddRootCertificate (Foundation.NSObject sender);

        [Action ("BackForwardAction:")]
        partial void BackForwardAction (Foundation.NSObject sender);

        [Action ("CreateConnection:")]
        partial void CreateConnection (Foundation.NSObject sender);

        [Action ("GotoNextAction:")]
        partial void GotoNextAction (Foundation.NSObject sender);

        [Action ("GotoPreviousAction:")]
        partial void GotoPreviousAction (Foundation.NSObject sender);

        [Action ("HandleConnection:")]
        partial void HandleConnection (Foundation.NSObject sender);

        [Action ("OnRefresh:")]
        partial void OnRefresh (Foundation.NSObject sender);

        [Action ("SearchAction:")]
        partial void SearchAction (Foundation.NSObject sender);

        [Action ("ShowRootCertificate:")]
        partial void ShowRootCertificate (Foundation.NSObject sender);

        [Action ("ShowServerInfo:")]
        partial void ShowServerInfo (Foundation.NSObject sender);

        [Action ("StartSearch:")]
        partial void StartSearch (AppKit.NSSearchField sender);

        void ReleaseDesignerOutlets ()
        {
            if (RefreshToolBarItem != null) {
                RefreshToolBarItem.Dispose ();
                RefreshToolBarItem = null;
            }

            if (AddRootCertificateToolBarItem != null) {
                AddRootCertificateToolBarItem.Dispose ();
                AddRootCertificateToolBarItem = null;
            }

            if (BackForwardToolBarItem != null) {
                BackForwardToolBarItem.Dispose ();
                BackForwardToolBarItem = null;
            }

            if (ContainerView != null) {
                ContainerView.Dispose ();
                ContainerView = null;
            }

            if (HandleConnectionToolBarItem != null) {
                HandleConnectionToolBarItem.Dispose ();
                HandleConnectionToolBarItem = null;
            }

            if (LoggedUserLabel != null) {
                LoggedUserLabel.Dispose ();
                LoggedUserLabel = null;
            }

            if (SearchToolBarItem != null) {
                SearchToolBarItem.Dispose ();
                SearchToolBarItem = null;
            }

            if (ServerInfoToolBarItem != null) {
                ServerInfoToolBarItem.Dispose ();
                ServerInfoToolBarItem = null;
            }

            if (ShowRootCertificateToolBarItem != null) {
                ShowRootCertificateToolBarItem.Dispose ();
                ShowRootCertificateToolBarItem = null;
            }

            if (splitview != null) {
                splitview.Dispose ();
                splitview = null;
            }

            if (ValidateRootCertificateToolBarItem != null) {
                ValidateRootCertificateToolBarItem.Dispose ();
                ValidateRootCertificateToolBarItem = null;
            }
        }
    }

    [Register ("MainWindow")]
    partial class MainWindow
    {
		
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

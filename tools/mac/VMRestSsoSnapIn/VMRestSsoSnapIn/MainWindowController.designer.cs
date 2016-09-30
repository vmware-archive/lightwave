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

namespace RestSsoAdminSnapIn
{
	[Register ("MainWindowController")]
	partial class MainWindowController
	{
		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem AddToolbarItem { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem ADToolbarItem { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem BackForwardToolbarItem { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem BackToolbarItem { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem ComputerToolbarItem { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem ConectToolbarItem { get; set; }

		[Outlet]
		AppKit.NSTableView CrumbTableView { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem DeleteToolbarItem { get; set; }

		[Outlet]
		AppKit.NSView DetailedCustomView { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem ForwardToolbarItem { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem HttptransportToolbarItem { get; set; }

		[Outlet]
		AppKit.NSView MainCustomView { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem PropertiesToolbarItem { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem RefreshToolbarItem { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem SearchToolbarItem { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem SuperLoggingToolbarItem { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem TenantConfigToolbarItem { get; set; }

		[Outlet]
		RestSsoAdminSnapIn.ActivatableToolBarItem TokenWizardToolbarItem { get; set; }

		[Outlet]
		AppKit.NSTextField TxtLogon { get; set; }

		[Action ("BackAction:")]
		partial void BackAction (Foundation.NSObject sender);

		[Action ("BackForwardAction:")]
		partial void BackForwardAction (Foundation.NSObject sender);

		[Action ("ConnectServerAction:")]
		partial void ConnectServerAction (Foundation.NSObject sender);

		[Action ("ForwardAction:")]
		partial void ForwardAction (Foundation.NSObject sender);

		[Action ("SearchAction:")]
		partial void SearchAction (Foundation.NSObject sender);

		[Action ("SearchText:")]
		partial void SearchText (AppKit.NSSearchField sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (AddToolbarItem != null) {
				AddToolbarItem.Dispose ();
				AddToolbarItem = null;
			}

			if (ADToolbarItem != null) {
				ADToolbarItem.Dispose ();
				ADToolbarItem = null;
			}

			if (BackForwardToolbarItem != null) {
				BackForwardToolbarItem.Dispose ();
				BackForwardToolbarItem = null;
			}

			if (BackToolbarItem != null) {
				BackToolbarItem.Dispose ();
				BackToolbarItem = null;
			}

			if (ComputerToolbarItem != null) {
				ComputerToolbarItem.Dispose ();
				ComputerToolbarItem = null;
			}

			if (ConectToolbarItem != null) {
				ConectToolbarItem.Dispose ();
				ConectToolbarItem = null;
			}

			if (CrumbTableView != null) {
				CrumbTableView.Dispose ();
				CrumbTableView = null;
			}

			if (DeleteToolbarItem != null) {
				DeleteToolbarItem.Dispose ();
				DeleteToolbarItem = null;
			}

			if (DetailedCustomView != null) {
				DetailedCustomView.Dispose ();
				DetailedCustomView = null;
			}

			if (SuperLoggingToolbarItem != null) {
				SuperLoggingToolbarItem.Dispose ();
				SuperLoggingToolbarItem = null;
			}

			if (ForwardToolbarItem != null) {
				ForwardToolbarItem.Dispose ();
				ForwardToolbarItem = null;
			}

			if (HttptransportToolbarItem != null) {
				HttptransportToolbarItem.Dispose ();
				HttptransportToolbarItem = null;
			}

			if (MainCustomView != null) {
				MainCustomView.Dispose ();
				MainCustomView = null;
			}

			if (PropertiesToolbarItem != null) {
				PropertiesToolbarItem.Dispose ();
				PropertiesToolbarItem = null;
			}

			if (RefreshToolbarItem != null) {
				RefreshToolbarItem.Dispose ();
				RefreshToolbarItem = null;
			}

			if (SearchToolbarItem != null) {
				SearchToolbarItem.Dispose ();
				SearchToolbarItem = null;
			}

			if (TenantConfigToolbarItem != null) {
				TenantConfigToolbarItem.Dispose ();
				TenantConfigToolbarItem = null;
			}

			if (TokenWizardToolbarItem != null) {
				TokenWizardToolbarItem.Dispose ();
				TokenWizardToolbarItem = null;
			}

			if (TxtLogon != null) {
				TxtLogon.Dispose ();
				TxtLogon = null;
			}
		}
	}
}

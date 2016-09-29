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
// This file has been generated automatically by Xamarin Studio Business to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMPSCHighAvailability.UI
{
	[Register ("MonitorController")]
	partial class MonitorController
	{
		[Outlet]
		AppKit.NSButton AutoRefreshButton { get; set; }

		[Outlet]
		AppKit.NSView ContentPanel { get; set; }

		[Outlet]
		AppKit.NSTextField CurrentStatusTextField { get; set; }

		[Outlet]
		AppKit.NSTextField DomainControllerTextField { get; set; }

		[Outlet]
		AppKit.NSTextField HealthtextField { get; set; }

		[Outlet]
		AppKit.NSTextField HostnameHeader { get; set; }

		[Outlet]
		AppKit.NSTextField HostnameServiceHeaderTextField { get; set; }

		[Outlet]
		AppKit.NSComboBox IntervalComboBox { get; set; }

		[Outlet]
		AppKit.NSTextField IpAddressTextField { get; set; }

		[Outlet]
		AppKit.NSTextField LastRefreshTextField { get; set; }

		[Outlet]
		AppKit.NSTextField LegacyModeWarning { get; set; }

		[Outlet]
		public AppKit.NSTableView PscTableView { get; private set; }

		[Outlet]
		AppKit.NSButton RefreshButton { get; set; }

		[Outlet]
		AppKit.NSTextField ServicesheaderTextField { get; set; }

		[Outlet]
		AppKit.NSTableView ServicesTableView { get; set; }

		[Outlet]
		AppKit.NSButton SiteAffinityButton { get; set; }

		[Outlet]
		AppKit.NSTextField SitenameTextField { get; set; }

		[Outlet]
		AppKit.NSTextField StatusTextField { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (AutoRefreshButton != null) {
				AutoRefreshButton.Dispose ();
				AutoRefreshButton = null;
			}

			if (ContentPanel != null) {
				ContentPanel.Dispose ();
				ContentPanel = null;
			}

			if (CurrentStatusTextField != null) {
				CurrentStatusTextField.Dispose ();
				CurrentStatusTextField = null;
			}

			if (DomainControllerTextField != null) {
				DomainControllerTextField.Dispose ();
				DomainControllerTextField = null;
			}

			if (HealthtextField != null) {
				HealthtextField.Dispose ();
				HealthtextField = null;
			}

			if (HostnameHeader != null) {
				HostnameHeader.Dispose ();
				HostnameHeader = null;
			}

			if (HostnameServiceHeaderTextField != null) {
				HostnameServiceHeaderTextField.Dispose ();
				HostnameServiceHeaderTextField = null;
			}

			if (IntervalComboBox != null) {
				IntervalComboBox.Dispose ();
				IntervalComboBox = null;
			}

			if (IpAddressTextField != null) {
				IpAddressTextField.Dispose ();
				IpAddressTextField = null;
			}

			if (LastRefreshTextField != null) {
				LastRefreshTextField.Dispose ();
				LastRefreshTextField = null;
			}

			if (LegacyModeWarning != null) {
				LegacyModeWarning.Dispose ();
				LegacyModeWarning = null;
			}

			if (PscTableView != null) {
				PscTableView.Dispose ();
				PscTableView = null;
			}

			if (RefreshButton != null) {
				RefreshButton.Dispose ();
				RefreshButton = null;
			}

			if (ServicesheaderTextField != null) {
				ServicesheaderTextField.Dispose ();
				ServicesheaderTextField = null;
			}

			if (ServicesTableView != null) {
				ServicesTableView.Dispose ();
				ServicesTableView = null;
			}

			if (SiteAffinityButton != null) {
				SiteAffinityButton.Dispose ();
				SiteAffinityButton = null;
			}

			if (SitenameTextField != null) {
				SitenameTextField.Dispose ();
				SitenameTextField = null;
			}

			if (StatusTextField != null) {
				StatusTextField.Dispose ();
				StatusTextField = null;
			}
		}
	}
}

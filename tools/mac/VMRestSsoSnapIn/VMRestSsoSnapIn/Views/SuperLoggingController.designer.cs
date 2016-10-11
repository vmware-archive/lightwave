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

namespace RestSsoAdminSnapIn
{
	[Register ("SuperLoggingController")]
	partial class SuperLoggingController
	{
		[Outlet]
		AppKit.NSTextField AccountTextField { get; set; }

		[Outlet]
		AppKit.NSButton AutoRefreshButton { get; set; }

		[Outlet]
		AppKit.NSComboBox AutoRefreshIntervalComboBox { get; set; }

		[Outlet]
		AppKit.NSButton ClearButton { get; set; }

		[Outlet]
		AppKit.NSButton CloseButton { get; set; }

		[Outlet]
		AppKit.NSTextField CorrelationIdTextField { get; set; }

		[Outlet]
		AppKit.NSTextField DurationTextField { get; set; }

		[Outlet]
		AppKit.NSTextField EventCountTextField { get; set; }

		[Outlet]
		AppKit.NSButton EventLogActionButton { get; set; }

		[Outlet]
		AppKit.NSTextField EventsCountTextField { get; set; }

		[Outlet]
		AppKit.NSTableView EventsLogTableView { get; set; }

		[Outlet]
		AppKit.NSTextField EventTypeTextField { get; set; }

		[Outlet]
		AppKit.NSButton ExportButton { get; set; }

		[Outlet]
		AppKit.NSButton FilterButton { get; set; }

		[Outlet]
		AppKit.NSTextField FilterStatusTextField { get; set; }

		[Outlet]
		AppKit.NSButton FriendlyButton { get; set; }

		[Outlet]
		AppKit.NSScrollView FriendlyScrollView { get; set; }

		[Outlet]
		AppKit.NSTextView FriendlyTextView { get; set; }

		[Outlet]
		WebKit.WebView FriendlyWebView { get; set; }

		[Outlet]
		AppKit.NSButton ImportButton { get; set; }

		[Outlet]
		AppKit.NSButton JsonButton { get; set; }

		[Outlet]
		AppKit.NSScrollView JsonScrollView { get; set; }

		[Outlet]
		AppKit.NSTextView JsonTextField { get; set; }

		[Outlet]
		AppKit.NSTextField LastUpdatedTimestamp { get; set; }

		[Outlet]
		AppKit.NSTextField LevelIndicatorTextField { get; set; }

		[Outlet]
		AppKit.NSTextField ProviderTextField { get; set; }

		[Outlet]
		AppKit.NSButton RefreshButton { get; set; }

		[Outlet]
		AppKit.NSTextField StartTextField { get; set; }

		[Outlet]
		AppKit.NSTextField StatusBannerTextField { get; set; }

		[Outlet]
		AppKit.NSButton StatusButton { get; set; }

		[Outlet]
		AppKit.NSTextField StatusTextField { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (LastUpdatedTimestamp != null) {
				LastUpdatedTimestamp.Dispose ();
				LastUpdatedTimestamp = null;
			}

			if (AccountTextField != null) {
				AccountTextField.Dispose ();
				AccountTextField = null;
			}

			if (AutoRefreshButton != null) {
				AutoRefreshButton.Dispose ();
				AutoRefreshButton = null;
			}

			if (AutoRefreshIntervalComboBox != null) {
				AutoRefreshIntervalComboBox.Dispose ();
				AutoRefreshIntervalComboBox = null;
			}

			if (ClearButton != null) {
				ClearButton.Dispose ();
				ClearButton = null;
			}

			if (CloseButton != null) {
				CloseButton.Dispose ();
				CloseButton = null;
			}

			if (CorrelationIdTextField != null) {
				CorrelationIdTextField.Dispose ();
				CorrelationIdTextField = null;
			}

			if (DurationTextField != null) {
				DurationTextField.Dispose ();
				DurationTextField = null;
			}

			if (EventCountTextField != null) {
				EventCountTextField.Dispose ();
				EventCountTextField = null;
			}

			if (EventLogActionButton != null) {
				EventLogActionButton.Dispose ();
				EventLogActionButton = null;
			}

			if (EventsCountTextField != null) {
				EventsCountTextField.Dispose ();
				EventsCountTextField = null;
			}

			if (EventsLogTableView != null) {
				EventsLogTableView.Dispose ();
				EventsLogTableView = null;
			}

			if (EventTypeTextField != null) {
				EventTypeTextField.Dispose ();
				EventTypeTextField = null;
			}

			if (ExportButton != null) {
				ExportButton.Dispose ();
				ExportButton = null;
			}

			if (FilterButton != null) {
				FilterButton.Dispose ();
				FilterButton = null;
			}

			if (FilterStatusTextField != null) {
				FilterStatusTextField.Dispose ();
				FilterStatusTextField = null;
			}

			if (FriendlyButton != null) {
				FriendlyButton.Dispose ();
				FriendlyButton = null;
			}

			if (FriendlyScrollView != null) {
				FriendlyScrollView.Dispose ();
				FriendlyScrollView = null;
			}

			if (FriendlyTextView != null) {
				FriendlyTextView.Dispose ();
				FriendlyTextView = null;
			}

			if (FriendlyWebView != null) {
				FriendlyWebView.Dispose ();
				FriendlyWebView = null;
			}

			if (ImportButton != null) {
				ImportButton.Dispose ();
				ImportButton = null;
			}

			if (JsonButton != null) {
				JsonButton.Dispose ();
				JsonButton = null;
			}

			if (JsonScrollView != null) {
				JsonScrollView.Dispose ();
				JsonScrollView = null;
			}

			if (JsonTextField != null) {
				JsonTextField.Dispose ();
				JsonTextField = null;
			}

			if (LevelIndicatorTextField != null) {
				LevelIndicatorTextField.Dispose ();
				LevelIndicatorTextField = null;
			}

			if (ProviderTextField != null) {
				ProviderTextField.Dispose ();
				ProviderTextField = null;
			}

			if (RefreshButton != null) {
				RefreshButton.Dispose ();
				RefreshButton = null;
			}

			if (StartTextField != null) {
				StartTextField.Dispose ();
				StartTextField = null;
			}

			if (StatusBannerTextField != null) {
				StatusBannerTextField.Dispose ();
				StatusBannerTextField = null;
			}

			if (StatusButton != null) {
				StatusButton.Dispose ();
				StatusButton = null;
			}

			if (StatusTextField != null) {
				StatusTextField.Dispose ();
				StatusTextField = null;
			}
		}
	}
}

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

namespace VMDirSnapIn.UI
{
	[Register ("SuperLoggingBrowserWindowController")]
	partial class SuperLoggingBrowserWindowController
	{
		[Outlet]
		AppKit.NSButton BtnBufferSizeChange { get; set; }

		[Outlet]
		AppKit.NSButton BtnClear { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnFilter { get; set; }

		[Outlet]
		AppKit.NSButton BtnOff { get; set; }

		[Outlet]
		AppKit.NSButton BtnRefresh { get; set; }

		[Outlet]
		AppKit.NSComboBox CboColumns { get; set; }

		[Outlet]
		AppKit.NSComboBox CbOperator { get; set; }

		[Outlet]
		AppKit.NSButton ChkAutoRefresh { get; set; }

		[Outlet]
		AppKit.NSTextField Status { get; set; }

		[Outlet]
		AppKit.NSTableView SuperLogsTableView { get; set; }

		[Outlet]
		AppKit.NSTextField TxtBufferSize { get; set; }

		[Outlet]
		AppKit.NSTextField TxtFilterValue { get; set; }

		[Outlet]
		AppKit.NSTextField TxtRefreshInterval { get; set; }

		[Action ("OnClearEntries:")]
		partial void OnClearEntries (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnBufferSizeChange != null) {
				BtnBufferSizeChange.Dispose ();
				BtnBufferSizeChange = null;
			}

			if (BtnClear != null) {
				BtnClear.Dispose ();
				BtnClear = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnFilter != null) {
				BtnFilter.Dispose ();
				BtnFilter = null;
			}

			if (BtnOff != null) {
				BtnOff.Dispose ();
				BtnOff = null;
			}

			if (BtnRefresh != null) {
				BtnRefresh.Dispose ();
				BtnRefresh = null;
			}

			if (CboColumns != null) {
				CboColumns.Dispose ();
				CboColumns = null;
			}

			if (CbOperator != null) {
				CbOperator.Dispose ();
				CbOperator = null;
			}

			if (ChkAutoRefresh != null) {
				ChkAutoRefresh.Dispose ();
				ChkAutoRefresh = null;
			}

			if (Status != null) {
				Status.Dispose ();
				Status = null;
			}

			if (SuperLogsTableView != null) {
				SuperLogsTableView.Dispose ();
				SuperLogsTableView = null;
			}

			if (TxtBufferSize != null) {
				TxtBufferSize.Dispose ();
				TxtBufferSize = null;
			}

			if (TxtFilterValue != null) {
				TxtFilterValue.Dispose ();
				TxtFilterValue = null;
			}

			if (TxtRefreshInterval != null) {
				TxtRefreshInterval.Dispose ();
				TxtRefreshInterval = null;
			}
		}
	}
}

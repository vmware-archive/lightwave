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
	[Register ("ExportSearchResultController")]
	partial class ExportSearchResultController
	{
		[Outlet]
		AppKit.NSButton AddButton { get; set; }

		[Outlet]
		AppKit.NSButton AllReturnAttrCheckBox { get; set; }

		[Outlet]
		AppKit.NSComboBox AttributeToExportComboBox { get; set; }

		[Outlet]
		AppKit.NSTableView AttributeToExportTableView { get; set; }

		[Outlet]
		AppKit.NSComboBox FormatComboBox { get; set; }

		[Outlet]
		AppKit.NSButton RemoveAllButton { get; set; }

		[Outlet]
		AppKit.NSButton RemoveButton { get; set; }

		[Outlet]
		AppKit.NSComboBox ScopeComboBox { get; set; }

		[Action ("OnAdd:")]
		partial void OnAdd (Foundation.NSObject sender);

		[Action ("OnAllAttrCheckBoxClick:")]
		partial void OnAllAttrCheckBoxClick (Foundation.NSObject sender);

		[Action ("OnCancel:")]
		partial void OnCancel (Foundation.NSObject sender);

		[Action ("OnExport:")]
		partial void OnExport (Foundation.NSObject sender);

		[Action ("OnRemove:")]
		partial void OnRemove (Foundation.NSObject sender);

		[Action ("OnRemoveAll:")]
		partial void OnRemoveAll (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (RemoveButton != null) {
				RemoveButton.Dispose ();
				RemoveButton = null;
			}

			if (AddButton != null) {
				AddButton.Dispose ();
				AddButton = null;
			}

			if (RemoveAllButton != null) {
				RemoveAllButton.Dispose ();
				RemoveAllButton = null;
			}

			if (AllReturnAttrCheckBox != null) {
				AllReturnAttrCheckBox.Dispose ();
				AllReturnAttrCheckBox = null;
			}

			if (AttributeToExportComboBox != null) {
				AttributeToExportComboBox.Dispose ();
				AttributeToExportComboBox = null;
			}

			if (AttributeToExportTableView != null) {
				AttributeToExportTableView.Dispose ();
				AttributeToExportTableView = null;
			}

			if (FormatComboBox != null) {
				FormatComboBox.Dispose ();
				FormatComboBox = null;
			}

			if (ScopeComboBox != null) {
				ScopeComboBox.Dispose ();
				ScopeComboBox = null;
			}
		}
	}
}

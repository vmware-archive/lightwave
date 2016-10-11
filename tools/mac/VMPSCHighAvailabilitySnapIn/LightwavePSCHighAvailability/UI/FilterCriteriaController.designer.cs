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

namespace VMPSCHighAvailability.UI
{
	[Register ("FilterCriteriaController")]
	partial class FilterCriteriaController
	{
		[Outlet]
		AppKit.NSButton AddButton { get; set; }

		[Outlet]
		AppKit.NSButton CancelButton { get; set; }

		[Outlet]
		AppKit.NSComboBox ColumnComboBox { get; set; }

		[Outlet]
		AppKit.NSTableView FilterTableView { get; set; }

		[Outlet]
		AppKit.NSComboBox OperatorComboBox { get; set; }

		[Outlet]
		AppKit.NSButton RemoveButton { get; set; }

		[Outlet]
		AppKit.NSButton SaveButton { get; set; }

		[Outlet]
		AppKit.NSTextField ValueTextField { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (ColumnComboBox != null) {
				ColumnComboBox.Dispose ();
				ColumnComboBox = null;
			}

			if (OperatorComboBox != null) {
				OperatorComboBox.Dispose ();
				OperatorComboBox = null;
			}

			if (ValueTextField != null) {
				ValueTextField.Dispose ();
				ValueTextField = null;
			}

			if (AddButton != null) {
				AddButton.Dispose ();
				AddButton = null;
			}

			if (RemoveButton != null) {
				RemoveButton.Dispose ();
				RemoveButton = null;
			}

			if (SaveButton != null) {
				SaveButton.Dispose ();
				SaveButton = null;
			}

			if (CancelButton != null) {
				CancelButton.Dispose ();
				CancelButton = null;
			}

			if (FilterTableView != null) {
				FilterTableView.Dispose ();
				FilterTableView = null;
			}
		}
	}
}

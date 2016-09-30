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

using System;
using System.Linq;
using System.Collections.Generic;
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;

namespace Vmware.Tools.RestSsoAdminSnapIn.Helpers
{
	public static class ListViewHelper
	{
		public static List<NSTableColumn> ToNSTableColumns (List<ColumnOptions> options)
		{
			var columns = new List<NSTableColumn> ();
			foreach (var option in options.OrderBy(x=>x.DisplayOrder)) {
				var width = option.Width == 0 ? 200 : option.Width;
				var col = new NSTableColumn (option.Id) {
					
					HeaderCell = new NSTableHeaderCell { 
						Title = option.DisplayName,
						Alignment = NSTextAlignment.Left,
					},
					MinWidth = width,
					Width = width,
					ResizingMask = NSTableColumnResizing.UserResizingMask
				};
				if (option.Type == ColumnType.Browser)
					col.DataCell = new NSBrowserCell ();
				else
					col.DataCell = new NSTextFieldCell ();

				col.HeaderToolTip = "Displays " + option.DisplayName;
				columns.Add (col);
			}	
			return columns;
		}

		public static NSTableView ToNSTableView(List<ColumnOptions> options, NSTableViewDataSource nodeListView)
		{
			var view = new NSTableView ();
			var columns = ToNSTableColumns (options);
			foreach (var column in columns)
				view.AddColumn (column);
			view.DataSource = nodeListView;
			return view;
		}
	}
}


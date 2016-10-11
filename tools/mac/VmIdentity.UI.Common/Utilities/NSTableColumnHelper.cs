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

namespace VmIdentity.UI.Common.Utilities
{
	/// <summary>
	/// NS table column helper.
	/// </summary>
	public class NSTableColumnHelper
	{
		/// <summary>
		/// Creates an instance of NS table column.
		/// </summary>
		/// <returns>The NS table column.</returns>
		/// <param name="id">Identifier.</param>
		/// <param name="displayHeader">Display header.</param>
		public static NSTableColumn ToNSTableColumn(string id, string displayHeader, bool browser=false, int width=VMIdentityConstants.DefaultColumnWidth)
		{
			var col = new NSTableColumn (id) {
				HeaderCell = new NSTableHeaderCell { 
					Title = displayHeader,
					Alignment = NSTextAlignment.Center
				},
				ResizingMask = NSTableColumnResizing.UserResizingMask,
				HeaderToolTip = "Displays " + displayHeader,
			};

			if (browser)
				col.DataCell = new NSBrowserCell ();
			else
				col.DataCell = new NSTextFieldCell ();

			if (width != VMIdentityConstants.DefaultColumnWidth)
				col.Width = width;
			return col;
		}

		/// <summary>
		/// Gets the NS table view from column options.
		/// </summary>
		/// <returns>The NS table view.</returns>
		/// <param name="options">Options.</param>
		/// <param name="nodeListView">Node list view.</param>
		public static NSTableView ToNSTableView(List<ColumnOptions> options, NSTableViewDataSource nodeListView)
		{
			var view = new NSTableView ();
			var columns = ToNSTableColumns (options);
			foreach (var column in columns)
				view.AddColumn (column);
			view.DataSource = nodeListView;
			return view;
		}

		/// <summary>
		/// Converts to the NS table columns.
		/// </summary>
		/// <returns>The NS table columns.</returns>
		/// <param name="options">Options.</param>
		public static List<NSTableColumn> ToNSTableColumns (List<ColumnOptions> options)
		{
			var columns = new List<NSTableColumn> ();
			foreach (var option in options.OrderBy(x=>x.DisplayOrder)) {
				var isBrowser = option.Type == ColumnType.Browser;
				var col = ToNSTableColumn (option.Id, option.DisplayName, isBrowser, option.Width);
				columns.Add (col);
			}	
			return columns;
		}
	}
}


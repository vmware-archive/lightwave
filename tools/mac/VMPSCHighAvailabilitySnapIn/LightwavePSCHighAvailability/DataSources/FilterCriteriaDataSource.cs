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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using AppKit;
using Foundation;
using VMAFD.Client;
using VMIdentity.CommonUtils;
using VmIdentity.UI.Common.Utilities;
using VMPSCHighAvailability.Common.DTO;
using VMPSCHighAvailability.UI;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.Helpers;

namespace VMPSCHighAvailability.DataSources
{
	/// <summary>
	/// Filter Criteria data source.
	/// </summary>
	public class FilterCriteriaDataSource : NSTableViewDataSource
	{
		/// <summary>
		/// Gets or sets the entries.
		/// </summary>
		/// <value>The entries.</value>
		public List<FilterCriteriaDto> Entries { get; set; }

		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.DataSources.ServiceDataSource"/> class.
		/// </summary>
		public FilterCriteriaDataSource ()
		{
			Entries = new List<FilterCriteriaDto> ();
		}

		// This method will be called by the NSTableView control to learn the number of rows to display.
		[Export ("numberOfRowsInTableView:")]
		/// <summary>
		/// Numbers the of rows in table view.
		/// </summary>
		/// <returns>The of rows in table view.</returns>
		/// <param name="table">Table.</param>
		public int NumberOfRowsInTableView (NSTableView table)
		{
			if (Entries != null)
				return Entries.Count;
			else
				return 0;
		}

		// This method will be called by the control for each column and each row.
		[Export ("tableView:objectValueForTableColumn:row:")]
		/// <summary>
		/// Objects the value for table column.
		/// </summary>
		/// <returns>The value for table column.</returns>
		/// <param name="table">Table.</param>
		/// <param name="col">Col.</param>
		/// <param name="row">Row.</param>
		public NSObject ObjectValueForTableColumn (NSTableView table, NSTableColumn col, int row)
		{
			var value = (NSString)string.Empty;
			UIErrorHelper.CheckedExec (delegate() {
				if (Entries != null) {
					var obj = (FilterCriteriaDto)(this.Entries [row]);
					switch (col.Identifier) {
					case Constants.FilterCriteriaTableColumnId:
						value = (NSString)obj.Column;
						break;
					case Constants.FilterCriteriaTableColumnOpearatorId:
						value = (NSString) EnumHelper.GetDescription(obj.Operator);
						break;
					case Constants.FilterCriteriaTableColumnValueId:
						value = (NSString)obj.Value;
						break;
					default:
						break;
					}
				}
			});
			return value;
		}
	}
}


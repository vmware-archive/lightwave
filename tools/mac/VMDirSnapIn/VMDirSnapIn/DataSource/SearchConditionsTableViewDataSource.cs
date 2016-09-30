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
using AppKit;
using Foundation;
using VMDir.Common;
using VMDir.Common.DTO;

namespace VMDirSnapIn.DataSource
{
	public class SearchConditionsTableViewDataSource:NSTableViewDataSource
	{
		public List<FilterDTO> condList;

		public SearchConditionsTableViewDataSource()
		{
			condList = new List<FilterDTO>();
		}

		public SearchConditionsTableViewDataSource(List<FilterDTO> condList)
		{
			this.condList = condList;
		}

		// This method will be called by the NSTableView control to learn the number of rows to display.
		[Export("numberOfRowsInTableView:")]
		public int NumberOfRowsInTableView(NSTableView table)
		{
			if (condList != null)
				return this.condList.Count;
			else
				return 0;
		}

		// This method will be called by the control for each column and each row.
		[Export("tableView:objectValueForTableColumn:row:")]
		public NSObject ObjectValueForTableColumn(NSTableView table, NSTableColumn col, int row)
		{
			try
			{
				if (condList != null)
				{
					if (col.Title == "Attribute")
						return (NSString)this.condList[row].Attribute;
					else if (col.Title == "Condition")
						return (NSString)VMDirConstants.ConditionList[(int)this.condList[row].Condition];
					else if (col.Title == "Value")
						return (NSString)this.condList[row].Value;
				}
			}
			catch (Exception e)
			{
				System.Diagnostics.Debug.WriteLine("Error in List Operation " + e.Message);
			}
			return null;
		}
	}
}


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
using System.Linq;
using VMDir.Common.DTO;
using VMDirInterop.LDAP;

namespace VMDirSnapIn.DataSource
{
	public class CreateObjectTableViewDataSource : NSTableViewDataSource
	{
		public Dictionary<string, VMDirAttributeDTO> Entries { get; set; }

		public Dictionary<string, string> PendingMod { get; set; }


		public CreateObjectTableViewDataSource()
		{
			Entries = new Dictionary<string, VMDirAttributeDTO>();
			PendingMod = new Dictionary<string, string>();
		}

		public CreateObjectTableViewDataSource(Dictionary<string, VMDirAttributeDTO> classList)
		{
			PendingMod = new Dictionary<string, string>();
			Entries = classList;
		}


		// This method will be called by the NSTableView control to learn the number of rows to display.
		[Export("numberOfRowsInTableView:")]
		public int NumberOfRowsInTableView(NSTableView table)
		{
			if (Entries != null)
				return this.Entries.Count;
			else
				return 0;
		}

		// This method will be called by the control for each column and each row.
		[Export("tableView:objectValueForTableColumn:row:")]
		public NSObject ObjectValueForTableColumn(NSTableView table, NSTableColumn col, int row)
		{
			try
			{
				if (Entries != null)
				{
					string key = this.Entries.Keys.ElementAt(row);
					if (col.Identifier.Equals("Key"))
						return (NSString)key;
					else
						return (NSString)this.Entries[key].Values[0].StringValue;
				}
			}
			catch (Exception e)
			{
				System.Diagnostics.Debug.WriteLine("Error in List Operation " + e.Message);
			}
			return null;
		}

		[Export("tableView:setObjectValue:forTableColumn:row:")]
		public override void SetObjectValue(NSTableView tableView, NSObject editedVal, NSTableColumn col, nint row)
		{
			try
			{
				if (Entries != null && !string.IsNullOrEmpty(editedVal.ToString()))
				{
					if (col.Identifier == "Value")
					{
						string currKey = this.Entries.Keys.ElementAt((int)row);
						LdapValue val = new LdapValue(editedVal.ToString());
						this.Entries[currKey].Values = new List<VMDirInterop.LDAP.LdapValue>() { val };
					}
				}
			}
			catch (Exception e)
			{
				System.Diagnostics.Debug.WriteLine("Error in List Operation " + e.Message);
			}

		}

	}
}



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
using AppKit;
using CoreGraphics;
using Foundation;
using VMDir.Common;
using VMDir.Common.DTO;
using VMDir.Common.Schema;
using VMDir.Common.VMDirUtilities;
using VMDirSnapIn.DataSource;
using VMDirSnapIn.UI;

namespace VMDirSnapIn.Delegate
{
	public class ModificationStatusTableDelegate : NSTableViewDelegate
	{
		private ModificationStatusTableViewDataSource ds;

		public ModificationStatusTableDelegate(ModificationStatusTableViewDataSource ds)
		{
			this.ds = ds;
		}

		public override nint GetNextTypeSelectMatch(NSTableView tableView, nint startRow, nint endRow, string searchString)
		{
			nint row = 0;
			foreach (var item in ds.attrModStatusList)
			{
				if (item.AttributeName.Contains(searchString)) return row;
				++row;
			}
			return 0;
		}

		public override NSView GetViewForItem(NSTableView tableView, NSTableColumn tableColumn, nint row)
		{
			NSTextField view = new NSTextField();
			view.Identifier = tableColumn.Identifier;
			view.BackgroundColor = NSColor.Clear;
			view.Bordered = false;
			view.Selectable = false;
			view.Editable = false;

			// Setup view based on the column selected
			switch (tableColumn.Title)
			{
				case "Attribute":
					view.StringValue = ds.attrModStatusList[(int)row].AttributeName;
					break;
				case "Status":
					if (ds.attrModStatusList[(int)row].ModStatus)
						view.BackgroundColor = NSColor.Green;
					else
						view.BackgroundColor = NSColor.Red;
					view.StringValue = ds.attrModStatusList[(int)row].ErrorMsg;
					break;
			}

			return view;
		}
	}
}


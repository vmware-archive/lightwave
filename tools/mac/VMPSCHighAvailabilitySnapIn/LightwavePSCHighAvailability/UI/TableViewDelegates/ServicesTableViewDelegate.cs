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
using Foundation;
using AppKit;
using VmIdentity.UI.Common;
using VMPSCHighAvailability.Common.DTO;
using VMPSCHighAvailability.Common.Helpers;
using VMPSCHighAvailability.DataSources;
using VmIdentity.UI.Common.Utilities;
using System.Threading.Tasks;
using VMPSCHighAvailability.Common;

namespace VMPSCHighAvailability.UI
{
	/// <summary>
	/// Services table view delegate.
	/// </summary>
	public class ServicesTableViewDelegate : NSTableViewDelegate
	{
		/// <summary>
		/// The controller.
		/// </summary>
		private IServiceTableViewController _controller;

		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.ServicesTableViewDelegate"/> class.
		/// </summary>
		/// <param name="controller">Controller.</param>
		public ServicesTableViewDelegate (IServiceTableViewController controller)
		{
			_controller = controller;
		}

		/// <summary>
		/// Alter the display of the cell text.
		/// </summary>
		/// <param name="tableView">Table view.</param>
		/// <param name="cell">Cell.</param>
		/// <param name="tableColumn">Table column.</param>
		/// <param name="row">Row.</param>
		public override void WillDisplayCell (NSTableView tableView, NSObject cell,
			NSTableColumn tableColumn, nint row)
		{
			UIErrorHelper.CheckedExec (delegate() {

				if(cell is NSBrowserCell)
				{
					NSBrowserCell browserCell = cell as NSBrowserCell;
					if (browserCell != null) {
						browserCell.Leaf = true;
						if(tableColumn.Identifier == Constants.TableColumnIconId)
							browserCell.Image = NSImage.ImageNamed(Constants.ServiceImage);
					}
				}
				else if(cell is NSTextFieldCell)
				{
					NSTextFieldCell textCell = cell as NSTextFieldCell;

					// Apply the text color code to the status column text
					if (textCell != null && tableColumn.Identifier == Constants.ServiceTableColumnStatusId) {
						var item = _controller.GetService((int)row);
						{
							// Color Code -> Heartbeat IN-ACTIVE: Red | Heartbeat ACTIVE: GREEN | No Heartbeat: Black
							textCell.TextColor = !item.Alive
									? NSColor.Red
									: NSColor.FromSrgb((nfloat)3.0/255,(nfloat)161/255,(nfloat)27/255,1);
						}
					}
				}
			});
		}
	}
}


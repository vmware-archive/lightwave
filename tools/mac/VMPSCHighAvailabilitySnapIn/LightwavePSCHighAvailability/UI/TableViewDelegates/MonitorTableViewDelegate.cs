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
using System.Threading.Tasks;
using AppKit;
using Foundation;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;
using VMPSCHighAvailability.Common.DTO;
using VMPSCHighAvailability.DataSources;
using VMPSCHighAvailability.Common;

namespace VMPSCHighAvailability.UI
{
	/// <summary>
	/// Monitor table view delegate.
	/// </summary>
	public class MonitorTableViewDelegate : NSTableViewDelegate
	{
		/// <summary>
		/// The controller.
		/// </summary>
		private MonitorController _controller;

		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.MonitorTableViewDelegate"/> class.
		/// </summary>
		/// <param name="controller">Controller.</param>
		public MonitorTableViewDelegate (MonitorController controller)
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
							browserCell.Image = NSImage.ImageNamed(Constants.InfrastructureNodeImage);
					}
				}
				else if(cell is NSTextFieldCell)
				{
					NSTextFieldCell textCell = cell as NSTextFieldCell;

					// Apply the text color code to the status column text
					if (textCell != null && tableColumn.Identifier == Constants.PscTableColumnStatusId) {
						var collection = _controller.PscDataSource.Entries;
						if (collection != null) {
							var item = collection [(int)row];

							// Color Code -> Heartbeat: REMOTE: LightGray | IN-ACTIVE: Red | Heartbeat ACTIVE: GREEN | No Heartbeat: Black
							textCell.TextColor = 
								item.IsRemote 
								? NSColor.LightGray
								:
								(!item.Active
									? NSColor.Red
									: NSColor.FromSrgb((nfloat)3.0/255,(nfloat)161/255,(nfloat)27/255,1)
								);
						}
					}
				}
			});
		}

		/// <summary>
		/// Handle the dusplay of the server details on selection change of the table view
		/// </summary>
		/// <param name="notification">Notification.</param>
		public override void SelectionDidChange (NSNotification notification)
		{
			_controller.LoadServices ();
		}
	}
}
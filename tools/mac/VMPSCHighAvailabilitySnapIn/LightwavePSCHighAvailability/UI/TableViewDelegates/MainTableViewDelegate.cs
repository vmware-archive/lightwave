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
	public class MainTableViewDelegate : NSTableViewDelegate
	{
		/// <summary>
		/// The controller.
		/// </summary>
		private MainTableController _controller;

		/// <summary>
		/// The main controller.
		/// </summary>
		private VMPSCHighAvailabilityMainWindowController _ob;

		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.MonitorTableViewDelegate"/> class.
		/// </summary>
		/// <param name="controller">Controller.</param>
		public MainTableViewDelegate (MainTableController controller, VMPSCHighAvailabilityMainWindowController ob)
		{
			_controller = controller;
			_ob = ob;
		}

		/// <summary>
		/// Gets the type of the image on node.
		/// </summary>
		/// <returns>The image on node type.</returns>
		/// <param name="datasource">Datasource.</param>
		/// <param name="row">Row.</param>
		/// <param name="ob">Ob.</param>
		private NSImage GetImageOnNodeType(PscDataSource datasource, nint row, VMPSCHighAvailabilityMainWindowController ob )
		{
			var collection = datasource.Entries;
			var isInfra = false;
			if (collection != null) {
				var item = collection [(int)row];
				isInfra = (item.NodeType == VMPSCHighAvailability.Common.NodeType.Infrastructure);
			}
			return isInfra
				? _ob.CachedImages [(int)ImageIndex.Infrastructure]
					: _ob.CachedImages [(int)ImageIndex.Management];
		}

		/// <summary>
		/// Gets the type of the image on node.
		/// </summary>
		/// <returns>The image on node type.</returns>
		/// <param name="datasource">Datasource.</param>
		/// <param name="row">Row.</param>
		/// <param name="ob">Ob.</param>
		private NSImage GetImageOnNodeType(NodeDataSource datasource, nint row, VMPSCHighAvailabilityMainWindowController ob )
		{
			var collection = datasource.Entries;
			var isInfra = false;
			if (collection != null) {
				var item = collection [(int)row];
				isInfra = (item.NodeType == VMPSCHighAvailability.Common.NodeType.Infrastructure);
			}
			return isInfra
				? _ob.CachedImages [(int)ImageIndex.Infrastructure]
					: _ob.CachedImages [(int)ImageIndex.Management];
		}

		/// <summary>
		/// Gets the color of the status text.
		/// </summary>
		/// <returns>The status text color.</returns>
		/// <param name="item">Item.</param>
		private NSColor GetStatusTextColor (NodeDto item)
		{
			return (!item.Active ? NSColor.Red : NSColor.FromSrgb ((nfloat)3.0 / 255, (nfloat)161 / 255, (nfloat)27 / 255, 1));
		}

		/// <summary>
		/// Gets the color of the status text.
		/// </summary>
		/// <returns>The status text color.</returns>
		/// <param name="item">Item.</param>
		private NSColor GetStatusTextColor (ServiceDto item)
		{
			return (!item.Alive ? NSColor.Red : NSColor.FromSrgb ((nfloat)3.0 / 255, (nfloat)161 / 255, (nfloat)27 / 255, 1));
		}

		/// <summary>
		/// Gets the icon for row.
		/// </summary>
		/// <returns>The icon for row.</returns>
		/// <param name="row">Row.</param>
		/// <param name="source">Source.</param>
		private NSImage GetIconForRow (nint row, INSTableViewDataSource source)
		{
			NSImage icon = null;
			if (source is PscDataSource) {
				icon = GetImageOnNodeType ((PscDataSource)source, row, _ob);
			} else if (source is NodeDataSource) {
				icon = GetImageOnNodeType ((NodeDataSource)source, row, _ob);
			} else if (source is ServiceDataSource) {
				icon = _ob.CachedImages [(int)ImageIndex.Service];
			}
			return icon;
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

				if (cell is NSTextFieldCell) {
					NSTextFieldCell textCell = cell as NSTextFieldCell;

					// Apply the text color code to the status column text
					if (textCell != null && tableColumn.Identifier == Constants.PscTableColumnStatusId) {

						if((_controller.MainTableView.DataSource) is NodeDataSource)
						{
							var datasource = (_controller.MainTableView.DataSource) as NodeDataSource;
							var collection = datasource.Entries;
							if (collection != null && (((int)row) < collection.Count)) {
								var item = collection [(int)row];

								// Color Code -> Heartbeat IN-ACTIVE: Red | Heartbeat ACTIVE: GREEN | No Heartbeat: Black
								textCell.TextColor = GetStatusTextColor (item);
							}
						}
						else if((_controller.MainTableView.DataSource) is PscDataSource)
						{
							var datasource = (_controller.MainTableView.DataSource) as PscDataSource;
							var collection = datasource.Entries;
							if (collection != null && (((int)row) < collection.Count)) {
								var item = collection [(int)row];

								// Color Code -> Heartbeat IN-ACTIVE: Red | Heartbeat ACTIVE: GREEN | No Heartbeat: Black
								textCell.TextColor = GetStatusTextColor (item);
							}
						} else if((_controller.MainTableView.DataSource) is ServiceDataSource)
						{
							var datasource = (_controller.MainTableView.DataSource) as ServiceDataSource;
							var collection = datasource.Entries;
							if (collection != null  && (((int)row) < collection.Count)) {
								var item = collection [(int)row];

								// Color Code -> Heartbeat IN-ACTIVE: Red | Heartbeat ACTIVE: GREEN | No Heartbeat: Black
								textCell.TextColor = GetStatusTextColor (item);
							}
						}
					}
				} else if (cell is NSBrowserCell) {
					NSBrowserCell browserCell = cell as NSBrowserCell;
					if (browserCell != null) {
						browserCell.Leaf = true;
						if (tableColumn.Identifier == Constants.TableColumnIconId) {
							var source = _controller.MainTableView.DataSource;
							var icon = GetIconForRow (row, source);
							if (icon != null)
								browserCell.Image = icon;
						}
					}
				}
			});
		}

		/// <summary>
		/// Handle the display of the server details on selection change of the table view
		/// </summary>
		/// <param name="notification">Notification.</param>
//		public override void SelectionDidChange (NSNotification notification)
//		{
//			var source = _controller.MainTableView.DataSource as PscDataSource;
//			var row = (int)_controller.MainTableView.SelectedRow;
//			if (row > -1 && row < source.Entries.Count) {
//				var connect = !source.Entries[row].HeartbeatStatus.HasValue;
//				_controller.EnableConnect (connect);
//
//				// Show the server pop over.
//				if (!connect && source.Entries [row].HeartbeatStatus.Value.dwCount > 0) {
//					_controller.ShowServiceDetails (_controller.PscTableView, source.Entries [row].HeartbeatStatus.Value.info);
//				}
//			}
//		}
	}
}
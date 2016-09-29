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
using Foundation;
using VMDirSnapIn.Nodes;
using VMDirSnapIn.UI;
using System.Linq;
using VmIdentity.UI.Common.Utilities;
using VMDir.Common;

namespace VMDirSnapIn.Delegate
{
	public class OutlineDelegate : NSOutlineViewDelegate
	{
		private NSImage directoryIcon, worldIcon, userIcon, groupIcon;
		MainWindowController mwCtl;

		public OutlineDelegate(MainWindowController mwCtl)
		{
			this.mwCtl = mwCtl;
			directoryIcon = NSImage.ImageNamed("directoryObject.png");
			worldIcon = NSImage.ImageNamed("home.png");
			userIcon = NSImage.ImageNamed("UserImg.png");
			groupIcon = NSImage.ImageNamed("GroupImg.png");
		}


		public override void WillDisplayCell(NSOutlineView outlineView, NSObject cell,
											 NSTableColumn tableColumn, NSObject item)
		{
			try
			{
				NSBrowserCell browserCell = cell as NSBrowserCell;
				if (browserCell != null)
				{
					browserCell.Leaf = true;
					var node = item as DirectoryNode;
					if (node.IsBaseNode)
						browserCell.Image = worldIcon;
					else if (string.Equals(node.ObjectClass, VMDirConstants.USER_OC))
						browserCell.Image = userIcon;
					else if (string.Equals(node.ObjectClass, VMDirConstants.GROUP_OC))
						browserCell.Image = groupIcon;
					else
						browserCell.Image = directoryIcon;

				}
			}
			catch (Exception e)
			{
				System.Diagnostics.Debug.WriteLine("Exception in casting : " + e.Message);
			}
		}



		public override void SelectionDidChange(NSNotification notification)
		{
			nint row = mwCtl.MainOutlineView.SelectedRow;
			mwCtl.RefreshTableViewBasedOnSelection(row);
		}

		public override void ItemDidExpand(NSNotification notification)
		{
			UIErrorHelper.CheckedExec(delegate
			{
				if (notification.UserInfo != null)
				{
					var kp = notification.UserInfo.FirstOrDefault();

					var node = kp.Value as DirectoryNode;
					if (node != null)
						node.Expand(node.Dn);
				}
			});
		}
	}
}


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
using VMPSCHighAvailability.UI;
using VMPSCHighAvailability.Nodes;
using VMPSCHighAvailability.Common;

namespace VMPSCHighAvailability.UI
{
	class OutlineDelegate : NSOutlineViewDelegate
	{
		private VMPSCHighAvailabilityMainWindowController ob;

		public OutlineDelegate (VMPSCHighAvailabilityMainWindowController ob)
		{
			this.ob = ob;
		}

		public override void WillDisplayCell (NSOutlineView outlineView, NSObject cell,
			NSTableColumn tableColumn, NSObject item)
		{
			try {
				NSBrowserCell browserCell = cell as NSBrowserCell;
				if (browserCell != null) {
					browserCell.Leaf = true;

					if (item is GlobalTopologyNode)
						browserCell.Image = ob.CachedImages [(int)ImageIndex.Global];
					else if (item is SiteNode)
						browserCell.Image = ob.CachedImages [(int)ImageIndex.Site];
					else if (item is InfrastucturesGroupNode)
						browserCell.Image = ob.CachedImages [(int)ImageIndex.InfraGroup];
					else if (item is ManagementsGroupNode)
						browserCell.Image = ob.CachedImages [(int)ImageIndex.ManagementGroup];
					else if (item is InfrastructureNode)
						browserCell.Image = ob.CachedImages [(int)ImageIndex.Infrastructure];
					else if (item is ManagementNode)
						browserCell.Image = ob.CachedImages [(int)ImageIndex.Management];
					else
						browserCell.Image = ob.CachedImages [(int)ImageIndex.Management];
				}

			} catch (Exception e) {
				System.Diagnostics.Debug.WriteLine ("Exception in casting : " + e.Message);
			}
		}

		public override void SelectionDidChange (NSNotification notification)
		{
			ob.OnOutlineViewActivated (this, EventArgs.Empty);
		}

	}
}


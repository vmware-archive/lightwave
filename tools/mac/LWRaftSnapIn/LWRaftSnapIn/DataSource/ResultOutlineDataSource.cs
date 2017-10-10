/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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
using LWRaftSnapIn.Nodes;

namespace LWRaftSnapIn.DataSource
{
	public class ResultOutlineDataSource : NSOutlineViewDataSource
	{
		public List<DirectoryNode> ResultList { get; set; }

		public ResultOutlineDataSource() : base()
		{
			ResultList = new List<DirectoryNode>();
		}
		public ResultOutlineDataSource(List<DirectoryNode> resultList) : base()
		{
			ResultList = resultList;
		}

		public override nint GetChildrenCount(NSOutlineView outlineView, NSObject item)
		{
			if (ResultList != null)
				return ResultList.Count;
			else
				return 0;
		}

		public override bool ItemExpandable(NSOutlineView outlineView, NSObject item)
		{
			return false;
		}

		public override NSObject GetObjectValue(NSOutlineView outlineView, NSTableColumn tableColumn, NSObject item)
		{
			if (item == null)
			{
				System.Diagnostics.Debug.WriteLine("passed null, returning empty String");
				return new NSString(" ");
			}
			else {
				DirectoryNode passedNode = item as DirectoryNode;
				if (passedNode != null)
				{
					return (NSString)passedNode.Dn;
				}
				else {
					System.Diagnostics.Debug.WriteLine("returning an empty string, cast failed.");
					return new NSString();
				}
			}
		}

		public override NSObject GetChild(NSOutlineView outlineView, nint childIndex, NSObject item)
		{
			return ResultList[(int)childIndex];
		}
	}
}


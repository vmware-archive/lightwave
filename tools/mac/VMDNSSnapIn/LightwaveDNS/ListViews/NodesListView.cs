﻿/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using VMDNS.Common;
using VmIdentity.UI.Common;


namespace VMDNS.ListViews
{
    public class NodesListView : NSTableViewDataSource
    {
        public List<ScopeNodeBase> Entries { get; set; }

        public VMDNSServerDTO ServerDTO { get; set; }

        public NodesListView()
        {
            Entries = new List<ScopeNodeBase>();
        }

        public NodesListView(List<ScopeNodeBase> nodesList, VMDNSServerDTO dto)
        {
            Entries = nodesList;
            ServerDTO = dto;
        }

        // This method will be called by the NSTableView control to learn the number of rows to display.
        [Export("numberOfRowsInTableView:")]
        public int NumberOfRowsInTableView(NSTableView table)
        {
            return (Entries != null) ? Entries.Count : 0;
        }

        // This method will be called by the control for each column and each row.
        [Export("tableView:objectValueForTableColumn:row:")]
        public NSObject ObjectValueForTableColumn(NSTableView table, NSTableColumn col, int row)
        {
            NSString value = NSString.Empty;
            try
            {
                if (Entries != null)
                    value = (NSString)this.Entries[row].DisplayName;
            }
            catch (Exception e)
            {
            }
            return value;
        }
    }
}


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


namespace VMDNS.ListViews
{
    public class ForwardersListView : NSTableViewDataSource
    {
        private  List<string> Entries { get; set; }

        public ForwardersListView()
        {
            Entries = null;
        }

        public ForwardersListView(List<string> entries)
        {
            Entries = entries;

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
                {
                    switch (col.Identifier)
                    {
                        case "Forwarders":
                            value = (NSString)Entries[row];
                            break;
                        default:
                            value = NSString.Empty;
                            break;
                    }
                }
            }
            catch (Exception e)
            {
                //Exception in casting.
            }
            return value;
        }
    }
}



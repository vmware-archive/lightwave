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
using VMDirSchema;
using System.Linq;

namespace VMDirSchemaEditorSnapIn
{
    public class DiffListView : NSTableViewDataSource
    {
        List<KeyValuePair<string,string>> Entries;

        public DiffListView(List<KeyValuePair<string,string>> entries)
        {
            this.Entries = entries;
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
                    KeyValuePair<string,string> val = Entries.ElementAt(row);
                    switch (col.Identifier)
                    {
                        case VMDirSchemaConstants.NODE1:
                            value = (NSString)val.Key;
                            break;
                        case VMDirSchemaConstants.NODE2:
                            value = (NSString)val.Value;
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


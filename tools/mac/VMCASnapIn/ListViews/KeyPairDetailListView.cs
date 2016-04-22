/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
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
using VMCASnapIn.DTO;

namespace VMCASnapIn.ListViews
{
    public class KeyPairDetailListView : NSTableViewDataSource
    {
        private List<KeyPairDTO> _entries;

        public List<KeyPairDTO> Entries{ get { return _entries; } }


        public KeyPairDetailListView ()
        {
            _entries = new List<KeyPairDTO> ();
        }

        public KeyPairDetailListView (List<KeyPairDTO> nodesList)
        {
            _entries = nodesList;
        }

        // This method will be called by the NSTableView control to learn the number of rows to display.
        [Export ("numberOfRowsInTableView:")]
        public int NumberOfRowsInTableView (NSTableView table)
        {
            if (_entries != null)
                return _entries.Count;
            else
                return 0;
        }

        // This method will be called by the control for each column and each row.
        [Export ("tableView:objectValueForTableColumn:row:")]
        public NSObject ObjectValueForTableColumn (NSTableView table, NSTableColumn col, int row)
        {
            try {
                if (_entries != null) {
                    if (col.Identifier == "CreatedDate")
                        return (NSString)this._entries [row].CreatedDateTime.ToLocalTime ().ToString ();
                    else
                        return (NSString)this._entries [row].KeyLength.ToString ();
                }
            } catch (Exception e) {
                System.Diagnostics.Debug.WriteLine ("Error in List Operation " + e.Message);
            }
            return null;
        }
    }
}


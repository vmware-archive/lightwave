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
using System.Linq;
using VMDir.Common.VMDirUtilities;
using VMDirInterop.LDAP;

namespace VMDirSnapIn.DataSource
{
    public class PropertiesTableViewDataSource: NSTableViewDataSource
    {
        public Dictionary<string,VMDirBagItem> Entries { get; set; }

        public List<KeyValuePair<string,string>> data { get; set; }

        public Dictionary<string,string> PendingMod { get; set; }


        public PropertiesTableViewDataSource ()
        {
            Entries = new Dictionary<string,VMDirBagItem> ();
            PendingMod = new Dictionary<string,string> ();
        }

        public PropertiesTableViewDataSource (Dictionary<string,VMDirBagItem>  classList)
        {
            PendingMod = new Dictionary<string,string> ();
            Entries = classList;
            data = new List<KeyValuePair<string,string>> ();
            FillData ();
        }

        public void FillData ()
        {
            data.Clear ();
            foreach (var entry in Entries) {
                object value = entry.Value.Value;
                if (value != null) {
                    Type valueType = value.GetType ();
                    if (valueType.IsArray) {
                        LdapValue[] arr = value as LdapValue[];
                        foreach (var arrayElement in arr) {
                            data.Add (new KeyValuePair<string, string> (entry.Key, arrayElement.StringValue));
                        }

                    } else {
                        var LdapEntry = (LdapValue)value;
                        data.Add (new KeyValuePair<string, string> (entry.Key, LdapEntry.StringValue));
                    }
                } else
                    data.Add (new KeyValuePair<string, string> (entry.Key, string.Empty));
            }
        }

        // This method will be called by the NSTableView control to learn the number of rows to display.
        [Export ("numberOfRowsInTableView:")]
        public int NumberOfRowsInTableView (NSTableView table)
        {
            if (data != null)
                return data.Count;
            else
                return 0;
        }

        // This method will be called by the control for each column and each row.
        [Export ("tableView:objectValueForTableColumn:row:")]
        public NSObject ObjectValueForTableColumn (NSTableView table, NSTableColumn col, int row)
        {
            try {
                if (data != null) {
                    if (col.Identifier.Equals ("Key"))
                        return (NSString)data [row].Key;
                    else
                        return (NSString)data [row].Value;
                }
            } catch (Exception e) {
                System.Diagnostics.Debug.WriteLine ("Error in List Operation " + e.Message);
            }
            return null;
        }

        [Export ("tableView:setObjectValue:forTableColumn:row:")]
        public override void SetObjectValue (NSTableView tableView, NSObject editedVal, NSTableColumn col, nint row)
        {
            try {
                if (data != null && !string.IsNullOrEmpty (editedVal.ToString ())) {
                    if (col.Identifier == "Value") {
                        string currKey = this.data [(int)row].Key;
                        if (currKey != "objectClass") {
                            this.data [(int)row] = new KeyValuePair<string, string> (currKey, (NSString)editedVal);
                            this.PendingMod.Add (currKey, this.data [(int)row].Value);
                        }
                    }
                }
            } catch (Exception e) {
                System.Diagnostics.Debug.WriteLine ("Error in List Operation " + e.Message);
            }

        }

    }
}



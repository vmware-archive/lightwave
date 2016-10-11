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
using VMDNS;
using VMDNS.Common;
using System.Linq;

/*
 * Zone Details DataSource for TableView
 *
 * @author Sumalatha Abhishek
 */

namespace VMDNS.ListViews
{
    public class ZoneDetailsListView : NSTableViewDataSource
    {
        public  IList<VMDNSZoneEntryNode> Entries { get; set; }

        public ZoneDetailsListView()
        {
            Entries = null;
        }

        public ZoneDetailsListView(VMDNSZonesBaseNode zoneNodes)
        {
            Entries = zoneNodes.Children.OfType<VMDNSZoneEntryNode>().ToList();
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
                    VMDNSZoneEntryNode zone = Entries[row];
                    switch (col.Identifier)
                    {
                        case VMDNSConstants.ZONE_NAME:
                            value = (NSString)zone.CurrentZone.Name;
                            break;
                        case VMDNSConstants.DNS_NAME:
                            value = (NSString)zone.CurrentZone.DNSName;
                            break;
                        case VMDNSConstants.ADMIN_EMAIL:
                            value = (NSString)zone.CurrentZone.AdminEmail;
                            break;
                    }
                }
            }
            catch (Exception e)
            {
                //Todo - log exception
            }
            return value;
        }
    }
}


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
using VMDNS.Client;
using VMDNS;
using VMDNS.Common;

/*
 * DNS Record Details DataSource for TableView
 *
 * @author Sumalatha Abhishek
 */

namespace VMDNS.ListViews
{
    public class DnsRecordListView : NSTableViewDataSource
    {
        public  IList<VmDnsRecord> Entries { get; set; }

        public VMDNSZoneEntryNode ZoneNode;

        public DnsRecordListView(VMDNSZoneEntryNode zoneNode, IList<VmDnsRecord> records)
        {
            this.ZoneNode = zoneNode;
            Entries = records;
        }

        public DnsRecordListView(VMDNSZoneEntryNode zoneNode)
        {
            this.ZoneNode = zoneNode;
            try
            {
                Entries = zoneNode.CurrentZone.ListRecords();
            }
            catch (Exception)
            {
                throw;
            }
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
                    VmDnsRecord record = Entries[row];
                    switch (col.Identifier)
                    {
                        case VMDNSConstants.RECORD_NAME:
                            value = (NSString)record.Name;
                            break;
                        case VMDNSConstants.RECORD_TYPE:
                            value = (NSString)VMDNSUtilityService.GetRecordNameFromType((RecordType)record.Type);
                            break;
                    }
                }
            }
            catch (Exception)
            {
                value = NSString.Empty;
            }
            return value;
        }
    }
}



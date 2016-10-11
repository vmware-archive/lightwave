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
using VmIdentity.UI.Common;
using AppKit;
using CoreGraphics;
using Foundation;
using VMDNS.ListViews;
using VMDNS.Client;
using VMDNS.Common;

namespace VMDNS
{
    [Register("CustomTableView")]
    public class DNSTableView:NSTableView
    {
        private nint selectedRow;

        [Export("init")]
        public DNSTableView()
            : base()
        {
        }

        [Export("initWithCoder:")]
        public DNSTableView(NSCoder coder)
            : base(coder)
        {
        }

        public DNSTableView(IntPtr handle)
            : base(handle)
        {

        }

        public override NSMenu MenuForEvent(NSEvent theEvent)
        {
            CGPoint pt = this.ConvertPointFromView(theEvent.LocationInWindow, null);
            selectedRow = this.GetRow(pt);
            NSTableViewDataSource ds = (NSTableViewDataSource)this.DataSource;
            NSMenu menu = new NSMenu();
            if (selectedRow >= (nint)0)
            {
                if (ds is ZoneDetailsListView)
                {
                    VMDNSZoneEntryNode zone = (ds as ZoneDetailsListView).Entries[(int)SelectedRow] as VMDNSZoneEntryNode;
                    NSMenuItem properties = new NSMenuItem(VMDNSConstants.ZONE_PROPERTIES, zone.OnClickZoneProperties);
                    menu.AddItem(properties);
                }
                else if (ds is DnsRecordListView)
                {
                    VMDNSZoneEntryNode zoneNode = (ds as DnsRecordListView).ZoneNode;
                    if (zoneNode != null)
                    {
                        VmDnsRecord record = (ds as DnsRecordListView).Entries[(int)selectedRow];
                        NSMenuItem properties = new NSMenuItem(VMDNSConstants.RECORD_PROPERTIES, (sender, e) => zoneNode.ShowRecordProperties(sender, e, record));
                        menu.AddItem(properties);
                        NSMenuItem deleteRecord = new NSMenuItem(VMDNSConstants.RECORD_DELETE, (sender, e) => zoneNode.DeleteRecord(sender, e, record));
                        menu.AddItem(deleteRecord);
                    }
                }
            }
            NSMenu.PopUpContextMenu(menu, theEvent, theEvent.Window.ContentView);
            return base.MenuForEvent(theEvent);
        }
    }
}


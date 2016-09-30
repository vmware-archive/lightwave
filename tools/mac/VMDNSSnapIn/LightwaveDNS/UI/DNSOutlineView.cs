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
using VMDNS.Nodes;
using VMDNS.Common;

namespace VMDNS
{
    [Register("CustomOutlineView")]
    public class DNSOutlineView:NSOutlineView
    {
        [Export("init")]
        public DNSOutlineView()
            : base()
        {
        }

        [Export("initWithCoder:")]
        public DNSOutlineView(NSCoder coder)
            : base(coder)
        {
        }

        public DNSOutlineView(IntPtr handle)
            : base(handle)
        {
        }

        public override NSMenu MenuForEvent(NSEvent theEvent)
        {
            CGPoint pt = this.ConvertPointFromView(theEvent.LocationInWindow, null);
            nint row = this.GetRow(pt);
            if (row >= (nint)0)
            {
                NSObject obj = this.ItemAtRow(row);
                if (obj != null)
                {
                    NSMenu menu = new NSMenu();
                    if (obj is VMDNSZoneEntryNode)
                    {
                        var node = obj as VMDNSZoneEntryNode;
                        NSMenuItem properties = new NSMenuItem(VMDNSConstants.ZONE_PROPERTIES, node.OnClickZoneProperties);
                        menu.AddItem(properties);
                        NSMenuItem deleteZone = new NSMenuItem(VMDNSConstants.ZONE_DELETE, node.DeleteZone);
                        menu.AddItem(deleteZone);
                        menu.AddItem(NSMenuItem.SeparatorItem);
                        NSMenuItem addRecord = new NSMenuItem(VMDNSConstants.RECORD_ADD, node.AddRecord);
                        menu.AddItem(addRecord);
                    }
                    else if (obj is VMDNSForwardZonesNode)
                    {
                        var node = obj as VMDNSForwardZonesNode;
                        NSMenuItem addZones = new NSMenuItem(VMDNSConstants.ZONE_ADD_FORWARD, node.AddForwardZone);
                        menu.AddItem(addZones);
                    }
                    else if (obj is VMDNSReverseZonesNode)
                    {
                        var node = obj as VMDNSReverseZonesNode;
                        NSMenuItem addZones = new NSMenuItem(VMDNSConstants.ZONE_ADD_REVERSE, node.AddReverseZone);
                        menu.AddItem(addZones);
                    }
                    else if (obj is VMDNSRootScopeNode)
                    {
                        var node = (obj as VMDNSRootScopeNode).ServerNode;
                        NSMenuItem serverConfig = new NSMenuItem(VMDNSConstants.SERVER_CONFIG, node.ViewServerConfiguration);
                        menu.AddItem(serverConfig);
                        menu.AddItem(NSMenuItem.SeparatorItem);
                        NSMenuItem refresh = new NSMenuItem(VMDNSConstants.REFRESH, node.Refresh);
                        menu.AddItem(refresh);
                    }

                    NSMenu.PopUpContextMenu(menu, theEvent, theEvent.Window.ContentView);
                }
            }
            return base.MenuForEvent(theEvent);
        }
    }
}


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
using VMDNS.Client;
using VMDNS.Common;

namespace VMDNS
{
    public class VMDNSForwardZonesNode : VMDNSZonesBaseNode
    {
        public VMDNSForwardZonesNode(VMDNSServerNode node)
            : base(node)
        {
            this.DisplayName = VMDNSConstants.FORWARDZONE;
            this.ZoneType = (int)VmDnsZoneType.FORWARD;
            RefreshChildren();
        }

        protected  override void RefreshChildren()
        {
            this.Children.Clear();
            ServerNode.FillForwardZones();
            ServerNode.ForwardZones.ForEach(x => this.Children.Add(
                    new VMDNSZoneEntryNode(x, this.ServerNode, this)));
        }

        public void AddForwardZone(object sender, EventArgs args)
        {
            AddNewZone((int)VmDnsZoneType.FORWARD);
        }
    }
}


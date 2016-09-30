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

using VMDNS.Nodes;
using VMDNS.Common;
using VMDNS.Client;

namespace VMDNS
{
    public class VMDNSZonesNode:VMDNSZonesBaseNode
    {
        public VMDNSZonesNode(VMDNSServerNode node)
            : base(node)
        {
            this.DisplayName = VMDNSConstants.ZONES;
            this.ZoneType = -1;
            this.FillChildZones();
        }

        private void FillChildZones()
        {
            this.Children.Clear();
            this.Children.Add(new VMDNSForwardZonesNode(this.ServerNode));
            this.Children.Add(new VMDNSReverseZonesNode(this.ServerNode));
        }

        protected override void RefreshChildren()
        {
            FillChildZones();
        }
    }
}


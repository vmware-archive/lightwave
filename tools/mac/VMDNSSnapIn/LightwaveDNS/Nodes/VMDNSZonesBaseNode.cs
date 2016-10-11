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
using VMDNS.Nodes;
using VmIdentity.UI.Common;
using AppKit;
using VmIdentity.UI.Common.Utilities;
using Foundation;

namespace VMDNS
{
    public abstract class VMDNSZonesBaseNode:VMDNSRootScopeNode
    {
        public int ZoneType{ get; set; }

        protected VMDNSZonesBaseNode(VMDNSServerNode node)
            : base(node)
        {
            this.DisplayName = string.Empty;
        }

        protected abstract void RefreshChildren();

        protected void AddNewZone(int zoneType)
        {
            AddNewZoneController awc = new AddNewZoneController(zoneType);
            nint ret = NSApplication.SharedApplication.RunModalForWindow(awc.Window);
            if (ret == VMIdentityConstants.DIALOGOK)
            {
                UIErrorHelper.CheckedExec(delegate()
                    {
                        ServerNode.ServerDTO.DNSClient.CreateZone(awc.ZoneInfo);
                        RefreshChildren();
                        NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadOutlineView", this);
                    });
               
            }
        }
    }
}


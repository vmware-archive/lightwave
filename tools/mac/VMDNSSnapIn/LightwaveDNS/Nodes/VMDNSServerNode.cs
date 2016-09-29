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
using VMDNS.Common;
using VmIdentity.UI.Common;
using System.Collections.Generic;
using VMDNS.Client;
using AppKit;
using System.Linq;
using Foundation;
using VmIdentity.UI.Common.Utilities;

namespace VMDNS
{
    public class VMDNSServerNode:ScopeNodeBase
    {

        public bool IsLoggedIn { get; set; }

        public List<VmDnsZone> ForwardZones{ get; set; }

        public List<VmDnsZone> ReverseZones{ get; set; }

        public List<string> Forwarders { get; set; }

        public VMDNSServerDTO ServerDTO{ get; set; }

        public VMDNSServerNode(VMDNSServerDTO serverDTO)
        {
            this.ServerDTO = serverDTO;
            IsLoggedIn = false;
        }

        public void Login()
        {
            IsLoggedIn = true;
            ServerDTO.LoginToServer();
            return;
        }

        public void FillZonesInfo()
        {
            FillForwardZones();
            FillReverseZones();
            FillForwarders();
        }

        //Fill any server information in cache here to be retrieved by UI
        public void FillForwardZones()
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    ForwardZones = ServerDTO.DNSClient.ListZones(VmDnsZoneType.FORWARD).ToList();
                });
        }

        public void FillReverseZones()
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    ReverseZones = ServerDTO.DNSClient.ListZones(VmDnsZoneType.REVERSE).ToList();
                });
        }

        public void FillForwarders()
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    Forwarders = ServerDTO.DNSClient.GetForwarders().ToList();
                });
        }

        public void ViewServerConfiguration(object sender, EventArgs args)
        {
            var swc = new ServerOptionsController(this);
            NSApplication.SharedApplication.RunModalForWindow(swc.Window);
        }

        public void Refresh(object sender, EventArgs args)
        {
            FillZonesInfo();
            NSNotificationCenter.DefaultCenter.PostNotificationName("RefreshServerData", this);
            NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", this);
        }
    }
}


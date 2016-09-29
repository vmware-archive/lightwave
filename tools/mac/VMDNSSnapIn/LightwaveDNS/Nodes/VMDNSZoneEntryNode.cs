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
using VMDNS.Client;
using AppKit;
using VmIdentity.UI.Common.Utilities;
using Foundation;
using VMDNS.Common;

namespace VMDNS
{
    public class VMDNSZoneEntryNode:VMDNSRootScopeNode
    {
        public VmDnsZone CurrentZone { get; set; }

        public VMDNSZoneEntryNode(VmDnsZone zone, VMDNSServerNode node, VMDNSRootScopeNode parent)
            : base(node)
        {
            this.DisplayName = zone.Name;
            this.CurrentZone = zone;
            this.Parent = parent;
        }

        public void OnClickZoneProperties(object sender, EventArgs args)
        {
            ShowProperties();
        }

        public void ShowProperties()
        {
            ZonePropertiesController zwc = new ZonePropertiesController(this);
            nint ret = NSApplication.SharedApplication.RunModalForWindow(zwc.Window);
            if (ret == (nint)VMIdentityConstants.DIALOGOK) //return 1 for update zone
            {
                UpdateZoneProperties();
            }
        }

        private void UpdateZoneProperties()
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    ServerNode.ServerDTO.DNSClient.UpdateZone(CurrentZone.ZoneInfo);
                    UIErrorHelper.ShowAlert("", VMDNSConstants.SUCCESS_UPDATE_ZONE);
                });
        }

        public void ShowRecordProperties(object sender, EventArgs args, VmDnsRecord record)
        {
            AddNewRecordController awc = new AddNewRecordController(record);
            NSApplication.SharedApplication.RunModalForWindow(awc.Window);
        }

        public void DeleteZone(object sender, EventArgs args)
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    if (UIErrorHelper.ConfirmDeleteOperation(VMIdentityConstants.CONFIRM_DELETE))
                    {
                        this.ServerNode.ServerDTO.DNSClient.DeleteZone(this.DisplayName);
                        this.Children.Clear();
                        ScopeNodeBase node = this.Parent;
                        if (node != null)
                        {
                            node.Children.Remove(this);
                            UIErrorHelper.ShowAlert("", VMDNSConstants.SUCCESS_DELETE_ZONE);
                            NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadOutlineView", node);
                            NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", node);
                        }
                    }
                });
        }

        public void AddRecord(object sender, EventArgs args)
        {
            ChooseRecordTypeController cwc = new ChooseRecordTypeController(CurrentZone.Name);
            nint ret = NSApplication.SharedApplication.RunModalForWindow(cwc.Window);
            if (ret == (nint)VMIdentityConstants.DIALOGOK)
            {
                RecordType recordType = VMDNSUtilityService.GetRecordType(cwc.RecordType);
                AddNewRecordController awc = new AddNewRecordController(recordType);
                nint retVal = NSApplication.SharedApplication.RunModalForWindow(awc.Window);
                if (retVal == VMIdentityConstants.DIALOGOK)
                    AddRecordToZone(awc.Record);
            }

        }

        void AddRecordToZone(VmDnsRecord record)
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    CurrentZone.AddRecord(record);
                    UIErrorHelper.ShowAlert("", VMDNSConstants.SUCCESS_ADD_RECORD);
                    NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", this);
                });
        }

        public void DeleteRecord(object sender, EventArgs args, VmDnsRecord record)
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    if (UIErrorHelper.ConfirmDeleteOperation(VMIdentityConstants.CONFIRM_DELETE))
                    {
                        CurrentZone.DeleteRecord(record);
                        UIErrorHelper.ShowAlert("", VMDNSConstants.SUCCESS_DELETE_RECORD);
                        NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", this);
                    }
                });
        }

    }
}


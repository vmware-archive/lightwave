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

using Microsoft.ManagementConsole;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VMDNS.Client;
using VMDNS.Common;
using VMDNSSnapIn.ListViews;
using VMDNSSnapIn.UI;
using VMDNSSnapIn.UI.RecordViews;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDNSSnapIn.Nodes
{
    class VMDNSZoneEntryNode : VMDNSRootNode
    {
        public VmDnsZone CurrentZone { get; set; }
        public DNSRecordListView ListView { get; set; }

        public VMDNSZoneEntryNode(VmDnsZone zone, VMDNSServerNode node, VMDNSRootNode parent)
            : base(node)
        {
            DisplayName = zone.Name;
            CurrentZone = zone;
            InitConsole();
        }
           

        private void InitConsole()
        {
            this.EnabledStandardVerbs = StandardVerbs.Delete;
            this.ImageIndex = this.SelectedImageIndex = (int)VMDNSTreeImageIndex.Zone;

            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(VMDNSConstants.ZONE_PROPERTIES,
                VMDNSConstants.ZONE_PROPERTIES, -1, ACTION_PROPERTIES));
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(VMDNSConstants.RECORD_ADD,
                VMDNSConstants.RECORD_ADD, -1, ACTION_RECORDADD));
            var lvd = new MmcListViewDescription();
            lvd.DisplayName = VMDNSConstants.RECORD_PROPERTIES;
            lvd.ViewType = typeof(DNSRecordListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes | MmcListViewOptions.SingleSelect;

            AddViewDescription(this, lvd);
        }

        void AddViewDescription(ScopeNode node, MmcListViewDescription lvd)
        {
            node.ViewDescriptions.Add(lvd);
            node.ViewDescriptions.DefaultIndex = 0;
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_PROPERTIES:
                    ShowProperties();
                    break;
                case ACTION_RECORDADD:
                    AddRecord();
                    break;
            }
        }

        protected override void OnExpand(AsyncStatus status)
        {
            //override and do nothing
        }

        public void ShowProperties()
        {
            var frm = new ZoneProperties(this);
            if (SnapIn.Console.ShowDialog(frm) == DialogResult.OK)
            {
                UpdateZoneProperties();
            }
        }

        private void UpdateZoneProperties()
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                ServerNode.ServerDTO.DNSClient.UpdateZone(CurrentZone.ZoneInfo);
            });
        }

        protected override void OnDelete(SyncStatus status)
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                if (!UIErrorHelper.Confirm(MMCUIConstants.CONFIRM_DELETE))
                    return;
                    this.ServerNode.ServerDTO.DNSClient.DeleteZone(this.DisplayName);
                    Parent.Children.Remove(this);
                    base.OnDelete(status);
            });
        }

        public void AddRecord()
        {
            var frm = new ChooseRecordType(CurrentZone.Name);
            if (SnapIn.Console.ShowDialog(frm) == DialogResult.OK)
            {
                RecordType recordType = VMDNSUtilityService.GetRecordType(frm.RecordType);
                var frmAdd = new AddNewRecord(recordType);
                if (SnapIn.Console.ShowDialog(frmAdd) == DialogResult.OK)
                {
                    AddRecordToZone(frmAdd.Record);
                }
            }
        }

        void AddRecordToZone(VmDnsRecord record)
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    CurrentZone.AddRecord(record);
                    this.ListView.Refresh();
                });
        }

        private void RefreshListView()
        {
            if (this.ListView != null)
                this.ListView.Refresh();
        }


        public void ShowRecordProperties(VmDnsRecord record)
        {
            var frmAdd = new AddNewRecord(record);
            SnapIn.Console.ShowDialog(frmAdd);
        }

        public void DeleteRecord(VmDnsRecord record)
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                    CurrentZone.DeleteRecord(record);
                    UIErrorHelper.ShowMessage(VMDNSConstants.SUCCESS_DELETE_RECORD);
            });
        }

    }
}

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
using VMDNS.Client;
using VMDNS.Common;
using VMDNSSnapIn.Nodes;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDNSSnapIn.ListViews
{
    class DNSRecordListView : MmcListView
    {
        const int ACTION_SHOW_PROPERTIES = 1;
        public IList<VmDnsRecord> Entries { get; set; }

        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);

            this.Columns[0].Title = VMDNSConstants.RECORD_NAME;
            this.Columns[0].SetWidth(130);

            this.Columns.Add(new MmcListViewColumn(VMDNSConstants.RECORD_TYPE, 130));

            this.Mode = MmcListViewMode.Report;

            this.SelectionData.EnabledStandardVerbs = StandardVerbs.Delete | StandardVerbs.Refresh;

            (this.ScopeNode as VMDNSZoneEntryNode).ListView = this;
            UIErrorHelper.CheckedExec(delegate()
            {
                Refresh();
            });
        }

        public void Refresh()
        {
            ResultNodes.Clear();
            int nodeCounter = 0;
            this.Entries = (this.ScopeNode as VMDNSZoneEntryNode).CurrentZone.ListRecords();
            foreach (var record in this.Entries)
            {
                var resultNode = new ResultNode { DisplayName = record.Name };
                resultNode.ImageIndex = (int)VMDNSTreeImageIndex.Record;
                resultNode.Tag = nodeCounter++;
                resultNode.SubItemDisplayNames.Add(VMDNSUtilityService.GetRecordNameFromType((RecordType)record.Type));

                this.ResultNodes.Add(resultNode);
            }
            this.Sort(0);
            this.DescriptionBarText = this.ResultNodes.Count.ToString();
        }

        protected override void OnDelete(SyncStatus status)
        {
            if (!UIErrorHelper.Confirm(MMCUIConstants.CONFIRM_DELETE))
                return;

            base.OnDelete(status);
            var zoneNode = this.ScopeNode as VMDNSZoneEntryNode;
            if (zoneNode != null)
            {
                int entriesIndex = (int)this.SelectedNodes[0].Tag;
                zoneNode.DeleteRecord(this.Entries[entriesIndex]);
                var node = this.SelectedNodes[0] as ResultNode;
                this.ResultNodes.Remove(node);
                this.Refresh();
            }
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);

            Refresh();
        }

        protected override void OnSelectionAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            switch ((int)action.Tag)
            {
                case ACTION_SHOW_PROPERTIES:
                    ShowProperties();
                    break;
            }
        }

        protected override void OnSelectionChanged(SyncStatus status)
        {
            if (this.SelectedNodes.Count == 0)
            {
                this.SelectionData.Clear();
            }
            else
            {
                this.SelectionData.Update(GetDetails(), this.SelectedNodes.Count > 1, null, null);
                this.SelectionData.ActionsPaneItems.Clear();
                this.SelectionData.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(VMDNSConstants.RECORD_PROPERTIES,
                                                      VMDNSConstants.RECORD_PROPERTIES, -1, ACTION_SHOW_PROPERTIES));
            }
        }

        private string GetDetails()
        {
            StringBuilder nodedetails = new StringBuilder();

            foreach (ResultNode resultNode in this.SelectedNodes)
            {
                nodedetails.Append(resultNode.DisplayName + ":   " + resultNode.SubItemDisplayNames[0].ToString() + "\n");
            }
            return nodedetails.ToString();
        }

        void ShowProperties()
        {
            if (this.SelectedNodes.Count == 0)
                 return;
            var zoneNode = this.ScopeNode as VMDNSZoneEntryNode;
            zoneNode.ShowRecordProperties(this.Entries[(int)this.SelectedNodes[0].Tag]);
        }
    }
}

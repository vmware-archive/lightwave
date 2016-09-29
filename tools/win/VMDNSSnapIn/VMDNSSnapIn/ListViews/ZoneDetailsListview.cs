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
using VMDNS.Common;
using VMDNSSnapIn.Nodes;

namespace VMDNSSnapIn.ListViews
{
    class ZoneDetailsListview : MmcListView
    {
        const int ACTION_SHOW_PROPERTIES = 3;
        public IList<VMDNSZoneEntryNode> Entries { get; set; }

        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);

            this.Columns[0].Title = VMDNSConstants.ZONE_NAME;
            this.Columns[0].SetWidth(130);

            this.Columns.Add(new MmcListViewColumn(VMDNSConstants.DNS_NAME, 130));
            this.Columns.Add(new MmcListViewColumn(VMDNSConstants.ADMIN_EMAIL, 130));
            this.Mode = MmcListViewMode.Report;

            (this.ScopeNode as VMDNSZonesBaseNode).ListView = this;

            UIErrorHelper.CheckedExec(delegate()
            {
                Refresh();
            });
        }

        public void Refresh()
        {
            ResultNodes.Clear();
            int nodeCounter = 0;
            this.Entries = this.ScopeNode.Children.OfType<VMDNSZoneEntryNode>().ToList();
            foreach (var item in this.Entries)
            {
                var resultNode = new ResultNode { DisplayName = item.CurrentZone.Name };
                resultNode.ImageIndex = (int)VMDNSTreeImageIndex.Zone;
                resultNode.Tag = nodeCounter++;
                resultNode.SubItemDisplayNames.Add(item.CurrentZone.DNSName);
                resultNode.SubItemDisplayNames.Add(item.CurrentZone.AdminEmail);

                this.ResultNodes.Add(resultNode);
            }
            this.Sort(0);
            this.DescriptionBarText = this.ResultNodes.Count.ToString();
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
                this.SelectionData.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(VMDNSConstants.ZONE_PROPERTIES,
                                                      VMDNSConstants.ZONE_PROPERTIES, -1, ACTION_SHOW_PROPERTIES));
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
            int selectedRow = (int)this.SelectedNodes[0].Tag;
            var zoneNode = this.Entries[selectedRow];
            zoneNode.ShowProperties();
        }

      
    }
}

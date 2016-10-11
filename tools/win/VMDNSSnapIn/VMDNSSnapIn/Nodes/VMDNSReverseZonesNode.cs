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

namespace VMDNSSnapIn.Nodes
{
    class VMDNSReverseZonesNode : VMDNSZonesBaseNode
    {
        public VMDNSReverseZonesNode(VMDNSServerNode node)
            : base(node)
        {
            DisplayName = VMDNSConstants.REVERSEZONE;
            ZoneType = (int)VmDnsZoneType.REVERSE;
            InitConsole();
            RefreshChildren();
        }

        private void InitConsole()
        {
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(VMDNSConstants.ZONE_ADD_REVERSE, VMDNSConstants.ZONE_ADD_REVERSE, -1, ACTION_ADD_REVERSEZONE));
            this.ImageIndex = this.SelectedImageIndex = (int)VMDNSTreeImageIndex.ForwardReverseZones;
            var lvd = new MmcListViewDescription();
            lvd.DisplayName = VMDNSConstants.ZONES;
            lvd.ViewType = typeof(ZoneDetailsListview);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes | MmcListViewOptions.SingleSelect;
            AddViewDescription(this, lvd);
        }

        private void AddViewDescription(ScopeNode node, MmcListViewDescription lvd)
        {
            node.ViewDescriptions.Add(lvd);
            node.ViewDescriptions.DefaultIndex = 0;
        }


        protected override void RefreshChildren()
        {
            this.Children.Clear();
            ServerNode.FillReverseZones();
            ServerNode.ReverseZones.ForEach(x => this.Children.Add(
                    new VMDNSZoneEntryNode(x, this.ServerNode, this)));
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_ADD_REVERSEZONE:
                    AddReverseZone();
                    break;
            }
        }

        public void AddReverseZone()
        {
            var frm = new AddNewReverseZone();
            if (SnapIn.Console.ShowDialog(frm) == DialogResult.OK)
            {
                AddNewZone(frm.ZoneInfo);
                this.ListView.Refresh();
            }
        }

    }
}

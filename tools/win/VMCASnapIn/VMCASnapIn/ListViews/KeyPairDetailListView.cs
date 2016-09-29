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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.ManagementConsole;
using VMCASnapIn.Nodes;
using VMCASnapIn.Utilities;
using VMCASnapIn.DTO;
using VMCASnapIn.UI;
using VMCA.Client;
using VMIdentity.CommonUtils;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCASnapIn.ListViews
{
    public class KeyPairDetailListView : MmcListView
    {
        const int ACTION_EXPORT_KEYPAIR = 1;
        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);

            this.Columns[0].Title = "Created Date";
            this.Columns[0].SetWidth(200);

            this.Columns.Add(new MmcListViewColumn("Key Length", 200));

            this.Mode = MmcListViewMode.Report;

            (this.ScopeNode as ChildScopeNode).ListView = this;

            RefreshList();
        }

        protected override void OnDelete(SyncStatus status)
        {
            if (!MMCDlgHelper.ShowConfirm(CommonConstants.GetSelectedDeleteMsg("key pairs", this.SelectedNodes.Count)))
                return;

            base.OnDelete(status);

            var serverDTO = (this.ScopeNode as ChildScopeNode).ServerDTO;
            foreach (ResultNode node in this.SelectedNodes)
            {
                serverDTO.KeyPairs.RemoveAt(this.ResultNodes.IndexOf(node));
                this.ResultNodes.Remove(node);
            }
            RefreshList();
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);

            RefreshList();
        }

        public void RefreshList()
        {
            this.ResultNodes.Clear();

            var serverDTO = (this.ScopeNode as ChildScopeNode).ServerDTO;
            if (serverDTO.KeyPairs != null)
            {
                foreach (var keypair in serverDTO.KeyPairs)
                {
                    if (keypair != null)
                    {
                        var resultNode = new ResultNode { DisplayName = keypair.CreatedDateTime.ToString() };
                        resultNode.SubItemDisplayNames.Add(keypair.KeyLength.ToString());
                        resultNode.Tag = keypair;
                        resultNode.ImageIndex = (int)VMCAImageIndex.KeyPairs;
                        this.ResultNodes.Add(resultNode);
                    }
                }
            }
            this.DescriptionBarText = this.ResultNodes.Count.ToString();
        }

        protected override void OnSelectionChanged(SyncStatus status)
        {
            base.OnSelectionChanged(status);

            if (this.SelectedNodes.Count == 0)
            {
                this.SelectionData.Clear();
            }
            else
            {
                this.SelectionData.Update(GetDetails(), this.SelectedNodes.Count > 1, null, null);
                this.SelectionData.ActionsPaneItems.Clear();
                this.SelectionData.EnabledStandardVerbs = StandardVerbs.Delete;
                if(this.SelectedNodes.Count==1)
                    this.SelectionData.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Export key pair",
                                                       "Export key pair to file", -1, ACTION_EXPORT_KEYPAIR));
            }
        }

        protected override void OnSelectionAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnSelectionAction(action, status);
            switch ((int)action.Tag)
            {
                case ACTION_EXPORT_KEYPAIR:
                    ExportKeyPair();
                    break;
            }
        }

        void ExportKeyPair()
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                var keypair = this.SelectedNodes[0].Tag as KeyPairDTO;
                var keypairData = new KeyPairData(keypair.PublicKey, keypair.PrivateKey);
                Helper.SaveKeyData(keypairData);
            });
        }


        private string GetDetails()
        {
            StringBuilder nodedetails = new StringBuilder();

            foreach (ResultNode resultNode in this.SelectedNodes)
            {
                nodedetails.AppendLine(resultNode.DisplayName);
            }
            return nodedetails.ToString();
        }

    }
}

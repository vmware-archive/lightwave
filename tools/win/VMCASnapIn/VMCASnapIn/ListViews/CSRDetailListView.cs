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
using VMCASnapIn.DTO;
using VMCASnapIn.Utilities;
using VMCA.Utilities;
using VMIdentity.CommonUtils;
using VMwareMMCIDP.UI.Common;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCASnapIn.ListViews
{
    public class CSRDetailListView : MmcListView
    {
        const int ACTION_SHOW_REQUEST = 1;
        const int ACTION_EXPORT_REQUEST = 2;
        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);

            this.Columns[0].Title = "Created Date";
            this.Columns[0].SetWidth(200);

            this.Columns.Add(new MmcListViewColumn("Data", 200));

            this.Mode = MmcListViewMode.Report;
            (this.ScopeNode as ChildScopeNode).ListView = this;

            RefreshList();
        }

        protected override void OnDelete(SyncStatus status)
        {
            if (!MMCDlgHelper.ShowConfirm(CommonConstants.GetSelectedDeleteMsg("signing requests", this.SelectedNodes.Count)))
                return;

            base.OnDelete(status);

            var serverDTO = (this.ScopeNode as ChildScopeNode).ServerDTO;
            foreach (ResultNode node in this.SelectedNodes)
            {
                serverDTO.SigningRequests.RemoveAt(this.ResultNodes.IndexOf(node));
                this.ResultNodes.Remove(node);
            }
            RefreshList();
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
                this.SelectionData.EnabledStandardVerbs = StandardVerbs.Delete;
                if (this.SelectedNodes.Count == 1)
                {
                    this.SelectionData.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Show Request",
                                                           "Shows certificate signing request", -1, ACTION_SHOW_REQUEST));
                    this.SelectionData.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Export Request",
                                                          "Expot certificate signing request", -1, ACTION_EXPORT_REQUEST));
                }
            }
        }

        protected override void OnSelectionAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            switch ((int)action.Tag)
            {
                case ACTION_SHOW_REQUEST:
                    ShowSigningRequest();
                    break;
                case ACTION_EXPORT_REQUEST:
                    ExportSigningRequest();
                    break;
            }
        }

        void ShowSigningRequest()
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                var cert = this.SelectedNodes[0].Tag as SigningRequestDTO;
                var frm = new InfoForm(cert.CSR);
                frm.Icon = VMCASnapInEnvironment.Instance.GetIconResource(VMCAIconIndex.cert);
                this.SnapIn.Console.ShowDialog(frm);
            });
        }

        void ExportSigningRequest()
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                var cert = this.SelectedNodes[0].Tag as SigningRequestDTO;
                MMCMiscUtil.SaveDataToFile(cert.CSR,"Save CSR",MMCUIConstants.CSR_FILTER);
            });
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

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);

            RefreshList();
        }

        public void RefreshList()
        {
            this.ResultNodes.Clear();

            var serverDTO = (this.ScopeNode as ChildScopeNode).ServerDTO;
            if (serverDTO.SigningRequests != null)
            {
                foreach (var dto in serverDTO.SigningRequests)
                {
                    var resultNode = new ResultNode
                    {
                        DisplayName = dto.CreatedDateTime.ToString()
                    };
                    resultNode.SubItemDisplayNames.Add(dto.CSR);
                    resultNode.Tag = dto;
                    resultNode.ImageIndex = (int)VMCAImageIndex.Certificate;
                    this.ResultNodes.Add(resultNode);
                }
            }
            this.DescriptionBarText = this.ResultNodes.Count.ToString();
        }
    }
}

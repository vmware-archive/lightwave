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
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using VMCertStore.Common.DTO;
using VMCertStoreSnapIn.Nodes;
using VMCertStoreSnapIn.Utilities;
using VMCertStore.Client;
using VMCertStore.Utilities;
using Vecs;
using VMIdentity.CommonUtils;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCertStoreSnapIn.ListViews
{
    public class VecsSecretKeysListView : VecsStoreEntriesListView
    {
        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);

            this.Columns[0].Title = "Alias";
            this.Columns[0].SetWidth(130);

            this.Mode = MmcListViewMode.Report;

            (this.ScopeNode as VecsStoreEntriesNode).ListView = this;

            Refresh();
        }

        protected override void OnSelectionChanged(SyncStatus status)
        {
            base.OnSelectionChanged(status);

            if (this.SelectedNodes.Count == 0)
            {
                this.SelectionData.Clear();
                this.SelectionData.ActionsPaneItems.Clear();
            }
            else
            {
                this.SelectionData.Update((ResultNode)this.SelectedNodes[0],
                                           this.SelectedNodes.Count > 1, null, null);
                this.SelectionData.ActionsPaneItems.Clear();
                this.SelectionData.EnabledStandardVerbs = StandardVerbs.Delete;
            }
        }

        protected override void OnDelete(SyncStatus status)
        {
            if (!MMCDlgHelper.ShowConfirm(CommonConstants.GetSelectedDeleteMsg("secret keys", this.SelectedNodes.Count)))
                return;

            base.OnDelete(status);
            var entriesNode = this.ScopeNode as VecsStoreEntriesNode;
            var dto = entriesNode.ServerDTO;
            var storeName = entriesNode.StoreName;
            var storePass = "";
            MMCActionHelper.CheckedExec(delegate()
            {
                using (var session = new VecsStoreSession(dto.VecsClient, storeName, storePass))
                {
                    foreach (ResultNode node in this.SelectedNodes)
                    {
                        var certDTO = node.Tag as Vecs.CertDTO;
                        session.DeleteCertificate(certDTO.Alias);
                    }
                }
            });
            Refresh();
        }

        public override void Refresh()
        {
            ResultNodes.Clear();

            var entriesNode = this.ScopeNode as VecsStoreEntriesNode;
            var dto = entriesNode.ServerDTO;
            var storeName = entriesNode.StoreName;
            var storePass = "";
            MMCActionHelper.CheckedExec(delegate()
            {
                using (var session = new VecsStoreSession(dto.VecsClient, storeName, storePass))
                {
                    var lst = session.GetSecretKeys();
                    if (lst == null)
                        return;

                    foreach (var certDTO in lst)
                    {
                        var resultNode = new ResultNode { DisplayName = certDTO.Alias };
                        resultNode.ImageIndex = (int)VMCertStoreImageIndex.SecretKeys;
                        resultNode.Tag = certDTO;
                        this.ResultNodes.Add(resultNode);
                    }
                }
            });
            this.Sort(0);
            this.DescriptionBarText = this.ResultNodes.Count.ToString();
        }
    }
}

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
using Vecs;
using VMIdentity.CommonUtils;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCertStoreSnapIn.ListViews
{
    public class VecsTrustedCertsListView : VecsStoreEntriesListView
    {
        const int ACTION_SHOW_CERTIFICATE = 1;
        const int ACTION_REVOKE_CERTIFICATE = 2;
        const int ACTION_SHOW_CERTIFICATE_STRING = 3;

        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);

            this.Columns[0].Title = "Alias";
            this.Columns[0].SetWidth(130);

            this.Columns.Add(new MmcListViewColumn("Subject", 130));
            this.Columns.Add(new MmcListViewColumn("Issued By", 130));
            this.Columns.Add(new MmcListViewColumn("Issued Date", 90));
            this.Columns.Add(new MmcListViewColumn("Expiration Date", 90));
            this.Columns.Add(new MmcListViewColumn("Intended Purposes", 150));

            this.Mode = MmcListViewMode.Report;

            (this.ScopeNode as VecsStoreEntriesNode).ListView = this;

            Refresh();

        }

        protected override void OnDelete(SyncStatus status)
        {
            if (!MMCDlgHelper.ShowConfirm(CommonConstants.GetSelectedDeleteMsg("certificates", this.SelectedNodes.Count)))
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
                    var lst = session.GetCertificates();
                    if (lst == null)
                        return;

                    foreach (var certDTO in lst)
                    {
                        var resultNode = new ResultNode { DisplayName = certDTO.Alias };
                        resultNode.ImageIndex = (int)VMCertStoreImageIndex.TrustedCert;
                        if (certDTO.Cert != null)
                        {
                            resultNode.SubItemDisplayNames.Add(certDTO.Cert.Subject);
                            resultNode.SubItemDisplayNames.Add(certDTO.Cert.Issuer);
                            resultNode.SubItemDisplayNames.Add(certDTO.Cert.NotBefore.ToShortDateString());
                            resultNode.SubItemDisplayNames.Add(certDTO.Cert.NotAfter.ToShortDateString());
                            resultNode.SubItemDisplayNames.Add(certDTO.Cert.GetKeyUsage());
                            resultNode.Tag = certDTO;
                        }

                        this.ResultNodes.Add(resultNode);
                    }
                }
            });
            this.Sort(0);
            this.DescriptionBarText = this.ResultNodes.Count.ToString();
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
                    this.SelectionData.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Show Certificate",
                                                       "Shows certificate details", -1, ACTION_SHOW_CERTIFICATE));
            }
        }

        protected override void OnSelectionAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            switch ((int)action.Tag)
            {
                case ACTION_SHOW_CERTIFICATE:
                    ShowCertificateDetails();
                    break;
            }
        }

        void ShowCertificateDetails()
        {
            MMCActionHelper.CheckedExec(delegate
            {
                var certDTO = this.SelectedNodes[0].Tag as Vecs.CertDTO;
                if(certDTO.Cert!=null)
				{
                    X509Certificate2UI.DisplayCertificate(certDTO.Cert);
				}
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
    }
}

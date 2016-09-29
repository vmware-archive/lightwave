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
using System.Security.Cryptography.X509Certificates;
using System.Text;
using Microsoft.ManagementConsole;
using VMCA;
using VMCA.Client;
using VMCASnapIn.Nodes;
using VMCASnapIn.Utilities;
using VMIdentity.CommonUtils;
using VMwareMMCIDP.UI.Common;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCASnapIn.ListViews
{
    class CertificateDetailsListView : MmcListView
    {
        const int ACTION_SHOW_CERTIFICATE = 1;
        const int ACTION_REVOKE_CERTIFICATE = 2;
        const int ACTION_SHOW_CERTIFICATE_STRING = 3;

        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);

            this.Columns[0].Title = "Issued To";
            this.Columns[0].SetWidth(200);

            this.Columns.Add(new MmcListViewColumn("Issued By", 200));
            this.Columns.Add(new MmcListViewColumn("Issued Date", 200));
            this.Columns.Add(new MmcListViewColumn("Expiration Date", 70));
            this.Columns.Add(new MmcListViewColumn("Intended Purposes", 150));
            this.Columns.Add(new MmcListViewColumn("Status", 100));

            this.Mode = MmcListViewMode.Report;
            this.SelectionData.EnabledStandardVerbs = StandardVerbs.Refresh;

            var node = (this.ScopeNode as ChildScopeNode);
            if(node != null)
                node.ListView = this;

            RefreshList();
        }

        protected override void OnDelete(SyncStatus status)
        {
            if (!MMCDlgHelper.ShowConfirm(CommonConstants.GetSelectedDeleteMsg("certificates", this.SelectedNodes.Count)))
                return;

            base.OnDelete(status);

            var serverDTO = (this.ScopeNode as ChildScopeNode).ServerDTO;
            foreach (ResultNode node in this.SelectedNodes)
            {
                serverDTO.PrivateCertificates.RemoveAt(this.ResultNodes.IndexOf(node));
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
            var state = (CertificateState)this.ScopeNode.Tag;
            FillNodes(state);
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
                var state = (CertificateState)this.ScopeNode.Tag;
                this.SelectionData.Update(GetDetails(), this.SelectedNodes.Count > 1, null, null);
                this.SelectionData.ActionsPaneItems.Clear();
                
                if (this.ScopeNode is VMCAPersonalCertificatesNode)
                    this.SelectionData.EnabledStandardVerbs = StandardVerbs.Delete;

                if (SelectedNodes.Count == 1)
                {
                    this.SelectionData.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Show Certificate",
                                                       "Shows certificate details", -1, ACTION_SHOW_CERTIFICATE));
                    if (state == CertificateState.Active)
                    {
                        this.SelectionData.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Revoke Certificate",
                                                       "Revoke Certificate", -1, ACTION_REVOKE_CERTIFICATE));
                    }
                    this.SelectionData.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Show Certificate String",
                                                       "Show Certificate String", -1, ACTION_SHOW_CERTIFICATE_STRING));
                }
            }
        }

        protected override void OnSelectionAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            switch ((int)action.Tag)
            {
                case ACTION_SHOW_CERTIFICATE:
                    ShowCertificateDetails();
                    break;
                case ACTION_REVOKE_CERTIFICATE:
                    RevokeCertificate();
                    break;
                case ACTION_SHOW_CERTIFICATE_STRING:
                    ShowCertificateString();
                    break;
            }
        }

        void ShowCertificateString()
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                var cert = this.SelectedNodes[0].Tag as X509Certificate2;
                var frm = new InfoForm(VMCACertificate.GetCertificateAsString(cert));
                frm.Icon = VMCASnapInEnvironment.Instance.GetIconResource(VMCAIconIndex.cert);
                MMCDlgHelper.ShowForm(frm);
            });
        }

        void RevokeCertificate()
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                var cert = this.SelectedNodes[0].Tag as X509Certificate2;
                var serverDTO = (ScopeNode.Parent as VMCAServerNode).ServerDTO;
                var vmcaCert = new VMCACertificate(serverDTO.VMCAClient, cert);
                vmcaCert.Revoke();
                ResultNodes.Remove(this.SelectedNodes[0] as ResultNode);
            });
            RefreshList();
            var parent=this.ScopeNode.Parent as VMCAServerNode;
            if (parent != null)
            {
                foreach (var node in parent.Children)
                {
                    var certNode = node as VMCACertsNode;
                    if (certNode != null && (CertificateState)certNode.Tag == CertificateState.Revoked)
                        certNode.DoRefresh();
                }
            }
        }

        void ShowCertificateDetails()
        {
            MMCActionHelper.CheckedExec(delegate
            {
                var cert = this.SelectedNodes[0].Tag as X509Certificate2;
                if (cert != null)
                {
                    X509Certificate2UI.DisplayCertificate(cert);
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

        void FillNodes(CertificateState filter)
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                this.ResultNodes.Clear();

                var serverDTO = (ScopeNode.Parent as ChildScopeNode).ServerDTO;

                if ((int)filter == -1)
                {
                    if (serverDTO.PrivateCertificates != null)
                    {
                        foreach (var dto in serverDTO.PrivateCertificates)
                        {
                            AddCert(dto.Certificate.GetX509Certificate2FromString());
                        }
                    }
                }
                else
                {
                    using (var context = new VMCAEnumContext(serverDTO.VMCAClient, filter))
                    {
                        foreach (var cert in context.GetCertificates())
                        {
                            AddCert(cert);
                        }
                    }
                }
            });
        }

        void AddCert(X509Certificate2 cert)
        {
            var resultNode = new ResultNode { DisplayName = cert.Subject };
            resultNode.ImageIndex = (int)VMCAImageIndex.Certificate;
            resultNode.SubItemDisplayNames.Add(cert.Issuer);
            resultNode.SubItemDisplayNames.Add(cert.NotBefore.ToShortDateString());
            resultNode.SubItemDisplayNames.Add(cert.NotAfter.ToShortDateString());
            resultNode.SubItemDisplayNames.Add(cert.GetKeyUsage());
            resultNode.Tag = cert;

            this.ResultNodes.Add(resultNode);
        }
    }
}

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
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;
using Action = Microsoft.ManagementConsole.Action;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public class CertificateDetailsListView : MmcListView
    {
        const int ActionShowCertificate = 1;
        const int ActionRemoveSslCertificate = 2;
        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);
            Columns.Add(new MmcListViewColumn("Chain", 60));
            Columns.Add(new MmcListViewColumn("Active", 60));
            Columns.Add(new MmcListViewColumn("Issued By", 200));
            Columns.Add(new MmcListViewColumn("Issued Date", 120));
            Columns.Add(new MmcListViewColumn("Expiration Date", 120));
            Columns.Add(new MmcListViewColumn("Intended Purposes", 180));
            Columns.Add(new MmcListViewColumn("Status", 80));
            Mode = MmcListViewMode.Report;
            SelectionData.EnabledStandardVerbs = StandardVerbs.Refresh;
            var node = ScopeNode as TrustedCertificatesNode;
            node.View = this;
            OnRefresh(status);
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);
            Refresh();
        }

        public void Refresh()
        {
            var node = ScopeNode as TrustedCertificatesNode;
            if (node != null)
            {
                var dto = node.Refresh() as List<CertificateDto>;
                if (dto != null)
                    FillNodes(dto);
            }
        }

        protected override void OnSelectionChanged(SyncStatus status)
        {
            if (SelectedNodes.Count == 0)
            {
                SelectionData.Clear();
            }
            else
            {
                SelectionData.Update(GetDetails(), SelectedNodes.Count > 1, null, null);
                SelectionData.ActionsPaneItems.Clear();
                SelectionData.ActionsPaneItems.Add(new Action("Show Certificate", "Shows certificate details", (int)TreeImageIndex.TrustedCertificate, ActionShowCertificate));
                SelectionData.ActionsPaneItems.Add(new Action("Remove Certificate", "Remove Certificate", -1, ActionRemoveSslCertificate));
            }
        }

        protected override void OnSelectionAction(Action action, AsyncStatus status)
        {
            switch ((int)action.Tag)
            {
                case ActionShowCertificate:
                    ShowCertificateDetails();
                    break;
                case ActionRemoveSslCertificate:
                    RemoveCertificate();
                    break;
            }
        }
        void ShowCertificateDetails()
        {
            if (SelectedNodes.Count == 0)
                return;
            var cert = SelectedNodes[0].Tag as X509Certificate2;
            if (cert != null)
            {
                X509Certificate2UI.DisplayCertificate(cert);
            }
        }

        void RemoveCertificate()
        {
            if (SelectedNodes.Count == 0)
                return;
            var node = ScopeNode as TrustedCertificatesNode;
            var serverDto = node.GetServerDto();
            var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
            var tenant = node.GetTenant().DisplayName;
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenant);
            ActionHelper.Execute(delegate
            {
                var cert = SelectedNodes[0].Tag as X509Certificate2;
                if (cert != null)
                {                    
                    service.Certificate.Delete(serverDto, tenant, cert.GetFormattedThumbPrint(), auth.Token);
                    Refresh();
                }
            }, auth);
        }

        private string GetDetails()
        {
            StringBuilder nodedetails = new StringBuilder();

            foreach (ResultNode resultNode in SelectedNodes)
            {
                nodedetails.Append(resultNode.DisplayName + ":   " + resultNode.SubItemDisplayNames[0] + "\n");
            }
            return nodedetails.ToString();
        }

        void FillNodes(IEnumerable<CertificateDto> certs)
        {
            ResultNodes.Clear();

            foreach (var cert in certs)
            {
                var x509Cert = new X509Certificate2(Encoding.ASCII.GetBytes(cert.Encoded));
                var resultNode = new ResultNode { DisplayName = x509Cert.Subject };
                resultNode.SubItemDisplayNames.Add(cert.Chain);
                resultNode.SubItemDisplayNames.Add(cert.IsSigner.ToString());
                resultNode.SubItemDisplayNames.Add(x509Cert.Issuer);
                resultNode.SubItemDisplayNames.Add(x509Cert.NotBefore.ToShortDateString());
                resultNode.SubItemDisplayNames.Add(x509Cert.NotAfter.ToShortDateString());
                resultNode.SubItemDisplayNames.Add(x509Cert.GetKeyUsage());
                resultNode.Tag = x509Cert;
                ResultNodes.Add(resultNode);
            }
        }
    }
}

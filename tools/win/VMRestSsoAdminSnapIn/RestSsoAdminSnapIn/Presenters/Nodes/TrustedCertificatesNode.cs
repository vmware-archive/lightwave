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
using System.Linq;
using System.Collections.Generic;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Views;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using Action = Microsoft.ManagementConsole.Action;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes
{
    public class TrustedCertificatesNode : ScopeNode
    {
        const int ActionAdd = 1;
        public CertificateDetailsListView View;
        public TrustedCertificatesNode()
            : base(true)
        {
            InitConsole();
            AddActions();
            ImageIndex = SelectedImageIndex = (int)TreeImageIndex.TrustedCertificate;
        }

        private void AddActions()
        {
            EnabledStandardVerbs = StandardVerbs.Refresh;
            var action = new Action("Add", "Add", (int)TreeImageIndex.TrustedCertificate, ActionAdd);
            ActionsPaneItems.Add(action);
        }

        public TenantNode GetTenant()
        {
            return Parent.Parent as TenantNode;
        }
        public ServerDto GetServerDto()
        {
            var dto = Parent.Parent.Parent.Tag as AuthTokenDto;
            return dto != null ? dto.ServerDto : null;
        }

        protected override void OnAction(Action action, AsyncStatus status)
        {
            base.OnAction(action, status);
            switch ((int)action.Tag)
            {
                case ActionAdd:
                    AddCertificate();
                    break;
            }
        }

        void AddCertificate()
        {
            var serverDto = GetServerDto();
            ActionHelper.Execute(delegate()
            {
                var tenantName = GetTenant().DisplayName;
                var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                var frm = new NewSignerIdentity(service, serverDto, tenantName);
                if (this.SnapIn.Console.ShowDialog(frm) == DialogResult.OK)
                {
                    View.Refresh();
                }
            }, null);
        }

        void InitConsole()
        {
            DisplayName = "Trusted Certificates";
            AddViewDescription();
        }

        void AddViewDescription()
        {
            var lvd = new MmcListViewDescription
            {
                DisplayName = "Certificate Details",
                ViewType = typeof(CertificateDetailsListView),
                Options = MmcListViewOptions.ExcludeScopeNodes
            };

            ViewDescriptions.Add(lvd);
            ViewDescriptions.DefaultIndex = 0;
        }

        public List<CertificateDto> Refresh()
        {
            var result = new List<CertificateDto>();
            var serverDto = GetServerDto();
            var tenantName = GetTenant().DisplayName;
            var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenantName);

            ActionHelper.Execute(delegate
            {
                if (auth.Token != null)
                {
                    var chains = service.Certificate.GetCertificates(serverDto, tenantName, CertificateScope.TENANT, auth.Token);
                    var chainId = 1;
                    foreach (var chain in chains)
                    {

                        result.AddRange(chain.Certificates.Select(x => new CertificateDto { Encoded = x.Encoded, Chain = "Chain-" + chainId, IsSigner = (chainId == chains.Count) }));
                        chainId++;
                    }
                }
            }, auth);

            return result;
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);
            View.Refresh();
        }
    }
}

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
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service.IdentityProvider;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Tenant;
using Vmware.Tools.RestSsoAdminSnapIn.Views;
using Action = Microsoft.ManagementConsole.Action;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes
{
    public class OidcClientsNode : ScopeNode
    {
        enum OidcClientsNodeAction
        {
            ActionAddOidcClient
        }
        public OidcClientsControl Control { get; set; }

        public OidcClientsNode()
            : base(true)
        {
            AddActions();
            ImageIndex = SelectedImageIndex = (int)TreeImageIndex.IdentityProvider;
            DisplayName = "OIDC Clients";
            AddViewDescription();
        }
        void AddViewDescription()
        {
            var fvd = new FormViewDescription
            {
                DisplayName = "OIDC Clients",
                ViewType = typeof(OidcClientsFormView),
                ControlType = typeof(OidcClientsControl)
            };
            ViewDescriptions.Add(fvd);
            ViewDescriptions.DefaultIndex = 0;
        }
        private void AddActions()
        {
            EnabledStandardVerbs = StandardVerbs.Refresh;
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Add OIDC Client", "Add OIDC Client", (int)TreeImageIndex.IdentityProvider, OidcClientsNodeAction.ActionAddOidcClient));
        }
        public TenantNode GetTenant()
        {
            return Parent as TenantNode;
        }
        public ServerDto GetServerDto()
        {
            var dto = Parent.Parent.Tag as AuthTokenDto;
            return dto != null ? dto.ServerDto : null;
        }
        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);
            DoRefresh();
        }
        protected override void OnExpand(AsyncStatus status)
        {
            base.OnExpand(status);
        }
        protected override void OnAction(Action action, AsyncStatus status)
        {
            base.OnAction(action, status);
            switch ((OidcClientsNodeAction)(int)action.Tag)
            {
                case OidcClientsNodeAction.ActionAddOidcClient:
                    ShowAddOidcClient();
                    break;
            }
        }
        public void DoRefresh()
        {
            Control.RefreshOidcClients();
        }
        void ShowAddOidcClient()
        {
            var serverDto = GetServerDto();
            ActionHelper.Execute(delegate()
            {
                var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                var tenantName = GetTenant().DisplayName;
                var form = new NewOidcClient(service, serverDto, tenantName);
                if (SnapIn.Console.ShowDialog(form) == DialogResult.OK)
                {
                    this.Control.RefreshOidcClients();
                }
            }, null);
        }
    }
}

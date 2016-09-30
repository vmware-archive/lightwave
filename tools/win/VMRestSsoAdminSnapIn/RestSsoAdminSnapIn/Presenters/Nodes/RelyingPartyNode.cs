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
    public class RelyingPartyNode : ScopeNode
    {
        enum RelyingPartyNodeAction
        {
            ActionAddRelyingParty
        }
        public RelyingPartyControl RelyingPartyControl { get; set; }

        public RelyingPartyNode()
            : base(true)
        {
            AddActions();
            ImageIndex = SelectedImageIndex = (int)TreeImageIndex.Server;
            DisplayName = "Relying Party";
            AddViewDescription();
        }
        void AddViewDescription()
        {
            var fvd = new FormViewDescription
            {
                DisplayName = "Relying Party",
                ViewType = typeof(RelyingPartyFormView),
                ControlType = typeof(RelyingPartyControl)
            };
            ViewDescriptions.Add(fvd);
            ViewDescriptions.DefaultIndex = 0;
        }
        private void AddActions()
        {
            EnabledStandardVerbs = StandardVerbs.Refresh;
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Add Relying Party", "Add Relying Party", (int)TreeImageIndex.Server, RelyingPartyNodeAction.ActionAddRelyingParty));
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
            switch ((RelyingPartyNodeAction)(int)action.Tag)
            {
                case RelyingPartyNodeAction.ActionAddRelyingParty:
                    ShowAddRelyingParty();
                    break;
            }
        }
        public void DoRefresh()
        {
            RelyingPartyControl.RefreshRelyingParty();
        }
        void ShowAddRelyingParty()
        {
            var serverDto = GetServerDto();
            ActionHelper.Execute(delegate()
            {
                var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                var tenantName = GetTenant().DisplayName;
                var form = new NewRelyingParty(service, serverDto, tenantName);
                if (SnapIn.Console.ShowDialog(form) == DialogResult.OK)
                {
                    this.RelyingPartyControl.RefreshRelyingParty();
                }
            }, null);
        }
    }
}

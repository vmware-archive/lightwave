/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Views;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes
{
    public class TenantNode : ScopeNode
    {
        public enum TenantNodeAction
        {
            ActionConfiguration = 1,
            ActionSuperLogging = 2
        }
        public TenantNode()
            : base(false)
        {
            EnabledStandardVerbs = StandardVerbs.Refresh;
            EnabledStandardVerbs = StandardVerbs.Delete;
            ImageIndex = SelectedImageIndex = (int)TreeImageIndex.Tenant;
            ActionsPaneItems.Add(new Action("Configuration", "Configuration", (int)TreeImageIndex.Settings, TenantNodeAction.ActionConfiguration));
            ActionsPaneItems.Add(new Action("Super Log", "Super Log", (int)TreeImageIndex.Settings, TenantNodeAction.ActionSuperLogging));
            Children.Add(new IdentityProvidersNode());
            Children.Add(new ExternalIdentityProvidersNode());
            Children.Add(new RelyingPartyNode());
            Children.Add(new OidcClientsNode());
            Children.Add(new ServerCertificatesNode());
        }
        protected override void OnAction(Action action, AsyncStatus status)
        {
            base.OnAction(action, status);
            switch ((TenantNodeAction)(int)action.Tag)
            {
                case TenantNodeAction.ActionConfiguration:
                    ShowTenantConfiguration();
                    break;
                case TenantNodeAction.ActionSuperLogging:
                    ShowSuperLogging();
                    break;
            }
        }
        protected override void OnDelete(SyncStatus status)
        {
            var deleted = false;
            var serverDto = GetServerDto();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, DisplayName);
            ActionHelper.Execute(delegate
            {
                if (ErrorMessageDisplayHelper.Confirm("Delete Tenant " + DisplayName + " permenantly?"))
                {
                    var service = this.GetServiceGateway();
                    service.Tenant.Delete(auth.ServerDto, DisplayName, auth.Token);
                    deleted = true;

                    var parent = ((ServerNode)Parent);
                    for (var i = 0; i < parent.Children.Count; i++)
                    {
                        if (parent.Children[i].DisplayName == DisplayName)
                        {
                            parent.Children.RemoveAt(i);
                            break;
                        }
                    }
                }
            }, auth);
            if (deleted)
                base.OnDelete(status);
        }

        private ServerDto GetServerDto()
        {
            var dto = Parent.Tag as AuthTokenDto;
            return dto != null ? dto.ServerDto : null;
        }

        private void ShowTenantConfiguration()
        {
            var serverDto = GetServerDto();
            ActionHelper.Execute(delegate
            {
                var tenantName = DisplayName;
                var frm = new TenantConfiguration(serverDto, tenantName);
                if (this.SnapIn.Console.ShowDialog(frm) == DialogResult.OK)
                {
                    // do nothing
                }
            }, null);
        }

        private void ShowSuperLogging()
        {
            var serverDto = GetServerDto();
            ActionHelper.Execute(delegate
            {
                var tenantName = DisplayName;
                var title = string.Format("Super Logging ({0})", tenantName);
                var frm = new SuperLogging(serverDto, tenantName) { Text = title };
                this.SnapIn.Console.ShowDialog(frm);
            }, null);
        }
    }
}

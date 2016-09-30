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
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.ExternalDomain;
using Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages;
using Vmware.Tools.RestSsoAdminSnapIn.Views.Wizard;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes
{
    public class ExternalDomainNode : ScopeNode
    {
        private IdentityProviderDto _provider;
        private string _tenantName;
        private enum ExternalDomainNodeAction
        {
            ActionProbe = 1,
            ActionProperties = 2
        }
        public ExternalDomainNode(string tenantName, IdentityProviderDto provider)
            : base(false)
        {
            _tenantName = tenantName;
            _provider = provider;
            this.DisplayName = _provider.Name;
            this.EnabledStandardVerbs = StandardVerbs.Delete;
            var action1 = new Action("Properties", "Properties", 1, ExternalDomainNodeAction.ActionProperties);
            ActionsPaneItems.Add(action1);
            ImageIndex = SelectedImageIndex = (int)TreeImageIndex.ExternalDomain;
        }
        protected override void OnExpand(AsyncStatus status)
        {
            base.OnExpand(status);
            var serverDto = GetServerDTO();
            var userAndgroupNode = new UsersAndGroupsNode(serverDto, _tenantName, _provider, false);
            this.Children.Add(userAndgroupNode);
        }

        protected override void OnAction(Action action, AsyncStatus status)
        {
            var serverDto = GetServerDTO();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, _tenantName);
            ActionHelper.Execute(delegate()
            {
                switch ((ExternalDomainNodeAction)(int)action.Tag)
                {
                    case ExternalDomainNodeAction.ActionProperties:
                        var form = new AddExternalDomain() { ServerDto = serverDto, TenantName = _tenantName, IdentityProviderDto = _provider };
                        var dataContext = SnapInContext.Instance.NavigationController.NavigateToView(this, form);
                        (Parent as ExternalDomainsNode).DoRefresh();
                        break;
                        
                }
            }, auth);
        }
        private ServerDto GetServerDTO()
        {
            var dto = Parent.Parent.Parent.Parent.Tag as AuthTokenDto;
            return dto != null ? dto.ServerDto : null;
        }
        protected override void OnDelete(SyncStatus status)
        {
            if (!ErrorMessageDisplayHelper.Confirm("Delete domain " + _provider.Name + "?"))
                return;
            var serverDto = GetServerDTO();
            var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, _tenantName);
            ActionHelper.Execute(delegate()
            {
                if (service.IdentityProvider.Delete(serverDto, _tenantName, _provider.Name, auth.Token))
                {
                    Parent.Children.Remove(this);
                }
            }, auth);
        }
    }
}

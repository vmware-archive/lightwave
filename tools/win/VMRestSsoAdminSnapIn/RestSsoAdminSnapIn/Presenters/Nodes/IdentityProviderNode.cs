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
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Tenant;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes
{
    public class IdentityProviderNode : ScopeNode
    {
        public enum IdentityProviderNodeAction
        {
            ActionSetDefault = 1,
            ActionRemoveDefault = 2,
            ActionProbe = 3,
        }
        private readonly bool _systemDomain;
        private readonly IdentityProviderDto _provider;
        private TenantConfigurationDto _tenantConfigDto;
        private readonly string _tenantName;

        public IdentityProviderNode(IdentityProviderDto provider, string tenantName, bool systemDomain, string displayName, TenantConfigurationDto tenantConfigDto)
            : base(false)
        {
            _provider = provider;
            _tenantName = tenantName;
            _systemDomain = systemDomain;
            _tenantConfigDto = tenantConfigDto;
            var isDefault = _tenantConfigDto.ProviderPolicy != null && _tenantConfigDto.ProviderPolicy.DefaultProvider == provider.Name;
            if (!isDefault)
            {
                var action = new Action("Set as default domain", "Set as default domain", -1, IdentityProviderNodeAction.ActionSetDefault);
                ActionsPaneItems.Add(action);
            }           
            DisplayName = isDefault ? displayName + " (Default)" : displayName;
            ImageIndex = SelectedImageIndex = systemDomain ? (int)TreeImageIndex.SystemDomain : (int)TreeImageIndex.LocalOs;
        }
        protected override void OnExpand(AsyncStatus status)
        {
            base.OnExpand(status);
            var serverDto = GetServerDto();
            var node = new UsersAndGroupsNode(serverDto, _tenantName, _provider, _systemDomain);
            Children.Add(node);
        }
        private ServerDto GetServerDto()
        {
            var dto = Parent.Parent.Parent.Tag as AuthTokenDto;
            return dto != null ? dto.ServerDto : null;
        }

        protected override void OnAction(Action action, AsyncStatus status)
        {
            base.OnAction(action, status); 
            var serverDto = GetServerDto();
            var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, _tenantName);
            ActionHelper.Execute(delegate()
            {                
                var isDefault = false;
                switch ((IdentityProviderNodeAction)(int)action.Tag)
                {
                    case IdentityProviderNodeAction.ActionSetDefault:
                        _tenantConfigDto.ProviderPolicy = new ProviderPolicyDto { DefaultProvider = _provider.Name, DefaultProviderAlias = _provider.Name };
                        _tenantConfigDto = service.Tenant.UpdateConfig(serverDto, _tenantName, _tenantConfigDto, auth.Token, TenantConfigType.PROVIDER);
                        isDefault = true;
                        break;
                }
                DisplayName = isDefault ? DisplayName + " (Default)" : DisplayName.Replace(" (Default)", string.Empty);
                ((IdentityProvidersNode)(this.Parent)).DoRefresh();
            }, auth);
        }
    }
}

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
using System.Collections.Generic;
using System.Linq;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service.IdentityProvider;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Tenant;
using Action = Microsoft.ManagementConsole.Action;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes
{
    public class IdentityProvidersNode : ScopeNode
    {
        public enum IdentityProvidersNodeAction
        {
            ActionProbeConnectivity = 1,
            ActionRegisterLocalOsDomain = 2
        }
        public IdentityProvidersNode()
            : base(false)
        {
            DisplayName = "Identity Sources";
            AddActions();
            ImageIndex = SelectedImageIndex = (int)TreeImageIndex.IdentityProvider;
        }
        private void AddActions()
        {
            EnabledStandardVerbs = StandardVerbs.Refresh;
        }
        private TenantNode GetTenant()
        {
            return Parent as TenantNode;
        }
        private ServerDto GetServerDto()
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
            DoRefresh();
        }
        protected override void OnAction(Action action, AsyncStatus status)
        {
            base.OnAction(action, status);
        }
        public void DoRefresh()
        {
            Children.Clear();
            var serverDto = GetServerDto();
            var tenantName = GetTenant().DisplayName;
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenantName);
            var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
            ActionHelper.Execute(delegate
           {
               var roles = new List<string> { "Administrator", "RegularUser" };

               if (roles.Contains(auth.Token.Role))
               {
                   var identityProviders = service.IdentityProvider.GetAll(serverDto, tenantName, auth.Token);
                   var tenantConfig = new TenantConfigurationDto();

                   ActionHelper.Execute(delegate
                   {
                       tenantConfig = service.Tenant.GetConfig(serverDto, tenantName, auth.Token, TenantConfigType.PROVIDER);
                   }, null);

                   var systemDomains = identityProviders.Where(x => x.DomainType == DomainType.SYSTEM_DOMAIN.ToString());
                   if (systemDomains != null)
                   {
                       foreach (var provider in systemDomains)
                       {
                           var systemDomain = new IdentityProviderNode(provider, tenantName, true, provider.Name + " (System Domain)", tenantConfig);
                           Children.Add(systemDomain);
                       }
                   }

                   var localOsDomains = identityProviders.Where(x => x.DomainType == DomainType.LOCAL_OS_DOMAIN.ToString());
                   if (localOsDomains != null)
                   {
                       foreach (var provider in localOsDomains)
                       {
                           var localOsDomain = new IdentityProviderNode(provider, tenantName, false, provider.Name + " (Local OS Domain)", tenantConfig);
                           Children.Add(localOsDomain);
                       }
                   }

                   var externalDomains = identityProviders.Where(x => x.DomainType == DomainType.EXTERNAL_DOMAIN.ToString()).ToList();
                   var externalDomainNode = new ExternalDomainsNode(tenantName, externalDomains);
                   Children.Add(externalDomainNode);
               }
           }, auth);
        }
    }
}

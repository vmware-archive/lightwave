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
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service.IdentityProvider;
using Vmware.Tools.RestSsoAdminSnapIn.Views;
using Vmware.Tools.RestSsoAdminSnapIn.Views.Wizard;
namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes{    public class ExternalDomainsNode : ScopeNode    {
        public enum ServerNodeAction
        {
            ActionAddExternalDomain = 1      
        }
        private string _tenantName;
        public ExternalDomainsNode(string tenantName, List<IdentityProviderDto> domains)
            : base(false)                    {
            _tenantName = tenantName;            this.DisplayName = "External Domains";
            AddActions();
            PopulateChildren(domains);
            ImageIndex = SelectedImageIndex = (int)TreeImageIndex.ExternalDomains;        }
        private void AddActions()
        {
            this.EnabledStandardVerbs = StandardVerbs.Refresh;            
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Add External Domain", "Add External Domain", (int)TreeImageIndex.ExternalDomain, ServerNodeAction.ActionAddExternalDomain));
        }
        protected override void OnRefresh(AsyncStatus status)        {            base.OnRefresh(status);
            DoRefresh();
        }
        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)        {            base.OnAction(action, status);
            switch ((ServerNodeAction)(int)action.Tag)            {
                case ServerNodeAction.ActionAddExternalDomain:                    AddExternalDomain();                    break;            }        }
        private ServerDto GetServerDto()
        {
            var dto = Parent.Parent.Parent.Tag as AuthTokenDto;
            return dto != null ? dto.ServerDto : null;
        }
        public void DoRefresh()        {
            this.Children.Clear();
            var serverDto = GetServerDto();
            var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, _tenantName);
            ActionHelper.Execute(delegate()
            {
                var identityProviders = service.IdentityProvider.GetAll(serverDto, _tenantName, auth.Token);
                var externalProviders = identityProviders.Where(x => x.DomainType == DomainType.EXTERNAL_DOMAIN.ToString()).ToList();
                PopulateChildren(externalProviders);
            }, auth);        }
        void PopulateChildren(List<IdentityProviderDto> providers)
        {
            if (providers != null)
            {
                foreach (var provider in providers)
                    Children.Add(new ExternalDomainNode(_tenantName, provider));
            }
        }
        void AddExternalDomain()        {
            var serverDto = GetServerDto();
            var form = new AddExternalDomain() { ServerDto = serverDto, TenantName = _tenantName };
            var dataContext = SnapInContext.Instance.NavigationController.NavigateToView(this, form);
            DoRefresh();        }    }}
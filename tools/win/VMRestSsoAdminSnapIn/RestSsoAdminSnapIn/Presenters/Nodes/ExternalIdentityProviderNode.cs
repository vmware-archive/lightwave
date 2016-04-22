/*
Copyright 2015 VMware, Inc. All rights reserved. VMware Confidential
*/
using System;
using Microsoft.ManagementConsole;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Views;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.ExternalDomain;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes{
    public class ExternalIdentityProviderNode : ScopeNode    {
        private IdentityProviderDto _provider;        
        private string _tenantName;

        public ExternalIdentityProviderNode(string tenantName, IdentityProviderDto provider)
            : base(false)        {
            _tenantName = tenantName;            _provider = provider;                        this.DisplayName = _provider.Name;
            this.EnabledStandardVerbs = StandardVerbs.Properties|StandardVerbs.Delete;
            ImageIndex = SelectedImageIndex = (int)TreeImageIndex.ExternalDomain;
        }
        protected override void OnExpand(AsyncStatus status)        {            base.OnExpand(status);            var serverDto = GetServerDTO();
            var userAndgroupNode = new UsersAndGroupsNode(serverDto, _tenantName, _provider, false);
            this.Children.Add(userAndgroupNode);        }
        private ServerDto GetServerDTO()        {            
            var dto = Parent.Parent.Parent.Parent.Tag as AuthTokenDto;
            return dto != null ? dto.ServerDto : null;        }      
        protected override void  OnDelete(SyncStatus status)
        {            if(!ErrorMessageDisplayHelper.Confirm("Delete domain " + _provider.Name + "?"))                return;

            ActionHelper.Execute(delegate()
            {
                var serverDto = GetServerDTO();
                var service = this.GetServiceGateway();
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto.ServerName);
                if (this.GetServiceGateway().IdentityProvider.Delete(serverDto, _tenantName, _provider.Name, auth.Token))
                {
                    Parent.Children.Remove(this);
                }
            });        }

        protected override void OnAddPropertyPages(PropertyPageCollection propertyPageCollection)
        {
            base.OnAddPropertyPages(propertyPageCollection);
            ActionHelper.Execute(delegate()
            {
                var serverDto = GetServerDTO();
                var service = this.GetServiceGateway();
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto.ServerName);
                var dto = service.IdentityProvider.Get(serverDto, _tenantName, _provider.Name, auth.Token);

                if (dto != null)
                {
                    var mgr = new ExternalDomainPropertyManager(_tenantName, service, serverDto, auth.Token, dto);
                    var generalPage = new ExternalDomainGeneralProperty(dto);
                    generalPage.Title = "General";
                    propertyPageCollection.Add(generalPage.Page);

                    var authPage = new ExternalDomainAuthProperty(dto);
                    authPage.Title = "Auth details";
                    propertyPageCollection.Add(authPage.Page);
                }
            });
        }    }}
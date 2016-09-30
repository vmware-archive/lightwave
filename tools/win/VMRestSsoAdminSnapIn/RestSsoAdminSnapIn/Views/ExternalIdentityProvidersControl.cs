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
using System.ComponentModel;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Contracts;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class ExternalIdentityProvidersControl : UserControl, IFormViewControl
    {
        ExternalIdentityProviderFormView _formView;

        public ExternalIdentityProvidersControl()
        {
            InitializeComponent();
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            Dock = DockStyle.Fill;
            var il = new ImageList();
            il.Images.AddStrip(ResourceHelper.GetToolbarImage());
            lstExternalIdentityProviders.SmallImageList = il;
        }

        string GetTenantName()
        {
            var ssoUsersNode = _formView.ScopeNode as ExternalIdentityProvidersNode;
            return ssoUsersNode != null ? ssoUsersNode.GetTenant().DisplayName : null;
        }

        Dto.ServerDto GetServerDto()
        {
            var ssoUsersNode = _formView.ScopeNode as ExternalIdentityProvidersNode;
            return ssoUsersNode != null ? ssoUsersNode.GetServerDto() : null;
        }
        void IFormViewControl.Initialize(FormView parentSelectionFormView)
        {
            _formView = (ExternalIdentityProviderFormView)parentSelectionFormView;
            _formView.SelectionData.ActionsPaneItems.Clear();
            RefreshExternalIdentityProviders();
        }
        public void RefreshExternalIdentityProviders()
        {
            var serverDto = GetServerDto();
            var tenantName = GetTenantName();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenantName);
            ActionHelper.Execute(delegate
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                    IExternalIdentityProviderService externalIdp = service.ExternalIdentityProvider;
                    var externalIdps = externalIdp.GetAll(serverDto, tenantName, auth.Token);
                    lstExternalIdentityProviders.Items.Clear();
                    if (externalIdps != null)
                    {
                        foreach (var idp in externalIdps)
                        {
                            var lvItem = new ListViewItem(new[] { idp.EntityID, idp.Alias, idp.JitEnabled ? "YES" : "NO" }) { Tag = idp };
                            lvItem.ImageIndex = (int)TreeImageIndex.IdentityProvider;
                            lstExternalIdentityProviders.Items.Add(lvItem);
                        }
                    }
                }, auth);
        }

        void ShowExternalIdentityProvider()
        {
            if (lstExternalIdentityProviders.SelectedItems.Count > 0)
            {
                var idp = lstExternalIdentityProviders.SelectedItems[0].Tag as ExternalIdentityProviderDto;
                var serverDto = GetServerDto();
                ActionHelper.Execute(delegate
                {
                    var tenantName = GetTenantName();
                    var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                    var form = new NewExternalIdp(idp, service, serverDto, tenantName);
                    form.ShowDialog();
                }, null);
                RefreshExternalIdentityProviders();
            }
        }

        private void lstUsers_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (lstExternalIdentityProviders.SelectedIndices.Count == 0)
                return;

            ShowExternalIdentityProvider();
        }

        private void propertiesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowExternalIdentityProvider();
        }

        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (lstExternalIdentityProviders.SelectedIndices.Count == 0)
                return;

            var idp = (ExternalIdentityProviderDto)lstExternalIdentityProviders.Items[lstExternalIdentityProviders.SelectedIndices[0]].Tag;
            var message = "Delete external identity provider " + idp.EntityID + " ? ";
            if (MMCDlgHelper.ShowConfirm(message))
            {
                var serverDto = GetServerDto();
                var tenantName = GetTenantName();
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenantName);
                ActionHelper.Execute(delegate
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                    IExternalIdentityProviderService externalIdp = service.ExternalIdentityProvider;
                    externalIdp.Delete(serverDto, tenantName, idp, auth.Token);
                    RefreshExternalIdentityProviders();
                }, auth);
            }
        }

        private void contextMenuStrip1_Opening(object sender, CancelEventArgs e)
        {
            if (lstExternalIdentityProviders.SelectedIndices.Count == 0)
                e.Cancel = true;
        }
    }
}

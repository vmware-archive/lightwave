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
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class OidcClientsControl : UserControl, IFormViewControl
    {
        OidcClientsFormView _formView;

        public OidcClientsControl()
        {
            InitializeComponent();
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            Dock = DockStyle.Fill;
            var il = new ImageList();
            il.Images.AddStrip(ResourceHelper.GetToolbarImage());
            lstOidcClients.SmallImageList = il;
        }
       
        string GetTenantName()
        {
            var ssoUsersNode = _formView.ScopeNode as OidcClientsNode;
            return ssoUsersNode != null ? ssoUsersNode.GetTenant().DisplayName : null;
        }

        Dto.ServerDto GetServerDto()
        {
            var ssoUsersNode = _formView.ScopeNode as OidcClientsNode;
            return ssoUsersNode != null ? ssoUsersNode.GetServerDto() : null;
        }
        void IFormViewControl.Initialize(FormView parentSelectionFormView)
        {
            _formView = (OidcClientsFormView)parentSelectionFormView;
            _formView.SelectionData.ActionsPaneItems.Clear();
            RefreshOidcClients();
        }
        public void RefreshOidcClients()
        { 
            var serverDto = GetServerDto();
            var tenantName = GetTenantName();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenantName);
            ActionHelper.Execute(delegate
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                    var externalIdps = service.OidcClient.GetAll(serverDto, tenantName, auth.Token);
                    lstOidcClients.Items.Clear();
                    if (externalIdps != null)
                    {
                        foreach (var idp in externalIdps)
                        {
                            var lvItem = new ListViewItem(new[] { idp.ClientId, idp.OidcClientMetadataDTO.CertSubjectDN, idp.OidcClientMetadataDTO.TokenEndpointAuthMethod, idp.OidcClientMetadataDTO.LogoutUri }) { Tag = idp };
                            lvItem.ImageIndex = (int)TreeImageIndex.IdentityProvider;
                            lstOidcClients.Items.Add(lvItem);
                        }
                    }
                }, auth);
        }

        void ShowOidcClient()
        {
            if (lstOidcClients.SelectedItems.Count > 0)
            {
                var idp = lstOidcClients.SelectedItems[0].Tag as OidcClientDto;
                var serverDto = GetServerDto();
                ActionHelper.Execute(delegate
                {
                    var tenantName = GetTenantName();
                    var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                    var form = new NewOidcClient(idp, service, serverDto, tenantName);
                    form.ShowDialog();
                }, null);
                RefreshOidcClients();
            }
        }

        private void lstUsers_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (lstOidcClients.SelectedIndices.Count == 0)
                return;

            ShowOidcClient();
        }

        private void propertiesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowOidcClient();
        }

        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (lstOidcClients.SelectedIndices.Count == 0)
                return;

            var idp = (OidcClientDto)lstOidcClients.Items[lstOidcClients.SelectedIndices[0]].Tag;
            var message = "Delete OIDC Client " + idp.ClientId + " ? ";
            if (MMCDlgHelper.ShowConfirm(message))
            {  
                var serverDto = GetServerDto();
                var tenantName = GetTenantName();
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenantName);
                    
                ActionHelper.Execute(delegate
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                    service.OidcClient.Delete(serverDto, tenantName, idp, auth.Token);
                    RefreshOidcClients();
                }, auth);
            }
        }

        private void contextMenuStrip1_Opening(object sender, CancelEventArgs e)
        {
            if (lstOidcClients.SelectedIndices.Count == 0)
                e.Cancel = true;
        }     
    }
}

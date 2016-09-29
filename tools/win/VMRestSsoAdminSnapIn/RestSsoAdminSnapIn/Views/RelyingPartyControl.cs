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
    public partial class RelyingPartyControl : UserControl, IFormViewControl
    {
        RelyingPartyFormView _formView;
        private ServiceGateway _ssoAdminSdkService;

        public RelyingPartyControl()
        {
            InitializeComponent();
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            Dock = DockStyle.Fill;
            var il = new ImageList();
            il.Images.AddStrip(ResourceHelper.GetToolbarImage());
            lstRelyingParties.SmallImageList = il;
        }
        private string GetTenantName()
        {
            var node = _formView.ScopeNode as RelyingPartyNode;
            return node != null ? node.GetTenant().DisplayName : null;
        }

        private Dto.ServerDto GetServerDto()
        {
            var node = _formView.ScopeNode as RelyingPartyNode;
            return node != null ? node.GetServerDto() : null;
        }
        void IFormViewControl.Initialize(FormView parentSelectionFormView)
        {
            _formView = (RelyingPartyFormView)parentSelectionFormView;
            _formView.SelectionData.ActionsPaneItems.Clear();
            RefreshRelyingParty();
        }
        public void RefreshRelyingParty()
        {
            var serverDto = GetServerDto();
            var tenantName = GetTenantName();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenantName);
            ActionHelper.Execute(delegate
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                    var relyingParties = service.RelyingParty.GetAll(serverDto, tenantName, auth.Token);
                    lstRelyingParties.Items.Clear();
                    if (relyingParties != null)
                    {
                        foreach (var relyingParty in relyingParties)
                        {
                            var lvItem = new ListViewItem(new[] { relyingParty.Name, relyingParty.Url }) { Tag = relyingParty };
                            lvItem.ImageIndex = (int)TreeImageIndex.Server;
                            lstRelyingParties.Items.Add(lvItem);
                        }
                    }
                }, auth);
        }

        void ShowRelyingParty()
        {
            var rp = lstRelyingParties.SelectedItems[0].Tag as RelyingPartyDto;
            var serverDto = GetServerDto();
            ActionHelper.Execute(delegate
            {
                var tenantName = GetTenantName();
                var form = new NewRelyingParty(rp, serverDto, tenantName);
                form.ShowDialog();
                RefreshRelyingParty();
            }, null);
        }

        private void lstUsers_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (lstRelyingParties.SelectedIndices.Count == 0)
                return;

            ShowRelyingParty();
        }

        private void propertiesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowRelyingParty();
        }

        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (lstRelyingParties.SelectedIndices.Count == 0)
                return;

            var rp = (RelyingPartyDto)lstRelyingParties.Items[lstRelyingParties.SelectedIndices[0]].Tag;
            var message = "Delete relying Party " + rp.Name + " ? ";
            if (MMCDlgHelper.ShowConfirm(message))
            {
                var serverDto = GetServerDto();
                var tenantName = GetTenantName();
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenantName);
                ActionHelper.Execute(delegate
                {

                    var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                    service.RelyingParty.Delete(serverDto, tenantName, rp, auth.Token);
                    RefreshRelyingParty();
                }, auth);
            }
        }

        private void contextMenuStrip1_Opening(object sender, CancelEventArgs e)
        {
            if (lstRelyingParties.SelectedIndices.Count == 0)
                e.Cancel = true;
        }
    }
}

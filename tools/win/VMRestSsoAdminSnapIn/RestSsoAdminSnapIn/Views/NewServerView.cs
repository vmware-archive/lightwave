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

 using System;
using System.Diagnostics;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using VMwareMMCIDP.UI.Common.Utilities;
using VMIdentity.CommonUtils;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class NewServerView : Form, IView
    {
        private readonly Dto.ServerDto _dto;
        private readonly ServiceGateway _service;
        private bool shouldClose = true;

        public IDataContext DataContext
        {
            get
            {   
                return _dto;
            }
        }
        public NewServerView(ServiceGateway service)
        {
            InitializeComponent();
            _service = service;
            _dto = new Dto.ServerDto();
            cbIsSsl.Checked = true;
            lblStsUrl.Visible = cbSAML.Checked;
            txtStsUrl.Visible = cbSAML.Checked;
            this.txtDefaultTenant.Text = MMCMiscUtil.GetBrandConfig(CommonConstants.TENANT);
            UpdateUrlPreview();
        }
        void UpdateUrlPreview()
        {
            var protocol = cbIsSsl.Checked ? "https" : "http";
            var urlPreview = cbSAML.Checked
                ? _service.GetTenantEndpoint(true, protocol, txtServer.Text, txtPort.Text, txtStsUrl.Text + "/" + txtDefaultTenant.Text) 
                : _service.GetTenantEndpoint(false, protocol, txtServer.Text, txtPort.Text, txtDefaultTenant.Text);
            lnkURLPreview.Text = urlPreview;
        }
        private void btnAdd_Click(object sender, EventArgs e)
        {
            if (ValidateInputs())
            {   
                _dto.ServerName = txtServer.Text;
                _dto.Port = txtPort.Text;
                _dto.Tenant = txtDefaultTenant.Text;
                _dto.Protocol = cbIsSsl.Checked ? "https" : "http";
                _dto.TokenType = cbSAML.Checked ? TokenType.SAML : TokenType.Bearer;
                _dto.Url = lnkURLPreview.Text;
                _dto.StsUrl = cbSAML.Checked ? txtStsUrl.Text : string.Empty; ;
                DialogResult = DialogResult.OK;
                shouldClose = true;
                Close();
            }
            else
            {
                shouldClose = false;
            }
        }
        private bool ValidateInputs()
        {
            if (string.IsNullOrEmpty(txtServer.Text))
            {
                MMCDlgHelper.ShowWarning("Please enter a valid Server name");
                return false;
            }

            int port;
            if (string.IsNullOrEmpty(txtPort.Text) && int.TryParse(txtPort.Text,out port))
            {
                MMCDlgHelper.ShowWarning("Please enter a valid port");
                return false;
            }

            if (string.IsNullOrEmpty(txtDefaultTenant.Text))
            {
                MMCDlgHelper.ShowWarning("Please enter a valid Default Tenant name");
                return false;
            }
            if (cbSAML.Checked && string.IsNullOrEmpty(txtStsUrl.Text))
            {
                MMCDlgHelper.ShowWarning("Please enter a valid STS Url");
                return false;
            }
            if (!WebUtil.PingHost(txtServer.Text))
            {
                MMCDlgHelper.ShowWarning("Server is invalid or unreachable");
                return false;
            }
            return true;
        }
        private void cbIsHttp_CheckedChanged(object sender, EventArgs e)
        {
            UpdateUrlPreview();
        }
        
        private void txtPort_TextChanged(object sender, EventArgs e)
        {
            UpdateUrlPreview();
        }
        private void txtDefaultTenant_TextChanged(object sender, EventArgs e)
        {
            UpdateUrlPreview();
        }
        private void txtServer_TextChanged(object sender, EventArgs e)
        {
            UpdateUrlPreview();
        }
        private void lnkURLPreview_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Process.Start(lnkURLPreview.Text);
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            base.OnClosing(e);
            e.Cancel = !shouldClose;
            shouldClose = true;
        }

        private void cbSAML_CheckedChanged(object sender, EventArgs e)
        {
            lblStsUrl.Visible = cbSAML.Checked;
            txtStsUrl.Visible = cbSAML.Checked;
            UpdateUrlPreview();
        }

        private void txtStsUrl_TextChanged(object sender, EventArgs e)
        {
            UpdateUrlPreview();
        }
    }
}

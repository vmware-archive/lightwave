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
using System.IO;
using System.Collections.Generic;
using System.Security.Cryptography.X509Certificates;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using System.Text;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class NewOidcClient : Form, IView
    {
        private readonly ServiceGateway _service;
        private readonly ServerDto _serverDto;
        private readonly string _tenantName;
        private OidcClientDto _oidcClientDtoOrig;
        public IDataContext DataContext
        {
            get { return _oidcClientDtoOrig; }
        }
        public NewOidcClient(ServiceGateway service, ServerDto serverDto, string tenantName)
        {
            _service = service;
            _serverDto = serverDto;
            _tenantName = tenantName;
            InitializeComponent();
            Text = "New OIDC Client";
            btnCreate.Text = "Create";
        }

        public NewOidcClient(OidcClientDto oidcClientDto, ServiceGateway service, ServerDto serverDto, string tenantName)
        {
            _service = service;
            _serverDto = serverDto;
            _tenantName = tenantName;
            _oidcClientDtoOrig = oidcClientDto;
            InitializeComponent();            
            DtoToView(_oidcClientDtoOrig);
            Text = "OIDC Client";
            btnCreate.Text = "Update";
        }
        private void DtoToView(OidcClientDto oidcClientDto)
        {
            txtClientId.Text = oidcClientDto.ClientId;
            txtSubjectDN.Text = oidcClientDto.OidcClientMetadataDTO.CertSubjectDN;
            txtLogoutUri.Text = oidcClientDto.OidcClientMetadataDTO.LogoutUri;
            cbTokenAuth.SelectedIndex = oidcClientDto.OidcClientMetadataDTO.TokenEndpointAuthMethod == "none" ? 0 : 1;
            if (oidcClientDto.OidcClientMetadataDTO.RedirectUris != null)
            {
                foreach (var value in oidcClientDto.OidcClientMetadataDTO.RedirectUris)
                {
                    var lstItem = new ListViewItem(value) { Tag = value };
                    lstRedirectUris.Items.Add(lstItem);
                }
            }

            if (oidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris != null)
            {
                foreach (var value in oidcClientDto.OidcClientMetadataDTO.PostLogoutRedirectUris)
                {
                    var lstItem = new ListViewItem(value) { Tag = value };
                    lstPostLogoutRedirectUris.Items.Add(lstItem);
                }
            }
        }
        private void btnCreateSignerIdentity_Click(object sender, EventArgs e)
        {
            if (ValidateInputs())
            {
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
                ActionHelper.Execute(delegate
                {   
                    var oidcClientMetadataDTO = new OidcClientMetadataDto
                    {
                        CertSubjectDN = txtSubjectDN.Text,
                        LogoutUri = string.IsNullOrWhiteSpace(txtLogoutUri.Text) ? null : txtLogoutUri.Text,
                        TokenEndpointAuthMethod = cbTokenAuth.Items[cbTokenAuth.SelectedIndex].ToString(),
                        RedirectUris = GetRedirectUris(),
                        PostLogoutRedirectUris = GetPostLogoutRedirectUris()
                    };
                    if (_oidcClientDtoOrig == null)
                    {
                     
                        _oidcClientDtoOrig = _service.OidcClient.Create(_serverDto, _tenantName, oidcClientMetadataDTO, auth.Token);
                    }
                    else
                    {
                        _oidcClientDtoOrig = _service.OidcClient.Update(_serverDto, _tenantName, txtClientId.Text, oidcClientMetadataDTO, auth.Token);
                    }
                }, auth);
                this.DialogResult = DialogResult.OK;
            }
        }
        private List<string> GetRedirectUris()
        {
            var redirectUri = new List<string>();
            foreach (ListViewItem item in lstRedirectUris.Items)
            {
                redirectUri.Add(item.Tag.ToString());
            }
            return redirectUri.Count == 0 ? null : redirectUri;
        }
        private List<string> GetPostLogoutRedirectUris()
        {
            var redirectUri = new List<string>();
            foreach (ListViewItem item in lstPostLogoutRedirectUris.Items)
            {
                redirectUri.Add(item.Tag.ToString());
            }
            return redirectUri.Count == 0 ? null : redirectUri;
        }
        private bool ValidateInputs()
        {
            if (cbTokenAuth.SelectedIndex == 1 && string.IsNullOrEmpty(txtSubjectDN.Text))
            {
                MMCDlgHelper.ShowWarning("Enter a valid certificate subject DN");
                return false;
            }
            if (!string.IsNullOrWhiteSpace(txtLogoutUri.Text) && !txtLogoutUri.Text.StartsWith("https:"))
            {
                MMCDlgHelper.ShowWarning("Enter a valid HTTPS Logout URI");
                return false;
            }
            if (lstRedirectUris.Items.Count > 0)
            {
                foreach (ListViewItem item in lstRedirectUris.Items)
                {
                    if (!item.SubItems[0].Text.StartsWith("https:"))
                    {
                        MMCDlgHelper.ShowWarning("Enter a valid HTTPS Redirect URI - " + item.ToString());
                        return false;
                    }
                }
            }
            else
            {
                MMCDlgHelper.ShowWarning("No Redirect URI specified");
                return false;
            }
            if (lstPostLogoutRedirectUris.Items.Count > 0)
            {
                foreach (ListViewItem item in lstPostLogoutRedirectUris.Items)
                {
                    if (!item.SubItems[0].Text.StartsWith("https:"))
                    {
                        MMCDlgHelper.ShowWarning("Enter a valid HTTPS Post Logout Redirect URI - " + item.ToString());
                        return false;
                    }
                }
            }
            return true;
        }
        private void btnAddRedirectUri_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                var form = new AddNameIdFormat("Redirect Uri", true);
                if (form.ShowDialog() == DialogResult.OK)
                {
                    var lstItem = new ListViewItem(form.NameIdFormatString) { Tag = form.NameIdFormatString };
                    lstRedirectUris.Items.Add(lstItem);
                }
            }, null);
        }
        private void btnRemoveRedirectUri_Click(object sender, EventArgs e)
        {
            if (lstRedirectUris.SelectedItems.Count > 0)
            {
                var selectedIndex = lstRedirectUris.SelectedIndices[0];
                lstRedirectUris.Items.RemoveAt(selectedIndex);
            }
        }
        private void lstRedirectUris_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveRedirecturi.Enabled = lstRedirectUris.SelectedIndices != null && lstRedirectUris.SelectedIndices.Count > 0;
        }
        private void btnAddPostLogoutRedirectUri_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                var form = new AddNameIdFormat("Post Logout Redirect Uri", true);
                if (form.ShowDialog() == DialogResult.OK)
                {
                    var lstItem = new ListViewItem(form.NameIdFormatString) { Tag = form.NameIdFormatString };
                    lstPostLogoutRedirectUris.Items.Add(lstItem);
                }
            }, null);
        }

        private void btnRemovePostlogoutUri_Click(object sender, EventArgs e)
        {
            if (lstPostLogoutRedirectUris.SelectedItems.Count > 0)
            {
                var selectedIndex = lstPostLogoutRedirectUris.SelectedIndices[0];
                lstPostLogoutRedirectUris.Items.RemoveAt(selectedIndex);
            }
        }

        private void lstPostLogoutRedirectUris_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemovePostlogoutUri.Enabled = lstPostLogoutRedirectUris.SelectedIndices != null && lstPostLogoutRedirectUris.SelectedIndices.Count > 0;
        }

        private void NewOidcClient_Load(object sender, EventArgs e)
        {
            if (_oidcClientDtoOrig == null)
                cbTokenAuth.SelectedIndex = 0;
        }
    }
}

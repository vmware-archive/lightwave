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
using Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class NewSignerIdentity : Form, IView
    {
        private readonly ServiceGateway _service;
        private readonly ServerDto _serverDto;
        private TenantDto _tenantDto;
        private string _tenantName;
        public IDataContext DataContext
        {
            get { return _tenantDto; }
        }
        public NewSignerIdentity(ServiceGateway service, ServerDto serverDto)
        {
            _service = service;
            _serverDto = serverDto;
            InitializeComponent();
            txtTenantName.Enabled = true;
            txtUsername.Enabled = true;
            txtPassword.Enabled = true;
            txtTenantName.Text = string.Empty;
            Text = "Add New Tenant";
            toolTip1.ToolTipTitle = Text;
        }
        public NewSignerIdentity(ServiceGateway service, ServerDto serverDto, string tenantName)
        {
            _service = service;
            _serverDto = serverDto;
            _tenantName = tenantName;
            InitializeComponent();
            var isEnabled = string.IsNullOrEmpty(tenantName);
            txtTenantName.Enabled = isEnabled;
            txtUsername.Enabled = isEnabled;
            txtPassword.Enabled = isEnabled;
            txtTenantName.Text = tenantName;
            Text = txtTenantName.Enabled ? "Add New Tenant" : "New Signer Identity";
            toolTip1.ToolTipTitle = Text;
        }

        private void btnSelectKeyFile_Click(object sender, EventArgs e)
        {
            using (var ofd = new OpenFileDialog())
            {
                ofd.Filter = MMCUIConstants.PRI_KEY_FILTER;
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    txtKeyFile.Text = ofd.FileName;
                }
            }
        }

        private void btnSelectCertFile_Click(object sender, EventArgs e)
        {
            using (var ofd = new OpenFileDialog())
            {
                ofd.Filter = MMCUIConstants.CERT_FILTER;
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    listviewCertificates.Items.Add(ofd.FileName);
                }
            }
        }

        private void btnCreateSignerIdentity_Click(object sender, EventArgs e)
        {
            if (ValidateInputs())
            {
                AuthTokenDto auth = null;
                if (txtTenantName.Enabled)
                {
                    var auths = SnapInContext.Instance.AuthTokenManager.GetAuthTokens(_serverDto);
                    auth = auths[0];
                }
                else
                    auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);

                ActionHelper.Execute(delegate
                {
                    // Get private key
                    var keyText = File.ReadAllText(txtKeyFile.Text);
                    keyText = PrivateKeyHelper.ExtractBase64EncodedPayload(keyText);
                    EncrptionAlgorithm algo;
                    if (cbAlgo.SelectedIndex > -1)
                    {
                        Enum.TryParse(cbAlgo.SelectedItem.ToString(), false, out algo);
                    }
                    else
                    {
                        algo = EncrptionAlgorithm.RSA;
                    }
                    var privatekey = new PrivateKeyDto { Encoded = keyText, Algorithm = algo };

                    // Get all certificates
                    var certs = new List<CertificateDto>();
                    foreach (var certificate in GetCertificateChain())
                    {
                        var cert = new X509Certificate2();
                        cert.Import(certificate);
                        var certDto = new CertificateDto { Encoded = cert.ExportToPem() };
                        certs.Add(certDto);
                    }
                    var tenantCredentials = new TenantCredentialsDto { Certificates = certs, PrivateKey = privatekey };

                    if (txtTenantName.Enabled)
                    {
                        var tenantDto = new TenantDto() { Name = txtTenantName.Text, Credentials = tenantCredentials, Username = txtUsername.Text, Password = txtPassword.Text };
                        _tenantDto = _service.Tenant.Create(_serverDto, tenantDto, auth.Token);
                    }
                    else
                    {
                        _service.Certificate.SetTenantCredentials(_serverDto, _tenantName, tenantCredentials, auth.Token);
                    }
                }, auth);
                this.DialogResult = DialogResult.OK;
            }
        }

        private bool ValidateInputs()
        {
            if (txtTenantName.Enabled && string.IsNullOrEmpty(txtTenantName.Text))
            {
                MMCDlgHelper.ShowWarning(MMCUIConstants.VALID_TENANT_ENT);
                return false;
            } else
            if (txtTenantName.Enabled && string.IsNullOrEmpty(txtUsername.Text))
            {
                MMCDlgHelper.ShowWarning(MMCUIConstants.USERNAME_ENT);
                return false;
            } else
            if (txtTenantName.Enabled && string.IsNullOrEmpty(txtPassword.Text))
            {
                MMCDlgHelper.ShowWarning(MMCUIConstants.PASSWORD_ENT);
                return false;
            } else
            if (string.IsNullOrEmpty(txtKeyFile.Text))
            {
                MMCDlgHelper.ShowWarning(MMCUIConstants.PRI_KEY_SEL);
                return false;
            }
            else if (listviewCertificates.Items.Count <= 1)
            {
                MMCDlgHelper.ShowWarning(MMCUIConstants.CERT_CHAIN_SEL);
                return false;
            }
            else
            { 
                try
                {
                    if (!string.IsNullOrEmpty(txtKeyFile.Text) && !System.IO.File.Exists(txtKeyFile.Text))
                    {
                        MMCDlgHelper.ShowWarning(MMCUIConstants.PRI_KEY_NOT_FOUND);
                        return false;
                    }
                }
                catch(Exception exception)
                {
                    MMCDlgHelper.ShowWarning(exception.Message);
                    return false;
                }
            }            
            return true;
        }

        List<string> GetCertificateChain()
        {
            var list = new List<string>();
            foreach (ListViewItem item in listviewCertificates.Items)
            {
                list.Add(item.Text);
            }
            return list;
        }

        private void btnRemoveCert_Click(object sender, EventArgs e)
        {
            if (listviewCertificates.SelectedItems != null && listviewCertificates.SelectedItems.Count > 0)
            {
                foreach (ListViewItem item in listviewCertificates.SelectedItems)
                    item.Remove();
            }
        }
        private void NewSignerIdentity_Load(object sender, EventArgs e)
        {
            cbAlgo.SelectedIndex = 0;
        }

        private void listviewCertificates_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveCert.Enabled = (listviewCertificates.SelectedItems != null && listviewCertificates.SelectedItems.Count > 0);
        }
    }
}

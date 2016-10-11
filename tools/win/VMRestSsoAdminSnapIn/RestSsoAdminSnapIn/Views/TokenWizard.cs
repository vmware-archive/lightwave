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
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Crypto;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class TokenWizard : Form
    {
        private bool close = false;
        public TokenWizard()
        {
            InitializeComponent();
            cbIsSsl.Checked = true;
            UpdateUrlPreview();
            SetControlVisibility();
        }
        void UpdateUrlPreview()
        {
            var protocol = cbIsSsl.Checked ? "https" : "http";
            var service = ScopeNodeExtensions.GetServiceGateway();
            var urlPreview = cbSaml.Checked
                ? service.GetTenantEndpoint(true, protocol, txtServer.Text, txtPort.Text, txtStsUri.Text + "/" + txtDefaultTenant.Text)
                : service.GetTenantEndpoint(false, protocol, txtServer.Text, txtPort.Text, txtDefaultTenant.Text);
            lnkURLPreview.Text = urlPreview;
        }
        private string GetProtocol()
        {
            return cbIsSsl.Checked ? "https" : "http";
        }
        private void btnAdd_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate()
            {
                txtSamlToken.Text = string.Empty;
                txtIdToken.Text = string.Empty;
                txtAccessToken.Text = string.Empty;
                txtRefreshToken.Text = string.Empty;
                if (ValidateInputs())
                {
                    var service = ScopeNodeExtensions.GetServiceGateway();
                    var serverDto = new ServerDto() { Tenant = txtDefaultTenant.Text, ServerName = txtServer.Text, Port = txtPort.Text, Protocol = GetProtocol() };
                    serverDto.TokenType = cbSaml.Checked ? TokenType.SAML : TokenType.Bearer;
                    serverDto.Url = lnkURLPreview.Text;
                    serverDto.StsUrl = cbSaml.Checked ? txtStsUri.Text : string.Empty;
                    if (cbSaml.Checked)
                    {
                        if (loginTab.SelectedTab == tabUser)
                        {
                            GetSamlTokenByUserCredentials(service, serverDto);
                        }
                        else if (loginTab.SelectedTab == tabGssTicket)
                        {
                            var token = GetSamlTokenByGss(service, serverDto);
                            txtSamlToken.Text = token;
                        }
                        else if (loginTab.SelectedTab == tabSolutionUser)
                        {
                             var isValid = ValidateSolutionUserFields();
                             if (isValid)
                             {
                                 var token = GetSamlTokenByCertificate(service, serverDto);
                                 txtSamlToken.Text = token;
                             }
                        }
                        else if (loginTab.SelectedTab == tabTokenFile)
                        {
                            var isValid = ValidateTokenFields();
                            if (isValid)
                            {
                                var token = GetSamlTokenByToken(service, serverDto);
                                txtSamlToken.Text = token;
                            }
                        }
                    }
                    else
                    {
                        if (loginTab.SelectedTab == tabUser)
                        {
                            var authToken = GetJwtTokenByUserCredentials(service, serverDto);
                            PopulateToken(authToken);
                        }
                        else if (loginTab.SelectedTab == tabSolutionUser)
                        {
                            var isValid = ValidateSolutionUserFields();
                            if (isValid)
                            {
                                var authToken = GetJwtTokenByCertificate(service, serverDto);
                                PopulateToken(authToken);
                            }
                        }
                        else if (loginTab.SelectedTab == tabGssTicket)
                        {
                            var authToken = GetJwtTokenByGssTicket(service, serverDto);
                            PopulateToken(authToken);
                        }
                    }
                }
            }, null);
            close = false;
        }

        private bool ValidateTokenFields()
        {
            var message = string.Empty;
            if(!File.Exists(txtTokPkey.Text))
            {
                message = MMCUIConstants.PRI_KEY_SEL;
            }
            else if (!File.Exists(txtTokCert.Text))
            {
                message = MMCUIConstants.CERT_SEL;
            }
            else if (!File.Exists(txtTokenFilePath.Text))
            {
                message = MMCUIConstants.TOKEN_SEL;
            }

            if (!string.IsNullOrEmpty(message))
            {
                MMCDlgHelper.ShowWarning(message);
                return false;
            }
            return true;
        }

        private bool ValidateSolutionUserFields()
        {
            var message = string.Empty;
            if (!File.Exists(txtPrivateKey.Text))
            {
                message = MMCUIConstants.PRI_KEY_SEL;
            }
            else if (!File.Exists(txtCertificate.Text))
            {
                message = MMCUIConstants.CERT_SEL;
            }

            if (!string.IsNullOrEmpty(message))
            {
                MMCDlgHelper.ShowWarning(message);
                return false;
            }
            return true;
        }

        private AuthTokenDto GetJwtTokenByUserCredentials(ServiceGateway service, ServerDto serverDto)
        {
            var loginDto = new LoginDto() { DomainName = txtDomainName.Text, TenantName = txtDefaultTenant.Text, User = txtUser.Text, Pass = txtPass.Text };
            return service.JwtTokenService.Authenticate(serverDto, loginDto, Constants.ClientId);
        }

        private AuthTokenDto GetJwtTokenByCertificate(ServiceGateway service, ServerDto serverDto)
        {
            var cert = new X509Certificate2(txtCertificate.Text);            
            var rsaKey = ShaWithRsaSigner.PrivatePemKeyToRSACryptoServiceProvider(txtPrivateKey.Text);
            return service.JwtTokenService.GetTokenFromCertificate(serverDto, cert, rsaKey);
        }

        private AuthTokenDto GetJwtTokenByGssTicket(ServiceGateway service, ServerDto serverDto)
        {
            var gssTicket = KerberosHelper.GetKerberosContext(txtGssUsername.Text, txtGssPassword.Text, txtGssDomain.Text, txtGssSpn.Text);
            var gssBase64Encoded = Uri.EscapeDataString(gssTicket);
            return service.JwtTokenService.GetTokenFromGssTicket(serverDto, gssBase64Encoded, Guid.NewGuid().ToString());            
        }

        private string GetSamlTokenByToken(ServiceGateway service, ServerDto serverDto)
        {
            var cert = new X509Certificate2(txtTokCert.Text);
            var rsaKey = ShaWithRsaSigner.PrivatePemKeyToRSACryptoServiceProvider(txtTokPkey.Text);
            var tokenText = File.ReadAllText(txtTokenFilePath.Text);
            var token = service.SamlTokenService.GetSamlTokenFromToken(serverDto, tokenText, cert, rsaKey);
            return token;
        }

        private string GetSamlTokenByCertificate(ServiceGateway service, ServerDto serverDto)
        {
            var cert = new X509Certificate2(txtCertificate.Text); 
            var rsaKey = ShaWithRsaSigner.PrivatePemKeyToRSACryptoServiceProvider(txtPrivateKey.Text);
            var token = service.SamlTokenService.GetSamlTokenFromCertificate(serverDto, cert, rsaKey);
            return token;
        }

        private string GetSamlTokenByGss(ServiceGateway service, ServerDto serverDto)
        {
            var gssTicket = KerberosHelper.GetKerberosContext(txtGssUsername.Text, txtGssPassword.Text, txtGssDomain.Text, txtGssSpn.Text);
            var token = service.SamlTokenService.GetSamlTokenFromGss(serverDto, gssTicket);
            return token;
        }

        private void GetSamlTokenByUserCredentials(ServiceGateway service, ServerDto serverDto)
        {
            var loginDto = new LoginDto() { DomainName = txtDomainName.Text, TenantName = txtDefaultTenant.Text, User = txtUser.Text, Pass = txtPass.Text };
            var authToken = service.SamlTokenService.Authenticate(serverDto, loginDto, Constants.ClientId);
            PopulateToken(authToken);
        }

        private void PopulateToken(AuthTokenDto authToken)
        {
            if (cbSaml.Checked)
            {
                var bytes = Convert.FromBase64String(authToken.Token.AccessToken);
                var value = System.Text.Encoding.Default.GetString(bytes);
                txtSamlToken.Text = value;
            }
            else
            {
                try
                {
                    txtIdToken.Text = JwtHelper.Decode(authToken.Token.IdToken);
                }
                catch
                {
                    txtIdToken.Text = authToken.Token.IdToken;
                }

                try
                {
                    txtAccessToken.Text = JwtHelper.Decode(authToken.Token.AccessToken);
                }
                catch
                {
                    txtAccessToken.Text = authToken.Token.AccessToken;
                }

                try
                {
                    txtRefreshToken.Text = JwtHelper.Decode(authToken.Token.RefreshToken);
                }
                catch
                {
                    txtRefreshToken.Text = authToken.Token.RefreshToken;
                }
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
            if (string.IsNullOrEmpty(txtPort.Text) || !int.TryParse(txtPort.Text, out port))
            {
                MMCDlgHelper.ShowWarning("Please enter a valid port");
                return false;
            }

            if (string.IsNullOrEmpty(txtDefaultTenant.Text))
            {
                MMCDlgHelper.ShowWarning("Please enter a valid Default Tenant name");
                return false;
            }
            if (loginTab.SelectedTab == tabUser)
            {
                if (string.IsNullOrEmpty(txtUser.Text))
                {
                    MMCDlgHelper.ShowWarning("Enter a valid username.");
                    return false;
                }
                if (string.IsNullOrEmpty(txtPass.Text))
                {
                    MMCDlgHelper.ShowWarning("Enter a valid password.");
                    return false;
                }

                if (string.IsNullOrEmpty(txtDomainName.Text))
                {
                    MMCDlgHelper.ShowWarning("Enter a valid domain name.");
                    return false;
                }
            }
            else
                if (loginTab.SelectedTab == tabGssTicket)
                {
                    if (string.IsNullOrEmpty(txtGssUsername.Text))
                    {
                        MMCDlgHelper.ShowWarning("Enter a valid GSS username.");
                        return false;
                    }
                    if (string.IsNullOrEmpty(txtGssPassword.Text))
                    {
                        MMCDlgHelper.ShowWarning("Enter a valid GSS password.");
                        return false;
                    }

                    if (string.IsNullOrEmpty(txtGssDomain.Text))
                    {
                        MMCDlgHelper.ShowWarning("Enter a valid GSS domain name.");
                        return false;
                    }
                    if (string.IsNullOrEmpty(txtGssSpn.Text))
                    {
                        MMCDlgHelper.ShowWarning("Enter a valid GSS SPN.");
                        return false;
                    }
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
            e.Cancel = !close;
        }

        private void TokenWizard_FormClosing(object sender, FormClosingEventArgs e)
        {
            close = true;
        }
       
        private void cbSaml_CheckedChanged(object sender, EventArgs e)
        {
            SetControlVisibility();
            UpdateUrlPreview();
        }

        private void SetControlVisibility()
        {
            lblStsUri.Visible = cbSaml.Checked;
            txtStsUri.Visible = cbSaml.Checked;
            lblIdTOken.Visible = !cbSaml.Checked;
            lblAccessToken.Visible = !cbSaml.Checked;
            lblRefreshToken.Visible = !cbSaml.Checked;
            txtIdToken.Visible = !cbSaml.Checked;
            txtAccessToken.Visible = !cbSaml.Checked;
            txtRefreshToken.Visible = !cbSaml.Checked;
            lblSamlToken.Visible = cbSaml.Checked;            
            txtSamlToken.Visible = cbSaml.Checked;
            tabTokenFile.Visible = cbSaml.Checked;            
            tabSolutionUser.Text = cbSaml.Checked ? "Solution User Certificate" : "Certificate";
            btnTokPKey.Visible = cbSaml.Checked;
            txtTokPkey.Visible = cbSaml.Checked;
            txtTokCert.Visible = cbSaml.Checked;
            btnTokCert.Visible = cbSaml.Checked;
            txtTokenFilePath.Visible = cbSaml.Checked;
            btnTokFilePath.Visible = cbSaml.Checked;
            lblTokFilePath.Visible = cbSaml.Checked;
            lblTokPKey.Visible = cbSaml.Checked;
            lblTokCert.Visible = cbSaml.Checked;
        }

        private void txtStsUri_TextChanged(object sender, EventArgs e)
        {
            UpdateUrlPreview();
        }      

        private void btnSelectSertificate_Click(object sender, EventArgs e)
        {
            using (var ofd = new OpenFileDialog())
            {

                ofd.Filter = MMCUIConstants.CERT_FILTER;
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    txtCertificate.Text = ofd.FileName;
                }
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            using (var ofd = new OpenFileDialog())
            {
                ofd.Filter = MMCUIConstants.SAML_TOKEN_FILTER;
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    txtTokenFilePath.Text = ofd.FileName;
                }
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            using (var ofd = new OpenFileDialog())
            {
                ofd.Filter = MMCUIConstants.CERT_FILTER;
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    txtTokCert.Text = ofd.FileName;
                }
            }
        }

        private void TokenWizard_Load(object sender, EventArgs e)
        {           
        }

        private void btnPrivateKey_Click(object sender, EventArgs e)
        {
            using (var ofd = new OpenFileDialog())
            {
                ofd.Filter = MMCUIConstants.PRI_KEY_FILTER;
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    txtPrivateKey.Text = ofd.FileName;
                }
            }
        }

        private void btnTokPKey_Click(object sender, EventArgs e)
        {
            using (var ofd = new OpenFileDialog())
            {
                ofd.Filter = MMCUIConstants.PRI_KEY_FILTER;
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    txtTokPkey.Text = ofd.FileName;
                }
            }
        }
    }
}

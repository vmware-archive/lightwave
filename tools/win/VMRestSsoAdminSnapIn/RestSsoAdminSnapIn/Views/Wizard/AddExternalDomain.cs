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
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service.IdentityProvider;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Security.Certificate;
using VMwareMMCIDP.UI.Common.Utilities;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views.Wizard
{
    public partial class AddExternalDomain : Form, IView
    {
        private Steps _current;
        private string _domainName;
        private bool _createNew;
        private AdvancedExternalDomain advancedSettings;

        /* External Domain constants */
        private const string AD_WIN_AUTH_TITLE = "External Domain (AD using Windows Integrated Auth)";
        private const string AD_AS_LDAP_TITLE = "External Domain (AD as an LDAP server)";
        private const string OPEN_LDAP_TITLE = "External Domain (Open LDAP server)";
        private const string NEW_EXTERNAL_DOMAIN_TITLE = "Add External Domain";

        public IdentityProviderDto IdentityProviderDto;
        public ServerDto ServerDto;
        public string TenantName;
        public AddExternalDomain()
        {
            InitializeComponent();
        }

        private void button2_Click(object sender, EventArgs e)
        {

        }

        private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {

        }

        private void AddExternalDomain_Load(object sender, EventArgs e)
        {
            advancedSettings = new AdvancedExternalDomain();
            _createNew = IdentityProviderDto == null;
            if (_createNew)
            {
                _current = Steps.One;
                rdoADWindowsAuth.Checked = false;
                rdoADLdap.Checked = true;
                rdoAnyDomain.Checked = true;
                IdentityProviderDto = new IdentityProviderDto();
                this.Text = NEW_EXTERNAL_DOMAIN_TITLE;
            }
            else
            {
                DtoToView();
                _current = rdoADWindowsAuth.Checked ? Steps.Four : Steps.Two;
                this.Text = rdoADWindowsAuth.Checked ? AD_WIN_AUTH_TITLE :
                    rdoADLdap.Checked ? AD_AS_LDAP_TITLE :
                    OPEN_LDAP_TITLE;
            }

            button11.Enabled = false;
            button1.Enabled = false;
            button2.Enabled = false;

            StepShow();
        }

        private void DtoToView()
        {
            SetSourceType(IdentityProviderDto.Type);
            rdoADWindowsAuth.Enabled = false;
            rdoADLdap.Enabled = false;
            rdoopenLdap.Enabled = false;
            txtDomainName.ReadOnly = true;
            txtDomainName.Text = IdentityProviderDto.Name;
            txtDomainAlias.Text = IdentityProviderDto.Alias;
            txtUserDN.Text = IdentityProviderDto.UserBaseDN;
            txtGroupDN.Text = IdentityProviderDto.GroupBaseDN;
            txtFriendlyName.Text = IdentityProviderDto.FriendlyName;
            if (!IdentityProviderDto.SiteAffinityEnabled || rdoopenLdap.Checked)
            {
                txtPrimaryConnectionString.Text = IdentityProviderDto.ConnectionStrings[0];

                if (IdentityProviderDto.ConnectionStrings.Count > 1)
                    txtSecondaryConnectionString.Text = IdentityProviderDto.ConnectionStrings[1];
                else
                    txtSecondaryConnectionString.Text = string.Empty;
            }
            else
            {
                txtConnectionString.Text = IdentityProviderDto.ConnectionStrings[0];
            }
            if (IdentityProviderDto.Certificates == null)
                IdentityProviderDto.Certificates = new List<CertificateDto>();
            if (IdentityProviderDto.Certificates.Count > 0)
            {
                foreach (var xcert in IdentityProviderDto.Certificates)
                {
                    var base64Value = CertificateHelper.PemToBase64EncodedString(xcert.Encoded);
                    var cert = CertificateHelper.GetX509Certificate2FromString(base64Value);
                    var thumbprint = cert.GetFormattedThumbPrint();
                    var values = new string[] { cert.Subject, cert.Issuer, cert.GetEffectiveDateString(), cert.GetExpirationDateString(), thumbprint };
                    var lst = new ListViewItem(values) { Tag = new CertificateDto { Encoded = cert.ExportToPem(), Chain = thumbprint } };
                    lstCertificateChain.Items.Add(lst);
                }
            }
            rdoAnyDomain.Checked = rdoADLdap.Checked && IdentityProviderDto.SiteAffinityEnabled;
            rdoSpecificDomain.Checked = rdoADLdap.Checked && !IdentityProviderDto.SiteAffinityEnabled;
            chkProtect.Checked = rdoAnyDomain.Checked && IdentityProviderDto.ConnectionStrings[0].StartsWith("ldaps");

            radioButton1.Checked = IdentityProviderDto.UserMachineAccount;
            radioButton2.Checked = !IdentityProviderDto.UserMachineAccount;
            txtUsername.Text = IdentityProviderDto.Username;
            txtPassword.Text = IdentityProviderDto.Password;
            txtSPN.Text = IdentityProviderDto.ServicePrincipalName;
            advancedSettings.IdentityProviderDto = IdentityProviderDto;
        }

        private void linkLabel2_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {

        }

        private void linkLabel3_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {

        }

        private void linkLabel4_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {

        }
        private void linkLabel5_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {

        }

        private void linkLabel6_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {

        }

        private void button3_Click(object sender, EventArgs e)
        {
            _current = Steps.One;
            StepShow();
        }

        private void btnNext_Click(object sender, EventArgs e)
        {
            if (ValidateStep1())
            {
                _current = rdoADWindowsAuth.Checked ? Steps.Four : Steps.Two;
                StepShow();
            }
        }

        private bool ValidateStep1()
        {
            var retVal = true;
            if (rdoADWindowsAuth.Checked)
            {
                retVal = false;
                _domainName = string.Empty;
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto, TenantName);
                ActionHelper.Execute(delegate()
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(ServerDto.ServerName);
                    var adJoinInfoDto = service.Adf.GetActiveDirectory(ServerDto, auth.Token);
                    if (adJoinInfoDto != null && adJoinInfoDto.JoinStatus == "DOMAIN")
                    {
                        _domainName = adJoinInfoDto.Name;
                        retVal = true;
                    }
                    else
                    {
                        var result = MMCDlgHelper.ShowQuestion("The server needs to be joined to the Active Directory, Do you wish to join now?");
                        if (result)
                        {
                            var form = new JoinActiveDirectoryView(ServerDto);
                            form.ShowDialog();
                        }
                        retVal = false;
                    }
                }, auth);
            }
            return retVal;
        }

        private bool IsLdaps()
        {
            return ((rdoSpecificDomain.Checked || rdoopenLdap.Checked) && (txtPrimaryConnectionString.Text.StartsWith("ldaps") || txtSecondaryConnectionString.Text.StartsWith("ldaps"))) ||
                (rdoAnyDomain.Checked && chkProtect.Checked);
        }
        private void button4_Click(object sender, EventArgs e)
        {
            if (ValidateStep2())
            {
                if (IsLdaps())
                {
                    _current = Steps.Three;
                }
                else
                {
                    _current = Steps.Four;
                }
                StepShow();
            }
        }

        private bool ValidateStep2()
        {
            if (string.IsNullOrEmpty(txtFriendlyName.Text.Trim()) || txtFriendlyName.Text.Trim() == "Contoso Domain")
            {
                MMCDlgHelper.ShowWarning("Domain friendly name cannot be left empty or default");
                return false;
            }
            if (string.IsNullOrEmpty(txtDomainName.Text.Trim()) || txtDomainName.Text.Trim() == "contoso.com")
            {
                MMCDlgHelper.ShowWarning("Domain name cannot be left empty or default");
                return false;
            }
            else if (string.IsNullOrEmpty(txtDomainAlias.Text.Trim()) || txtDomainAlias.Text.Trim() == "Contoso")
            {
                MMCDlgHelper.ShowWarning("Domain alias cannot be left empty or default");
                return false;
            }
            else if (string.IsNullOrEmpty(txtUserDN.Text.Trim()) || txtUserDN.Text.Trim() == "CN=users, CN=Configuration, DC=contoso, DC=com")
            {
                MMCDlgHelper.ShowWarning("Users base DN cannot be left empty or default");
                return false;
            }
            else if (string.IsNullOrEmpty(txtGroupDN.Text.Trim()) || txtDomainAlias.Text.Trim() == "CN=groups, CN=Configuration, DC=contoso, DC=com")
            {
                MMCDlgHelper.ShowWarning("Groups base DN cannot be left empty or default");
                return false;
            }
            else if ((rdoopenLdap.Checked || rdoADLdap.Checked && rdoSpecificDomain.Checked)
                    && (string.IsNullOrEmpty(txtPrimaryConnectionString.Text) || txtPrimaryConnectionString.Text.Trim() == "ldap://constoso.com"))
            {
                MMCDlgHelper.ShowWarning("Primary URL cannot be left empty or default");
                return false;
            }
            return true;
        }

        private void button6_Click(object sender, EventArgs e)
        {
            _current = Steps.Two;
            StepShow();
        }

        private void button7_Click(object sender, EventArgs e)
        {
            if (ValidateStep3())
            {
                _current = Steps.Four;
                StepShow();
            }
        }

        private bool ValidateStep3()
        {
            if (lstCertificateChain.Items.Count <= 0)
            {
                MMCDlgHelper.ShowWarning("Please add atleast one certificate to the chain");
                return false;
            }
            return true;
        }

        private void button8_Click(object sender, EventArgs e)
        {
            _current = rdoADWindowsAuth.Checked ? Steps.One : IsLdaps() ? Steps.Three : Steps.Two;
            StepShow();
        }

        private void button9_Click(object sender, EventArgs e)
        {
            if (ValidateStep4())
            {
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto, TenantName);
                ActionHelper.Execute(delegate()
                {
                    var provider = ViewToDto();
                    if (_createNew)
                    {
                        var service = ScopeNodeExtensions.GetServiceGateway(ServerDto.ServerName);
                        var result = service.IdentityProvider.Create(ServerDto, TenantName, provider, auth.Token);
                        if (result != null)
                        {
                            MMCDlgHelper.ShowWarning(string.Format("External domain {0} created successfully", result.Name));
                        }
                    }
                    else
                    {
                        var service = ScopeNodeExtensions.GetServiceGateway(ServerDto.ServerName);
                        var result = service.IdentityProvider.Update(ServerDto, TenantName, provider, auth.Token);
                        if (result != null)
                        {
                            MMCDlgHelper.ShowWarning(string.Format("External domain {0} updated successfully", result.Name));
                        }
                    }
                    this.Close();
                }, auth);
            }
        }

        private IdentityProviderDto ViewToDto()
        {
            var isAd = rdoADWindowsAuth.Checked;
            var providerDto = new IdentityProviderDto
            {
                DomainType = DomainType.EXTERNAL_DOMAIN.ToString(),
                Name = isAd ? radioButton1.Checked ? _domainName : txtSPN.Text.Substring(txtSPN.Text.LastIndexOf("/") + 1) : txtDomainName.Text,
                Alias = isAd ? null : txtDomainAlias.Text,
                FriendlyName = isAd ? null : txtFriendlyName.Text,
                Type = GetSourceType(),
                AuthenticationType = isAd ? "USE_KERBEROS" : "PASSWORD",
                Username = isAd && radioButton1.Checked ? null : txtUsername.Text,
                Password = isAd && radioButton1.Checked ? null : txtPassword.Text,
                UserMachineAccount = isAd && radioButton1.Checked,
                ServicePrincipalName = isAd && radioButton1.Checked ? null : txtSPN.Text,
                UserBaseDN = isAd ? null : txtUserDN.Text,
                GroupBaseDN = isAd ? null : txtGroupDN.Text,
                ConnectionStrings = GetConnectionString(isAd),
                Certificates = isAd ? null : GetCertificates()
            };

            if (advancedSettings.IdentityProviderDto != null)
            {
                if (advancedSettings.IdentityProviderDto.Schema != null)
                    providerDto.Schema = new Dictionary<string, SchemaObjectMappingDto>(advancedSettings.IdentityProviderDto.Schema);
                if (advancedSettings.IdentityProviderDto.AttributesMap != null)
                    providerDto.AttributesMap = new Dictionary<string, string>(advancedSettings.IdentityProviderDto.AttributesMap);
                providerDto.SiteAffinityEnabled = rdoADLdap.Checked && rdoAnyDomain.Checked;
                providerDto.BaseDnForNestedGroupsEnabled = advancedSettings.IdentityProviderDto.BaseDnForNestedGroupsEnabled;
                providerDto.DirectGroupsSearchEnabled = advancedSettings.IdentityProviderDto.DirectGroupsSearchEnabled;
                providerDto.MatchingRuleInChainEnabled = advancedSettings.IdentityProviderDto.MatchingRuleInChainEnabled;
            }

            return providerDto;
        }

        private List<string> GetConnectionString(bool isAd)
        {
            var ldap = (rdoopenLdap.Checked || rdoADLdap.Checked && rdoSpecificDomain.Checked);
            var isEmpty = (string.IsNullOrEmpty(txtSecondaryConnectionString.Text) || txtSecondaryConnectionString.Text.Trim() == "ldap://contoso.com:389 or ldaps://contoso.com:3268");
            return isAd ? null : ldap ? (isEmpty ?
                new List<string> { txtPrimaryConnectionString.Text } :
                new List<string> { txtPrimaryConnectionString.Text, txtSecondaryConnectionString.Text }) :
                new List<string> { txtConnectionString.Text };
        }

        private List<CertificateDto> GetCertificates()
        {
            var certificates = new List<CertificateDto>();

            foreach (ListViewItem item in lstCertificateChain.Items)
            {
                certificates.Add((CertificateDto)item.Tag);
            }
            return certificates;
        }

        private string GetSourceType()
        {
            if (rdoADWindowsAuth.Checked) return "IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY";
            if (rdoADLdap.Checked) return "IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING";
            if (rdoopenLdap.Checked) return "IDENTITY_STORE_TYPE_LDAP";
            return string.Empty;
        }

        private void SetSourceType(string sourceType)
        {
            if (sourceType == "IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY") rdoADWindowsAuth.Checked = true;
            if (sourceType == "IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING") rdoADLdap.Checked = true;
            if (sourceType == "IDENTITY_STORE_TYPE_LDAP") rdoopenLdap.Checked = true;
        }

        private void button10_Click(object sender, EventArgs e)
        {
            advancedSettings.IsAD = rdoADWindowsAuth.Checked;
            advancedSettings.ShowDialog();
        }

        private void button5_Click(object sender, EventArgs e)
        {
            if (ValidateStep4())
            {
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto, TenantName);
                // Call probe
                ActionHelper.Execute(delegate()
                {
                    var provider = ViewToDto();
                    var service = ScopeNodeExtensions.GetServiceGateway(ServerDto.ServerName);
                    var result = service.IdentityProvider.Probe(ServerDto, TenantName, provider, auth.Token);
                    if (result != null)
                        MMCDlgHelper.ShowInformation("Test connection successful");
                }, auth);
            }
        }

        private bool ValidateStep4()
        {
            if (radioButton2.Visible && radioButton2.Checked)
            {
                if (string.IsNullOrEmpty(txtSPN.Text.Trim()) || txtSPN.Text.Trim() == "sts/contoso.com")
                {
                    MMCDlgHelper.ShowWarning("SPN cannot be empty or default");
                    return false;
                }
            }

            if (!radioButton2.Visible || (radioButton2.Visible && radioButton2.Checked))
            {
                if (string.IsNullOrEmpty(txtUsername.Text.Trim()) || txtUsername.Text.Trim() == "CN=Administrator,CN=users,DC=contoso,DC=com")
                {
                    MMCDlgHelper.ShowWarning("Username cannot be empty or default");
                    return false;
                }
                else if (string.IsNullOrEmpty(txtPassword.Text.Trim()))
                {
                    MMCDlgHelper.ShowWarning("Password cannot be empty or default");
                    return false;
                }
            }
            return true;
        }
        private void button1_Click(object sender, EventArgs e)
        {
            ImportCertificatesFromDomainController(txtPrimaryConnectionString.Text);
        }

        private void button2_Click_1(object sender, EventArgs e)
        {
            ImportCertificatesFromDomainController(txtSecondaryConnectionString.Text);
        }

        private void ImportCertificatesFromDomainController(string connection)
        {
            try
            {
                var xcert = LdapSecureConnectionCertificateFetcher.FetchServerCertificate(connection);
                var cert = new X509Certificate2(xcert);
                var thumbprint = cert.GetFormattedThumbPrint();
                foreach (ListViewItem item in lstCertificateChain.Items)
                {
                    if (((CertificateDto)item.Tag).Chain == thumbprint)
                    {
                        MMCDlgHelper.ShowWarning("Certificate with the same fingerprint already exists");
                        return;
                    }
                }
                var values = new string[] { cert.Subject, cert.Issuer, cert.GetEffectiveDateString(), cert.GetExpirationDateString(), thumbprint };
                var lst = new ListViewItem(values) { Tag = new CertificateDto { Encoded = cert.ExportToPem(), Chain = thumbprint } };
                lstCertificateChain.Items.Add(lst);
                MMCDlgHelper.ShowWarning(string.Format("Certificate with subject {0} imported successfully", cert.Subject));
            }
            catch
            {
                // do nothing
            }
        }

        private void btnAddCert_Click(object sender, EventArgs e)
        {
            X509Certificate2 cert;
            using (var ofd = new OpenFileDialog())
            {
                ofd.Filter = "Certificate Files (*.cer)|*.cer|All Files (*.*)|*.*";
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    try
                    {
                        cert = new X509Certificate2();
                        cert.Import(ofd.FileName);
                    }
                    catch
                    {
                        MMCDlgHelper.ShowWarning("Invalid certificate");
                        return;
                    }
                    var thumbprint = cert.GetFormattedThumbPrint();
                    foreach (ListViewItem item in lstCertificateChain.Items)
                    {
                        if (((CertificateDto)item.Tag).Chain == thumbprint)
                        {
                            MMCDlgHelper.ShowWarning("Certificate with the same fingerprint already exists");
                            return;
                        }
                    }

                    var values = new string[] { cert.Subject, cert.Issuer, cert.GetEffectiveDateString(), cert.GetExpirationDateString(), thumbprint };
                    var lst = new ListViewItem(values) { Tag = new CertificateDto { Encoded = cert.ExportToPem(), Chain = thumbprint } };
                    lstCertificateChain.Items.Add(lst);
                }
            }
        }

        private void btnRemoveCert_Click(object sender, EventArgs e)
        {
            if (lstCertificateChain.SelectedItems.Count > -1)
                lstCertificateChain.Items.RemoveAt(lstCertificateChain.SelectedIndices[0]);
        }


        private void StepShow()
        {
            pnlStep1.Visible = _current == Steps.One;
            pnlStep2.Visible = _current == Steps.Two;
            pnlStep3.Visible = _current == Steps.Three;
            pnlStep4.Visible = _current == Steps.Four;
            if (pnlStep2.Visible)
            {
                rdoSpecificDomain.Visible = rdoADLdap.Checked;
                pnlProtect.Visible = rdoADLdap.Checked;
                if (!rdoAnyDomain.Checked && !rdoSpecificDomain.Checked)
                    rdoSpecificDomain.Checked = true;
            }
            if (pnlStep4.Visible)
            {
                radioButton1_CheckedChanged(this, EventArgs.Empty);
            }
            button3.Enabled = (!_createNew && !rdoADWindowsAuth.Checked && _current > Steps.Two) ||
                (_createNew && _current != Steps.One);
        }

        public enum Steps
        {
            One,
            Two,
            Three,
            Four
        }

        private void button11_Click(object sender, EventArgs e)
        {

        }

        private void txtDomainName_Enter(object sender, EventArgs e)
        {
            if (txtDomainName.Text.Equals("contoso.com", StringComparison.InvariantCulture)
              && txtDomainName.ForeColor == Color.Gray)
            {
                txtDomainName.Text = string.Empty;
                txtDomainName.ForeColor = Color.Black;
            }
        }

        private void button12_Click(object sender, EventArgs e)
        {
            var _help = new HelpDialog();
            _help.Location = pnlStep1.PointToScreen(button12.Location);
            _help.Title = "Help";
            _help.Content = "Choose this option if the users will be authenticated automatically using the client integration plugin";
            _help.ShowDialog();
        }

        private void button13_Click(object sender, EventArgs e)
        {
            var _help = new HelpDialog();
            _help.Location = pnlStep1.PointToScreen(button13.Location);
            _help.Title = "Help";
            _help.Content = "Choose this option if the users will authenticate to Active Directory using LDAP";
            _help.ShowDialog();
        }

        private void button14_Click(object sender, EventArgs e)
        {
            var _help = new HelpDialog();
            _help.Location = pnlStep1.PointToScreen(button14.Location);
            _help.Title = "Help";
            _help.Content = "Choose this option if you need to connect to a generic LDAP server";
            _help.ShowDialog();
        }

        private void button16_Click(object sender, EventArgs e)
        {
            var _help = new HelpDialog();
            _help.Location = pnlStep2.PointToScreen(button16.Location);
            _help.Title = "Help";
            _help.Content = "This will be the suffix of User Principal Names (UPN) by which the users will authenticate from this identity source";
            _help.ShowDialog();
        }

        private void button17_Click(object sender, EventArgs e)
        {
            var _help = new HelpDialog();
            _help.Location = pnlStep4.PointToScreen(button17.Location);
            _help.Title = "Help";
            _help.Content = "These credentials will be used to fetch the user and group information from the domain";
            _help.ShowDialog();
        }

        private void rdoADWindowsAuth_CheckedChanged(object sender, EventArgs e)
        {
            SetADControls();
        }

        private void rdoADLdap_CheckedChanged(object sender, EventArgs e)
        {
            SetADControls();
        }

        private void SetADControls()
        {
            radioButton1.Checked = rdoADWindowsAuth.Checked;
            radioButton1.Visible = rdoADWindowsAuth.Checked;
            radioButton2.Visible = rdoADWindowsAuth.Checked;
            lblSPN.Visible = rdoADWindowsAuth.Checked;
            txtSPN.Visible = rdoADWindowsAuth.Checked;
            button5.Visible = !rdoADWindowsAuth.Checked;
            button10.Visible = !rdoADWindowsAuth.Checked;
        }

        private void txtPrimaryConnectionString_TextChanged(object sender, EventArgs e)
        {
            button1.Enabled = txtPrimaryConnectionString.Text.StartsWith("ldaps://");
        }

        private void txtSecondaryConnectionString_TextChanged(object sender, EventArgs e)
        {
            button2.Enabled = txtSecondaryConnectionString.Text.StartsWith("ldaps://");
        }

        private void txtConnectionString_TextChanged(object sender, EventArgs e)
        {

        }

        private void SetImportButtonStatus()
        {
            button1.Enabled = rdoSpecificDomain.Checked && txtPrimaryConnectionString.Text.StartsWith("ldaps://");
            button2.Enabled = rdoSpecificDomain.Checked && txtSecondaryConnectionString.Text.StartsWith("ldaps://");
        }
        private void pnlStep2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            SetSpnUsernamePassword();
        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            SetSpnUsernamePassword();
        }

        private void SetSpnUsernamePassword()
        {
            var show = (rdoopenLdap.Checked || rdoADLdap.Checked || (radioButton2.Visible && radioButton2.Checked));
            txtSPN.Enabled = show;
            txtUsername.Enabled = show;
            txtPassword.Enabled = show;
        }
        private void rdoopenLdap_CheckedChanged(object sender, EventArgs e)
        {
            SetADControls();
        }

        private void txtDomainName_TextChanged(object sender, EventArgs e)
        {
            SetConnectionString();
        }

        private void SetConnectionString()
        {
            if (rdoAnyDomain.Checked)
                txtConnectionString.Text = (chkProtect.Checked ? "ldaps://" : "ldap://") + txtDomainName.Text;
        }

        private void chkProtect_CheckedChanged(object sender, EventArgs e)
        {
            SetConnectionString();
        }

        private void rdoAnyDomain_CheckedChanged(object sender, EventArgs e)
        {
            SetConnectionString();
            SetImportButtonStatus();
        }

        public IDataContext DataContext
        {
            get { return IdentityProviderDto; }
        }

        private void button15_Click(object sender, EventArgs e)
        {
            if (lstCertificateChain.SelectedIndices.Count > 0)
            {
                var certDto = (CertificateDto)lstCertificateChain.Items[lstCertificateChain.SelectedIndices[0]].Tag;
                var base64 = CertificateHelper.PemToBase64EncodedString(certDto.Encoded);
                CertificateHelper.ShowX509Certificate(base64);
            }
        }

        private void lstCertificateChain_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveCert.Enabled = lstCertificateChain.SelectedIndices.Count > 0;
        }

        private void rdoSpecificDomain_CheckedChanged(object sender, EventArgs e)
        {
            SetImportButtonStatus();
        }

        private void button18_Click(object sender, EventArgs e)
        {
            var _help = new HelpDialog();
            _help.Location = pnlStep1.PointToScreen(button18.Location);
            _help.Title = "Help";
            _help.Content = "Contains the list of certificates added either by establishing a connection with the target server or manually. To enable connection to any domain controller, add a valid LDAP certificate for each of the domain controllers in the domain.";
            _help.ShowDialog();
        }

        private void txtSPN_Enter(object sender, EventArgs e)
        {
            if (txtSPN.Text.Equals("sts/contoso.com", StringComparison.InvariantCulture)
                && txtFriendlyName.ForeColor == Color.Gray)
            {
                txtSPN.Text = string.Empty;
                txtSPN.ForeColor = Color.Black;
            }
        }

        private void txtSPN_Leave(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(txtSPN.Text))
            {
                txtSPN.Text = "sts/contoso.com";
                txtSPN.ForeColor = Color.Gray;
            }
        }

        private void txtFriendlyName_Leave(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(txtFriendlyName.Text))
            {
                txtFriendlyName.Text = "Contoso Domain";
                txtFriendlyName.ForeColor = Color.Gray;
            }
        }

        private void txtFriendlyName_Enter(object sender, EventArgs e)
        {
            if (txtFriendlyName.Text.Equals("Contoso Domain", StringComparison.InvariantCulture)
                && txtFriendlyName.ForeColor == Color.Gray)
            {
                txtFriendlyName.Text = string.Empty;
                txtFriendlyName.ForeColor = Color.Black;
            }
        }

        private void txtUsername_Leave(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(txtUsername.Text))
            {
                txtUsername.Text = "CN=Administrator,CN=users,DC=contoso,DC=com";
                txtUsername.ForeColor = Color.Gray;
            }
        }

        private void txtUsername_Enter(object sender, EventArgs e)
        {
            if (txtUsername.Text.Equals("CN=Administrator,CN=users,DC=contoso,DC=com", StringComparison.InvariantCulture)
               && txtUsername.ForeColor == Color.Gray)
            {
                txtUsername.Text = string.Empty;
                txtUsername.ForeColor = Color.Black;
            }
        }

        private void txtDomainName_Leave(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(txtDomainName.Text))
            {
                txtDomainName.Text = "contoso.com";
                txtDomainName.ForeColor = Color.Gray;
            }
        }

        private void txtDomainAlias_Enter(object sender, EventArgs e)
        {
            if (txtDomainAlias.Text.Equals("Contoso", StringComparison.InvariantCulture)
               && txtDomainAlias.ForeColor == Color.Gray)
            {
                txtDomainAlias.Text = string.Empty;
                txtDomainAlias.ForeColor = Color.Black;
            }
        }

        private void txtDomainAlias_Leave(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(txtDomainAlias.Text))
            {
                txtDomainAlias.Text = "Contoso";
                txtDomainAlias.ForeColor = Color.Gray;
            }
        }

        private void txtUserDN_Enter(object sender, EventArgs e)
        {
            if (txtUserDN.Text.Equals("CN=users, CN=Configuration, DC=contoso, DC=com", StringComparison.InvariantCulture)
               && txtUserDN.ForeColor == Color.Gray)
            {
                txtUserDN.Text = string.Empty;
                txtUserDN.ForeColor = Color.Black;
            }
        }

        private void txtUserDN_Leave(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(txtUserDN.Text))
            {
                txtUserDN.Text = "CN=users, CN=Configuration, DC=contoso, DC=com";
                txtUserDN.ForeColor = Color.Gray;
            }
        }

        private void txtGroupDN_Enter(object sender, EventArgs e)
        {
            if (txtGroupDN.Text.Equals("CN=groups, CN=Configuration, DC=contoso, DC=com", StringComparison.InvariantCulture)
               && txtGroupDN.ForeColor == Color.Gray)
            {
                txtGroupDN.Text = string.Empty;
                txtGroupDN.ForeColor = Color.Black;
            }
        }

        private void txtGroupDN_Leave(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(txtGroupDN.Text))
            {
                txtGroupDN.Text = "CN=groups, CN=Configuration, DC=contoso, DC=com";
                txtGroupDN.ForeColor = Color.Gray;
            }
        }

        private void txtPrimaryConnectionString_Enter(object sender, EventArgs e)
        {
            if (txtPrimaryConnectionString.Text.Equals("ldap://contoso.com:389 or ldaps://contoso.com:3268", StringComparison.InvariantCulture)
               && txtPrimaryConnectionString.ForeColor == Color.Gray)
            {
                txtPrimaryConnectionString.Text = string.Empty;
                txtPrimaryConnectionString.ForeColor = Color.Black;
            }
        }

        private void txtPrimaryConnectionString_Leave(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(txtPrimaryConnectionString.Text))
            {
                txtPrimaryConnectionString.Text = "ldap://contoso.com:389 or ldaps://contoso.com:3268";
                txtPrimaryConnectionString.ForeColor = Color.Gray;
            }
        }

        private void txtSecondaryConnectionString_Enter(object sender, EventArgs e)
        {
            if (txtSecondaryConnectionString.Text.Equals("ldap://contoso.com:389 or ldaps://contoso.com:3268", StringComparison.InvariantCulture)
               && txtSecondaryConnectionString.ForeColor == Color.Gray)
            {
                txtSecondaryConnectionString.Text = string.Empty;
                txtSecondaryConnectionString.ForeColor = Color.Black;
            }
        }

        private void txtSecondaryConnectionString_Leave(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(txtSecondaryConnectionString.Text))
            {
                txtSecondaryConnectionString.Text = "ldap://contoso.com:389 or ldaps://contoso.com:3268";
                txtSecondaryConnectionString.ForeColor = Color.Gray;
            }
        }
    }
}

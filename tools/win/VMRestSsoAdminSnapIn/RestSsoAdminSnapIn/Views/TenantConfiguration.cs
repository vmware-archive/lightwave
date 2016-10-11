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
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Security.Certificate;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class TenantConfiguration : Form, IView
    {
        private readonly ServerDto _serverDto;
        private readonly string _tenantName;
        private TenantConfigurationDto _tenantConfigurationDto;

        public TenantConfiguration(ServerDto serverDto, string tenantName)
        {
            InitializeComponent();
            _serverDto = serverDto;
            _tenantName = tenantName;
        }
        private void RefreshView()
        {
            var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
            ActionHelper.Execute(delegate()
            {
                _tenantConfigurationDto = service.Tenant.GetConfig(_serverDto, _tenantName, auth.Token);
                if (_tenantConfigurationDto == null)
                {
                    MMCDlgHelper.ShowWarning("No configuration retrieved for the tenant");
                    Close();
                }
                else
                {
                    DtoToView();
                }
            }, auth);
        }
        private void btnApply_Click(object sender, EventArgs e)
        {
            var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
            ActionHelper.Execute(delegate()
           {
               ViewToDto();
               service.Tenant.UpdateConfig(_serverDto, _tenantName, _tenantConfigurationDto, auth.Token);
               service.Tenant.UpdatePasswordAndLockoutConfig(_serverDto, _tenantName, _tenantConfigurationDto, auth.Token);
               _tenantConfigurationDto = service.Tenant.GetConfig(_serverDto, _tenantName, auth.Token);
           }, auth);
        }

        private void ViewToDto()
        {
            _tenantConfigurationDto.LockoutPolicy.Description = txtLockoutDescription.Text;
            _tenantConfigurationDto.LockoutPolicy.FailedAttemptIntervalSec = long.Parse(nudLockoutFailedAttemptIntervalSecs.Text);
            _tenantConfigurationDto.LockoutPolicy.MaxFailedAttempts = int.Parse(nudLockoutMaxFailedAttempts.Text);
            _tenantConfigurationDto.LockoutPolicy.AutoUnlockIntervalSec = long.Parse(nudLockoutAutounlockIntervalSecs.Text);

            _tenantConfigurationDto.TokenPolicy.ClockToleranceMillis = long.Parse(nudTokenClockTolerenceMillis.Text);
            _tenantConfigurationDto.TokenPolicy.DelegationCount = int.Parse(nudTokenDelegationCount.Text);
            _tenantConfigurationDto.TokenPolicy.RenewCount = int.Parse(nudTokenRenewCount.Text);
            _tenantConfigurationDto.TokenPolicy.MaxBearerTokenLifeTimeMillis = long.Parse(nudTokenBearerMaxLifetimeMillis.Text);
            _tenantConfigurationDto.TokenPolicy.MaxBearerRefreshTokenLifeTimeMillis = long.Parse(nudTokenBearerMaxRefreshLifetimeMillis.Text);
            _tenantConfigurationDto.TokenPolicy.MaxHOKTokenLifeTimeMillis = long.Parse(nudTokenHokMaxLifetimeMillis.Text);
            _tenantConfigurationDto.TokenPolicy.MaxHOKRefreshTokenLifeTimeMillis = long.Parse(nudTokenHokMaxRefreshLifetimeMillis.Text);

            _tenantConfigurationDto.ProviderPolicy.DefaultProvider = txtProviderDefault.Text;
            _tenantConfigurationDto.ProviderPolicy.DefaultProviderAlias = string.IsNullOrWhiteSpace(txtProviderAlias.Text) ? null : txtProviderAlias.Text;
            _tenantConfigurationDto.ProviderPolicy.ProviderSelectionEnabled = cbProviderSelection.Checked;

            _tenantConfigurationDto.BrandPolicy.Name = txtBrandName.Text;
            _tenantConfigurationDto.BrandPolicy.LogonBannerContent = txtBrandLogonContent.Text;
            _tenantConfigurationDto.BrandPolicy.LogonBannerTitle = txtLogonTitle.Text;
            _tenantConfigurationDto.BrandPolicy.LogonBannerDisabled = chkDisableBanner.Checked;
            _tenantConfigurationDto.BrandPolicy.LogonBannerCheckboxEnabled = chkLogonBannerCheckbox.Checked;

            _tenantConfigurationDto.PasswordPolicy.Description = txtPasswordDescription.Text;
            _tenantConfigurationDto.PasswordPolicy.MaxIdenticalAdjacentCharacters = int.Parse(nudPasswordMaxIdenticalAdjacentChars.Text);
            _tenantConfigurationDto.PasswordPolicy.MaxLength = int.Parse(nudPasswordMaxLength.Text);
            _tenantConfigurationDto.PasswordPolicy.MinAlphabeticCount = int.Parse(nudPasswordMinAlphabeticCount.Text);
            _tenantConfigurationDto.PasswordPolicy.MinLength = int.Parse(nudPasswordMinLength.Text);
            _tenantConfigurationDto.PasswordPolicy.MinLowercaseCount = int.Parse(nudPasswordMinLowercaseCount.Text);
            _tenantConfigurationDto.PasswordPolicy.MinNumericCount = int.Parse(nudPasswordMinNumericCount.Text);
            _tenantConfigurationDto.PasswordPolicy.MinSpecialCharCount = int.Parse(nudPasswordMinSpecialCharacterCount.Text);
            _tenantConfigurationDto.PasswordPolicy.MinUppercaseCount = int.Parse(nudPasswordMinUppercaseCount.Text);
            _tenantConfigurationDto.PasswordPolicy.PasswordLifetimeDays = int.Parse(nudPasswordLifetimeDays.Text);
            _tenantConfigurationDto.PasswordPolicy.ProhibitedPreviousPasswordCount = int.Parse(nudPasswordProhibitedPreviousPasswordCount.Text);

            _tenantConfigurationDto.AuthenticationPolicy.PasswordBasedAuthentication = chkAuthPolicyPassword.Checked;
            _tenantConfigurationDto.AuthenticationPolicy.WindowsBasedAuthentication = chkAuthPolicyWindows.Checked;
            _tenantConfigurationDto.AuthenticationPolicy.CertificateBasedAuthentication = chkAuthPolicyCertificate.Checked;

            _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.FailOverToCrlEnabled = cbAuthenticationFailoverToCrl.Checked;
            _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.RevocationCheckEnabled = cbAuthenticationRevocationCheck.Checked;
            _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.OcspEnabled = cbAuthenticationOcsp.Checked;
            _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CrlDistributionPointUsageEnabled = cbAuthenticationCrlDistributionPointUsage.Checked;
            _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.OcspUrlOverride = string.IsNullOrEmpty(txtAuthenticationOcspUrlOverride.Text) ? null : txtAuthenticationOcspUrlOverride.Text;
            _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CrlDistributionPointOverride = string.IsNullOrEmpty(txtAuthenticationCrlDistributionPointOverride.Text) ? null : txtAuthenticationCrlDistributionPointOverride.Text;
            _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs = new System.Collections.Generic.List<string>();
            foreach (ListViewItem item in lstAuthenticationCertificatePolicyOids.Items)
            {
                _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs.Add((string)item.Tag);
            }

            _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates = new System.Collections.Generic.List<CertificateDto>();
            foreach (ListViewItem item in lstAuthenticationCertificates.Items)
            {
                _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates.Add((CertificateDto)item.Tag);
            }
        }
        public Dto.IDataContext DataContext
        {
            get { return _tenantConfigurationDto; }
        }
        private void TenantConfiguration_Load(object sender, EventArgs e)
        {
            RefreshView();
        }
        private void DtoToView()
        {
            if (_tenantConfigurationDto.LockoutPolicy == null)
                _tenantConfigurationDto.LockoutPolicy = new LockoutPolicyDto();
            txtLockoutDescription.Text = _tenantConfigurationDto.LockoutPolicy.Description;
            nudLockoutFailedAttemptIntervalSecs.Text = _tenantConfigurationDto.LockoutPolicy.FailedAttemptIntervalSec.ToString();
            nudLockoutMaxFailedAttempts.Text = _tenantConfigurationDto.LockoutPolicy.MaxFailedAttempts.ToString();
            nudLockoutAutounlockIntervalSecs.Text = _tenantConfigurationDto.LockoutPolicy.AutoUnlockIntervalSec.ToString();

            if (_tenantConfigurationDto.TokenPolicy == null)
                _tenantConfigurationDto.TokenPolicy = new TokenPolicyDto();
            nudTokenClockTolerenceMillis.Text = _tenantConfigurationDto.TokenPolicy.ClockToleranceMillis.ToString();
            nudTokenDelegationCount.Text = _tenantConfigurationDto.TokenPolicy.DelegationCount.ToString();
            nudTokenRenewCount.Text = _tenantConfigurationDto.TokenPolicy.RenewCount.ToString();
            nudTokenBearerMaxLifetimeMillis.Text = _tenantConfigurationDto.TokenPolicy.MaxBearerTokenLifeTimeMillis.ToString();
            nudTokenBearerMaxRefreshLifetimeMillis.Text = _tenantConfigurationDto.TokenPolicy.MaxBearerRefreshTokenLifeTimeMillis.ToString();
            nudTokenHokMaxLifetimeMillis.Text = _tenantConfigurationDto.TokenPolicy.MaxHOKTokenLifeTimeMillis.ToString();
            nudTokenHokMaxRefreshLifetimeMillis.Text = _tenantConfigurationDto.TokenPolicy.MaxHOKRefreshTokenLifeTimeMillis.ToString();

            if (_tenantConfigurationDto.ProviderPolicy == null)
                _tenantConfigurationDto.ProviderPolicy = new ProviderPolicyDto();
            txtProviderDefault.Text = _tenantConfigurationDto.ProviderPolicy.DefaultProvider;
            txtProviderAlias.Text = _tenantConfigurationDto.ProviderPolicy.DefaultProviderAlias;
            cbProviderSelection.Checked = _tenantConfigurationDto.ProviderPolicy.ProviderSelectionEnabled;

            if (_tenantConfigurationDto.BrandPolicy == null)
                _tenantConfigurationDto.BrandPolicy = new BrandPolicyDto();
            txtBrandName.Text = _tenantConfigurationDto.BrandPolicy.Name;
            txtBrandLogonContent.Text = _tenantConfigurationDto.BrandPolicy.LogonBannerContent;
            txtLogonTitle.Text = _tenantConfigurationDto.BrandPolicy.LogonBannerTitle;
            chkLogonBannerCheckbox.Checked = _tenantConfigurationDto.BrandPolicy.LogonBannerCheckboxEnabled;
            chkDisableBanner.Checked = _tenantConfigurationDto.BrandPolicy.LogonBannerDisabled;

            if (_tenantConfigurationDto.PasswordPolicy == null)
                _tenantConfigurationDto.PasswordPolicy = new PasswordPolicyDto();
            txtPasswordDescription.Text = _tenantConfigurationDto.PasswordPolicy.Description;
            nudPasswordMaxIdenticalAdjacentChars.Text = _tenantConfigurationDto.PasswordPolicy.MaxIdenticalAdjacentCharacters.ToString();
            nudPasswordMaxLength.Text = _tenantConfigurationDto.PasswordPolicy.MaxLength.ToString();
            nudPasswordMinAlphabeticCount.Text = _tenantConfigurationDto.PasswordPolicy.MinAlphabeticCount.ToString();
            nudPasswordMinLength.Text = _tenantConfigurationDto.PasswordPolicy.MinLength.ToString();
            nudPasswordMinLowercaseCount.Text = _tenantConfigurationDto.PasswordPolicy.MinLowercaseCount.ToString();
            nudPasswordMinNumericCount.Text = _tenantConfigurationDto.PasswordPolicy.MinNumericCount.ToString();
            nudPasswordMinSpecialCharacterCount.Text = _tenantConfigurationDto.PasswordPolicy.MinSpecialCharCount.ToString();
            nudPasswordMinUppercaseCount.Text = _tenantConfigurationDto.PasswordPolicy.MinUppercaseCount.ToString();
            nudPasswordLifetimeDays.Text = _tenantConfigurationDto.PasswordPolicy.PasswordLifetimeDays.ToString();
            nudPasswordProhibitedPreviousPasswordCount.Text = _tenantConfigurationDto.PasswordPolicy.ProhibitedPreviousPasswordCount.ToString();

            if (_tenantConfigurationDto.AuthenticationPolicy == null)
                _tenantConfigurationDto.AuthenticationPolicy = new AuthenticationPolicyDto();
            chkAuthPolicyPassword.Checked = _tenantConfigurationDto.AuthenticationPolicy.PasswordBasedAuthentication;
            chkAuthPolicyWindows.Checked = _tenantConfigurationDto.AuthenticationPolicy.WindowsBasedAuthentication;
            chkAuthPolicyCertificate.Checked = _tenantConfigurationDto.AuthenticationPolicy.CertificateBasedAuthentication;

            if (_tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy == null)
                _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy = new ClientCertificatePolicyDto();
            cbAuthenticationFailoverToCrl.Checked = _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.FailOverToCrlEnabled;
            cbAuthenticationRevocationCheck.Checked = _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.RevocationCheckEnabled;
            cbAuthenticationOcsp.Checked = _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.OcspEnabled;
            cbAuthenticationCrlDistributionPointUsage.Checked = _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CrlDistributionPointUsageEnabled;
            txtAuthenticationOcspUrlOverride.Text = _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.OcspUrlOverride;
            txtAuthenticationCrlDistributionPointOverride.Text = _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CrlDistributionPointOverride;

            if (_tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs != null)
            {
                foreach (string item in _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.CertPolicyOIDs)
                {
                    var lstItem = new ListViewItem(item) { Tag = item };
                    lstAuthenticationCertificatePolicyOids.Items.Add(lstItem);
                }
            }

            if (_tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates != null)
            {
                int count = 1;
                foreach (CertificateDto item in _tenantConfigurationDto.AuthenticationPolicy.ClientCertificatePolicy.TrustedCACertificates)
                {
                    var displayName = string.Empty;
                    try
                    {
                        var x509Cert = new X509Certificate2(Encoding.ASCII.GetBytes(item.Encoded));
                        displayName = x509Cert.Subject;
                    }
                    catch
                    {
                        displayName = "Certificate " + count;
                    }
                    var lstItem = new ListViewItem(displayName) { Tag = item };
                    lstAuthenticationCertificates.Items.Add(lstItem);
                    count++;
                }
            }
        }
        private void btnAuthenticationAddOid_Click(object sender, EventArgs e)
        {
            var form = new AddNameIdFormat("OID");
            if (form.ShowDialog() == DialogResult.OK)
            {
                var lstItem = new ListViewItem(form.NameIdFormatString) { Tag = form.NameIdFormatString };
                lstAuthenticationCertificatePolicyOids.Items.Add(lstItem);
            }
        }
        private void btnAuthenticationRemoveOid_Click(object sender, EventArgs e)
        {
            if (lstAuthenticationCertificates.SelectedIndices.Count > 0)
                lstAuthenticationCertificates.Items.RemoveAt(lstAuthenticationCertificates.SelectedIndices[0]);
        }
        private void btnAddCert_Click(object sender, EventArgs e)
        {
            using (var ofd = new OpenFileDialog())
            {
                ofd.Filter = "Certificate Files (*.crt)|*.crt|All Files (*.*)|*.*";
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    X509Certificate2 x509 = null;
                    try
                    {
                        x509 = new X509Certificate2(ofd.FileName);
                    }
                    catch (Exception exc)
                    {
                        MMCDlgHelper.ShowWarning("Invalid X509 certificate " + exc.Message);
                        return;
                    }
                    var lstItem = new ListViewItem(ofd.FileName) { Tag = new CertificateDto { Encoded = x509.ExportToPem() } };
                    lstAuthenticationCertificates.Items.Add(lstItem);
                }
            }
        }
        private void btnRemoveCert_Click(object sender, EventArgs e)
        {
            if (lstAuthenticationCertificates.SelectedIndices.Count > 0)
                lstAuthenticationCertificates.Items.RemoveAt(lstAuthenticationCertificates.SelectedIndices[0]);
        }
        void ShowCertificateDetails(CertificateDto certDto)
        {
            var cert = new X509Certificate2(Encoding.ASCII.GetBytes(certDto.Encoded));
            if (cert != null)
            {
                X509Certificate2UI.DisplayCertificate(cert);
            }
        }
        private void lstAuthenticationCertificates_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveCert.Enabled = lstAuthenticationCertificates.SelectedIndices.Count > 0;
        }

        private void lstAuthenticationCertificatePolicyOids_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnAuthenticationRemoveOid.Enabled = lstAuthenticationCertificatePolicyOids.SelectedIndices.Count > 0;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (lstAuthenticationCertificates.SelectedItems.Count > 0)
            {
                var certDto = lstAuthenticationCertificates.SelectedItems[0].Tag as CertificateDto;
                ShowCertificateDetails(certDto);
            }
        }

        private void lstAuthenticationCertificates_DoubleClick(object sender, EventArgs e)
        {
            if (lstAuthenticationCertificates.SelectedItems.Count > 0)
            {
                var certDto = lstAuthenticationCertificates.SelectedItems[0].Tag as CertificateDto;
                ShowCertificateDetails(certDto);
            }
        }
    }
}

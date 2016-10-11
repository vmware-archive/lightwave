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
using System.Linq;
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
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class NewRelyingParty : Form, IView
    {
        private readonly ServerDto _serverDto;
        private readonly string _tenantName;
        private CertificateDto _certDto;
        private RelyingPartyDto _relyingPartyDtoOrig;
        public IDataContext DataContext
        {
            get { return _relyingPartyDtoOrig; }
        }
        public NewRelyingParty(ServiceGateway service, ServerDto serverDto, string tenantName)
        {
            _serverDto = serverDto;
            _tenantName = tenantName;
            InitializeComponent();
            Text = "New Relying Party";
            btnCreate.Text = "Create";
        }

        public NewRelyingParty(RelyingPartyDto relyingPartyDto, ServerDto serverDto, string tenantName)
        {
            _serverDto = serverDto;
            _tenantName = tenantName;
            _relyingPartyDtoOrig = relyingPartyDto;
            InitializeComponent();
            Text = "Relying Party";
            btnCreate.Text = "Update";
            DtoToView(relyingPartyDto);
        }

        private void DtoToView(RelyingPartyDto relyingPartyDto)
        {
            txtName.ReadOnly = true;
            txtName.Text = relyingPartyDto.Name;
            txtUrl.Text = relyingPartyDto.Url;
            chkSigned.Checked = relyingPartyDto.AuthnRequestsSigned;
            _certDto = relyingPartyDto.Certificate;
            txtCertificateFilePath.Text = "Certificate";

            if (relyingPartyDto.AssertionConsumerServices != null)
            {
                foreach (AssertionConsumerServiceDto service in relyingPartyDto.AssertionConsumerServices)
                {
                    service.IsDefault = service.Name == relyingPartyDto.DefaultAssertionConsumerService;
                    var lstItem = new ListViewItem(new[] { service.Name, service.Index.ToString(), service.IsDefault ? "YES" : "NO", service.Endpoint, service.Binding }) { Tag = service };
                    lstAssertionConsumerServices.Items.Add(lstItem);
                }
            }

            if (relyingPartyDto.AttributeConsumerServices != null)
            {
                foreach (AttributeConsumerServiceDto service in relyingPartyDto.AttributeConsumerServices)
                {
                    service.IsDefault = service.Name == relyingPartyDto.DefaultAttributeConsumerService;
                    var lstItem = new ListViewItem(new string[] { service.Name, service.Index.ToString(), service.IsDefault ? "YES" : "NO" }) { Tag = service };
                    lstAttributeConsumerServices.Items.Add(lstItem);
                }
            }

            if (relyingPartyDto.SignatureAlgorithms != null)
            {
                foreach (SignatureAlgorithmDto service in relyingPartyDto.SignatureAlgorithms)
                {
                    var lstItem = new ListViewItem(new[] { service.MaxKeySize.ToString(), service.MinKeySize.ToString(), service.Priority.ToString() }) { Tag = service };
                    lstSignatureAlgorithms.Items.Add(lstItem);
                }
            }

            if (relyingPartyDto.SingleLogoutServices != null)
            {
                foreach (ServiceEndpointDto service in relyingPartyDto.SingleLogoutServices)
                {
                    var lstItem = new ListViewItem(new[] { service.Name, service.Endpoint, service.Binding }) { Tag = service };
                    lstSloServices.Items.Add(lstItem);
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
                    var relyingPartyDto = new RelyingPartyDto
                    {
                        Name = txtName.Text,
                        Url = txtUrl.Text,
                        AuthnRequestsSigned = chkSigned.Checked,
                        Certificate = _certDto,
                        AttributeConsumerServices = GetAttributeConsumerServices(),
                        AssertionConsumerServices = GetAssertionConsumerServices(),
                        SingleLogoutServices = GetSloServices(),
                        SignatureAlgorithms = GetSignatureAlgorithms()
                    };
                    var defaultAssertionConsumerService = relyingPartyDto.AssertionConsumerServices.Where(x => x.IsDefault).FirstOrDefault();

                    if(defaultAssertionConsumerService != null)
                        relyingPartyDto.DefaultAssertionConsumerService = defaultAssertionConsumerService.Name;

                    var defaultAttributeConsumerService = relyingPartyDto.AttributeConsumerServices.Where(x => x.IsDefault).FirstOrDefault();
                    if(defaultAttributeConsumerService != null)
                        relyingPartyDto.DefaultAttributeConsumerService = defaultAttributeConsumerService.Name;

                    var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                    if (_relyingPartyDtoOrig == null)
                    {
                        _relyingPartyDtoOrig = service.RelyingParty.Create(_serverDto, _tenantName, relyingPartyDto, auth.Token);
                    }
                    else
                    {
                        _relyingPartyDtoOrig = service.RelyingParty.Update(_serverDto, _tenantName, relyingPartyDto, auth.Token);
                    }
                }, auth);
                this.DialogResult = DialogResult.OK;
            }
        }

        private List<SignatureAlgorithmDto> GetSignatureAlgorithms()
        {
            var list = new List<SignatureAlgorithmDto>();
            foreach (ListViewItem item in lstSignatureAlgorithms.Items)
            {
                list.Add((SignatureAlgorithmDto)item.Tag);
            }
            return list;
        }

        private List<AssertionConsumerServiceDto> GetAssertionConsumerServices()
        {
            var list = new List<AssertionConsumerServiceDto>();
            foreach (ListViewItem item in lstAssertionConsumerServices.Items)
            {
                list.Add((AssertionConsumerServiceDto)item.Tag);
            }
            return list;
        }

        private List<AttributeConsumerServiceDto> GetAttributeConsumerServices()
        {
            var list = new List<AttributeConsumerServiceDto>();
            foreach (ListViewItem item in lstAttributeConsumerServices.Items)
            {
                list.Add((AttributeConsumerServiceDto)item.Tag);
            }
            return list;
        }
        private bool ValidateInputs()
        {
            if (txtName.Enabled && string.IsNullOrEmpty(txtName.Text))
            {
                MMCDlgHelper.ShowWarning("Enter a valid relying party name");
                return false;
            }
            if (string.IsNullOrEmpty(txtUrl.Text) && !Uri.IsWellFormedUriString(txtUrl.Text, UriKind.Absolute))
            {
                MMCDlgHelper.ShowWarning("Enter a valid and well formed Url");
                return false;
            }
            if (string.IsNullOrEmpty(txtCertificateFilePath.Text))
            {
                MMCDlgHelper.ShowWarning("Enter a valid Certificate");
                return false;
            }
            if (lstSignatureAlgorithms.Items.Count == 0)
            {
                MMCDlgHelper.ShowWarning("Enter valid Signature Algorthms");
                return false;
            }
            if (lstAssertionConsumerServices.Items.Count == 0)
            {
                MMCDlgHelper.ShowWarning("Enter valid Assertion Consumer Services");
                return false;
            }
            if (lstAttributeConsumerServices.Items.Count == 0)
            {
                MMCDlgHelper.ShowWarning("Enter valid Attribute Consumer Services");
                return false;
            }
            if (lstSloServices.Items.Count == 0)
            {
                MMCDlgHelper.ShowWarning("Enter valid Single Logout Services");
                return false;
            }
            return true;
        }
        private List<ServiceEndpointDto> GetSloServices()
        {
            var list = new List<ServiceEndpointDto>();
            foreach (ListViewItem item in lstSloServices.Items)
            {
                list.Add((ServiceEndpointDto)item.Tag);
            }
            return list;
        }
        private void btnRemoveSloService_Click(object sender, EventArgs e)
        {
            if (lstSloServices.SelectedIndices != null && lstSloServices.SelectedIndices.Count > 0)
                lstSloServices.Items.RemoveAt(lstSloServices.SelectedIndices[0]);
        }
        private void btnAddSignatureAlgorithm_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                var form = new AddNewSignatureAlgorithm();
                if (form.ShowDialog() == DialogResult.OK)
                {
                    var service = (SignatureAlgorithmDto)form.DataContext;
                    if (service.MaxKeySize < service.MinKeySize)
                    {
                        MMCDlgHelper.ShowError("MAX Key size cannot be less than MIN key size");
                        DialogResult = DialogResult.Cancel;
                        return;
                    }
                    var lstItem = new ListViewItem(new[] { service.MaxKeySize.ToString(), service.MinKeySize.ToString(), service.Priority.ToString() }) { Tag = service };
                    lstSignatureAlgorithms.Items.Add(lstItem);
                }
            }, null);
        }
        private void btnRemoveSignatureAlgorithm_Click(object sender, EventArgs e)
        {
            if (lstSignatureAlgorithms.SelectedIndices != null && lstSignatureAlgorithms.SelectedIndices.Count > 0)
                lstSignatureAlgorithms.Items.RemoveAt(lstSignatureAlgorithms.SelectedIndices[0]);
        }

        private void btnAddSloService_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                var form = new AddNewServiceEndpoints();
                if (form.ShowDialog() == DialogResult.OK)
                {
                    var service = (ServiceEndpointDto)form.DataContext;
                    var lstItem = new ListViewItem(new[] { service.Name, service.Endpoint, service.Binding }) { Tag = service };
                    lstSloServices.Items.Add(lstItem);
                }
            }, null);
        }

        private void btnAddAttributeConsumerService_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                var form = new AddNewAttributeConsumerService();
                if (form.ShowDialog() == DialogResult.OK)
                {
                    var service = (AttributeConsumerServiceDto)form.DataContext;
                    var lstItem = new ListViewItem(new string[] { service.Name, service.Index.ToString(), service.IsDefault ? "YES" : "NO" }) { Tag = service };
                    lstAttributeConsumerServices.Items.Add(lstItem);
                }
            }, null);
        }

        private void btnRemoveAttributeConsumerService_Click(object sender, EventArgs e)
        {
            if (lstAttributeConsumerServices.SelectedIndices != null && lstAttributeConsumerServices.SelectedIndices.Count > 0)
                lstAttributeConsumerServices.Items.RemoveAt(lstAttributeConsumerServices.SelectedIndices[0]);

        }

        private void btnAddAssertionConsumerService_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                var form = new AddNewAssertionConsumerService();
                if (form.ShowDialog() == DialogResult.OK)
                {
                    var service = (AssertionConsumerServiceDto)form.DataContext;
                    var lstItem = new ListViewItem(new[] { service.Name, service.Index.ToString(), service.IsDefault ? "YES" : "NO", service.Endpoint, service.Binding }) { Tag = service };
                    lstAssertionConsumerServices.Items.Add(lstItem);
                }
            }, null);
        }
        private void btnRemoveAssertionConsumerService_Click(object sender, EventArgs e)
        {
            if (lstAssertionConsumerServices.SelectedIndices != null && lstAssertionConsumerServices.SelectedIndices.Count > 0)
                lstAssertionConsumerServices.Items.RemoveAt(lstAssertionConsumerServices.SelectedIndices[0]);
        }
        private void btnChooseCertificate_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate()
            {
                using (var ofd = new OpenFileDialog())
                {
                    ofd.Filter = "Cert Files (*.cer)|*.cer|All Files (*.*)|*.*";
                    if (ofd.ShowDialog() == DialogResult.OK)
                    {
                        txtCertificateFilePath.Text = ofd.FileName;
                        var cert = new X509Certificate2();
                        cert.Import(txtCertificateFilePath.Text);
                        _certDto = new CertificateDto { Encoded = cert.ExportToPem() };
                    }
                }
            }, null);
        }

        private void lstSignatureAlgorithms_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveSignatureAlgorithm.Enabled = (lstSignatureAlgorithms.SelectedIndices != null && lstSignatureAlgorithms.SelectedIndices.Count > 0);
        }

        private void lstAssertionConsumerServices_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveAssertionConsumerService.Enabled = (lstAssertionConsumerServices.SelectedIndices != null && lstAssertionConsumerServices.SelectedIndices.Count > 0);
        }

        private void lstAttributeConsumerServices_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveAttributeConsumerService.Enabled = (lstAttributeConsumerServices.SelectedIndices != null && lstAttributeConsumerServices.SelectedIndices.Count > 0);
        }

        private void lstSloServices_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveSloService.Enabled = (lstSloServices.SelectedIndices != null && lstSloServices.SelectedIndices.Count > 0);
        }

        private void btnViewCertificate_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate()
            {
                var cert = new X509Certificate2(Encoding.ASCII.GetBytes(_certDto.Encoded));
                X509Certificate2UI.DisplayCertificate(cert);
            }, null);
        }
    }
}

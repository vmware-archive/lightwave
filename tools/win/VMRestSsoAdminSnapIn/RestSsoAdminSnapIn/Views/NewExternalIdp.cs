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
using Vmware.Tools.RestSsoAdminSnapIn.Service.Contracts;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class NewExternalIdp : Form, IView
    {
        private readonly ServiceGateway _service;
        private readonly ServerDto _serverDto;
        private readonly string _tenantName;
        private ExternalIdentityProviderDto _externalIdentityProviderDtoOrig;
        public IDataContext DataContext
        {
            get { return _externalIdentityProviderDtoOrig; }
        }
        public NewExternalIdp(ServiceGateway service, ServerDto serverDto, string tenantName)
        {
            _service = service;
            _serverDto = serverDto;
            _tenantName = tenantName;
            InitializeComponent();
            Text = "New External Identity Provider";
            btnCreate.Text = "Create";
        }

        public NewExternalIdp(ExternalIdentityProviderDto externalIdentityProviderDto, ServiceGateway service, ServerDto serverDto, string tenantName)
        {
            _service = service;
            _serverDto = serverDto;
            _tenantName = tenantName;
            _externalIdentityProviderDtoOrig = externalIdentityProviderDto;
            InitializeComponent();

            txtEntityId.ReadOnly = true;
            DtoToView(_externalIdentityProviderDtoOrig);
            if (lstCertificateChain.Items.Count > 0)
                lstCertificateChain.Items[0].Selected = true;
            Text = "External Identity Provider";
            btnCreate.Text = "Update";
        }
        private void DtoToView(ExternalIdentityProviderDto externalIdentityProviderDto)
        {
            txtEntityId.Text = externalIdentityProviderDto.EntityID;
            txtAlias.Text = externalIdentityProviderDto.Alias;
            chkJit.Checked = externalIdentityProviderDto.JitEnabled;
            foreach (var value in externalIdentityProviderDto.NameIDFormats)
            {
                lstNameIdFormats.Items.Add(value);
            }

            foreach (var value in externalIdentityProviderDto.SsoServices)
            {
                lstSsoServices.Items.Add(value);
            }

            foreach (var value in externalIdentityProviderDto.SloServices)
            {
                lstSloServices.Items.Add(value);
            }

            foreach (var value in externalIdentityProviderDto.SubjectFormats)
            {
                var listViewItem = new ListViewItem(new[] { value.Key, value.Value }) { Tag = new SubjectFormatDto { Key = value.Key, Value = value.Value } };
                lstSubjectFormat.Items.Add(listViewItem);
            }

            foreach (var value in externalIdentityProviderDto.SigningCertificates.Certificates)
            {
                try
                {
                    var cert = new X509Certificate2(Encoding.ASCII.GetBytes(value.Encoded));
                    var lst = new ListViewItem(cert.Subject) { Tag = value };
                    lstCertificateChain.Items.Add(lst);
                }
                catch
                {
                    // do nothing
                }
            }            
        }
        private void btnSelectCertFile_Click(object sender, EventArgs e)
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
                    var lst = new ListViewItem(ofd.FileName) { Tag = new CertificateDto { Encoded = cert.ExportToPem() } };
                    lstCertificateChain.Items.Add(lst);
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
                    var externalIdentityProviderDto = new ExternalIdentityProviderDto
                    {
                        EntityID = txtEntityId.Text,
                        Alias = txtAlias.Text,
                        JitEnabled = chkJit.Checked,
                        NameIDFormats = GetNamedIdFormats(),
                        SsoServices = GetSsoServices(),
                        SloServices = GetSloServices(),
                        SubjectFormats = GetSubjectFormats(),
                        SigningCertificates = GetCertificates()
                    };

                    IExternalIdentityProviderService idp = _service.ExternalIdentityProvider;
                    _externalIdentityProviderDtoOrig = idp.Create(_serverDto, _tenantName, externalIdentityProviderDto, auth.Token);                   
                }, auth);
                this.DialogResult = DialogResult.OK;
            }
        }

        private CertificateChainDto GetCertificates()
        {
            var chain = new CertificateChainDto();
            chain.Certificates = new List<CertificateDto>();
            foreach (ListViewItem item in lstCertificateChain.Items)
            {
                var dto = item.Tag as CertificateDto;
                chain.Certificates.Add(dto);
            }
            return chain;
        }

        private Dictionary<string, string> GetSubjectFormats()
        {
            var dict = new Dictionary<string, string>();
            foreach (ListViewItem item in lstSubjectFormat.Items)
            {
                var subjectFormatDto = (SubjectFormatDto)item.Tag;
                dict.Add(subjectFormatDto.Key, subjectFormatDto.Value);
            }
            return dict;
        }

        private bool ValidateInputs()
        {
            if (txtEntityId.Enabled && string.IsNullOrEmpty(txtEntityId.Text))
            {
                MMCDlgHelper.ShowWarning("Enter a valid entity id");
                return false;
            }
            if (lstSubjectFormat.Items.Count < 1)
            {
                MMCDlgHelper.ShowWarning("Enter atleast one subject format");
                return false;
            }
            if (lstNameIdFormats.Items.Count < 1)
            {
                MMCDlgHelper.ShowWarning("Enter atleast one name id format");
                return false;
            }
            if (lstSloServices.Items.Count < 1)
            {
                MMCDlgHelper.ShowWarning("Enter atleast one Slo service");
                return false;
            }
            if (lstSsoServices.Items.Count < 1)
            {
                MMCDlgHelper.ShowWarning("Enter atleast one Sso service");
                return false;
            }
            if (lstCertificateChain.Items.Count < 1)
            {
                MMCDlgHelper.ShowWarning("Enter atleast one certificate");
                return false;
            }
            return true;
        }

        private List<ServiceEndpointDto> GetSsoServices()
        {
            var list = new List<ServiceEndpointDto>();
            foreach (var item in lstSsoServices.Items)
            {
                list.Add((ServiceEndpointDto)item);
            }
            return list;
        }

        private List<ServiceEndpointDto> GetSloServices()
        {
            var list = new List<ServiceEndpointDto>();
            foreach (var item in lstSloServices.Items)
            {
                list.Add((ServiceEndpointDto)item);
            }
            return list;
        }

        private List<string> GetNamedIdFormats()
        {
            var list = new List<string>();
            foreach (var item in lstNameIdFormats.Items)
            {
                list.Add(item.ToString());
            }
            return list;
        }

        private void btnRemoveCert_Click(object sender, EventArgs e)
        {
            if (lstCertificateChain.SelectedItems.Count > -1)
                lstCertificateChain.Items.RemoveAt(lstCertificateChain.SelectedIndices[0]);
        }
        private void lstCertificateChain_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveCert.Enabled = lstCertificateChain.SelectedIndices.Count > 0;
        }
        private void btnAddNameIdFormat_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                var form = new AddNameIdFormat("Name Id Format");
                if (form.ShowDialog() == DialogResult.OK)
                {
                    lstNameIdFormats.Items.Add(form.NameIdFormatString);
                }
            },null);

        }
        private void btnRemoveNameIdFormat_Click(object sender, EventArgs e)
        {
            if (lstNameIdFormats.SelectedIndex > -1)
                lstNameIdFormats.Items.RemoveAt(lstNameIdFormats.SelectedIndex);
        }

        private void btnRemoveSsoService_Click(object sender, EventArgs e)
        {
            if (lstSsoServices.SelectedIndex > -1)
                lstSsoServices.Items.RemoveAt(lstSsoServices.SelectedIndex);
        }

        private void btnRemoveSloService_Click(object sender, EventArgs e)
        {
            if (lstSloServices.SelectedIndex > -1)
                lstSloServices.Items.RemoveAt(lstSloServices.SelectedIndex);
        }

        private void btnAddSsoService_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                var form = new AddNewServiceEndpoints();
                if (form.ShowDialog() == DialogResult.OK)
                {
                    var service = (ServiceEndpointDto)form.DataContext;
                    lstSsoServices.Items.Add(service);
                }
            }, null);
        }

        private void btnSloService_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                var form = new AddNewServiceEndpoints();
                if (form.ShowDialog() == DialogResult.OK)
                {
                    var service = (ServiceEndpointDto)form.DataContext;
                    lstSloServices.Items.Add(service);
                }
            }, null);
        }

        private void btnAddSubjectFormat_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
           {
               var form = new AddSubjectFormat();
               if (form.ShowDialog() == DialogResult.OK)
               {
                   var item = (SubjectFormatDto)form.DataContext;
                   var listViewItem = new ListViewItem(new[] { item.Key, item.Value }) { Tag = item };
                   lstSubjectFormat.Items.Add(listViewItem);
               }
           }, null);
        }

        private void btnRemoveSubjectFormat_Click(object sender, EventArgs e)
        {
            if (lstSubjectFormat.SelectedItems.Count > 0)
            {
                var selectedIndex = lstSubjectFormat.SelectedIndices[0];
                lstSubjectFormat.Items.RemoveAt(selectedIndex);
            }
        }

        private void lstSubjectFormat_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveSubjectFormat.Enabled = lstSubjectFormat.SelectedIndices != null && lstSubjectFormat.SelectedIndices.Count > 0;
        }

        private void lstNameIdFormats_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveNameIdFormat.Enabled = lstNameIdFormats.SelectedIndex > -1;
        }

        private void lstSsoServices_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveSsoService.Enabled = lstSsoServices.SelectedIndex > -1;
        }

        private void lstSloServices_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveSloService.Enabled = lstSloServices.SelectedIndex > -1;
        }

        private void lstCertificateChain_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (lstCertificateChain.SelectedIndices.Count > 0)
            {
                var certificateDto = lstCertificateChain.SelectedItems[0].Tag as CertificateDto;
                var encoded = certificateDto.Encoded;
                var cert = new X509Certificate2(Encoding.ASCII.GetBytes(encoded));
                X509Certificate2UI.DisplayCertificate(cert);
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (lstCertificateChain.SelectedIndices.Count > 0)
            {
                var certificateDto = lstCertificateChain.SelectedItems[0].Tag as CertificateDto;
                var encoded = certificateDto.Encoded;
                var cert = new X509Certificate2(Encoding.ASCII.GetBytes(encoded));
                X509Certificate2UI.DisplayCertificate(cert);
            }
        }
    }
}

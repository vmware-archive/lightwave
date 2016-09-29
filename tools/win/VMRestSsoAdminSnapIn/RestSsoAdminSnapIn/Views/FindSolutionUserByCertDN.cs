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
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Crypto;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using Vmware.Tools.RestSsoAdminSnapIn.Service.SolutionUser;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class FindSolutionUserByCertDN : Form
    {
        private readonly ServiceGateway _service;
        private readonly ServerDto _serverDto;
        public string CertDn { get; private set; }
        public FindSolutionUserByCertDN(ServiceGateway service, ServerDto serverDto)
        {
            _service = service;
            _serverDto = serverDto;
            InitializeComponent();
        }

        private void btnSelectCertFile_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate()
            {
                using (var ofd = new OpenFileDialog())
                {
                    ofd.Filter = "Certificate Files (*.crt)|*.crt|All Files (*.*)|*.*";
                    if (ofd.ShowDialog() == DialogResult.OK)
                    {
                        txtCertFile.Text = ofd.FileName;
                        PopulateCertDN();
                    }
                }
            }, null);
        }

        void PopulateCertDN()
        {
            if (rdoUseFile.Checked)
            {
                var cert = new X509Certificate2(txtCertFile.Text);
                var subjectDN = ShaWithRsaSigner.GetX500SubjectDN(cert);
                txtCertDN.Text = subjectDN;
            }
            CertDn = txtCertDN.Text;
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate()
            {
                if (string.IsNullOrWhiteSpace(txtCertDN.Text) && string.IsNullOrWhiteSpace(txtCertFile.Text))
                    throw new Exception("Please select a certificate file or enter Certificate DN");
                if (!string.IsNullOrWhiteSpace(txtCertDN.Text))
                    PopulateCertDN();
                DialogResult = DialogResult.OK;
                Close();
            },null);
        }

        private void rdoUseDN_CheckedChanged(object sender, EventArgs e)
        {
            txtCertDN.ReadOnly = false;
            txtCertFile.ReadOnly = true;
            btnSelectCertFile.Enabled = false;
        }

        private void rdoUseFile_CheckedChanged(object sender, EventArgs e)
        {
            txtCertDN.ReadOnly = true;
            txtCertFile.ReadOnly = false;
            btnSelectCertFile.Enabled = true;
        }
    }
}

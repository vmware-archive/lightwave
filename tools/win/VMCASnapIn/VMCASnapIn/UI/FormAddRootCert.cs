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
using System.Windows.Forms;
using VMCASnapIn.DTO;
using VMCASnapIn.UI.GridEditors;
using VMCASnapIn.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCASnapIn.UI
{
    public partial class FormAddRootCert : Form
    {
        AddCertificateDTO certDTO;

        public FormAddRootCert(AddCertificateDTO certDTO)
        {
            this.certDTO = certDTO;
            this.certDTO.PrivateKey = new PrivateKeyDTO();
            InitializeComponent();
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            if (!ValidateForm())
            {
                this.DialogResult = DialogResult.None;
                return;
            }
            certDTO.Certificate = txtCertificate.Text;
            this.Close();
        }

        public bool ValidateForm()
        {
            string msg = null;
            if (string.IsNullOrEmpty(txtCertificate.Text))
                msg = MMCUIConstants.CERT_SEL;
            else if (string.IsNullOrEmpty(txtPrivateKey.Text))
                msg = MMCUIConstants.PRI_KEY_SEL;

            if (msg != null)
            {
                MMCDlgHelper.ShowWarning(msg);
                return false;
            }
            return true;
        }

        private void btnBrowsePrivateKey_Click(object sender, EventArgs e)
        {
            var frm = new frmPrivateKeyEditor(certDTO.PrivateKey);

            if (frm.ShowDialog()==DialogResult.OK)
            {
                if (!String.IsNullOrEmpty(certDTO.PrivateKey.PrivateKeyFileName))
                    txtPrivateKey.Text = certDTO.PrivateKey.PrivateKeyFileName;
                else
                    txtPrivateKey.Text = certDTO.PrivateKey.PrivateKeyString;
            }
        }

        private void btnBrowseCertificate_Click(object sender, EventArgs e)
        {
            txtCertificate.Text = MMCMiscUtil.SelectFile("Select Certificate",MMCUIConstants.CERT_FILTER);
        }
    }
}

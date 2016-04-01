﻿/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
using System;
using System.Windows.Forms;
using VMCertStore.Common.DTO;
using VMCertStoreSnapIn.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCertStoreSnapIn.UI
{
    public partial class frmAddCertificate : Form
    {
        AddCertificateDTO _certificateDTO = new AddCertificateDTO();
        public AddCertificateDTO CertificateDTO { get { return _certificateDTO; } }

        public frmAddCertificate()
        {
            InitializeComponent();
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            if (!ValidateForm())
            {
                this.DialogResult = DialogResult.None;
                return;
            }

            _certificateDTO.Alias = txtAlias.Text;
            _certificateDTO.Certificate = txtCertificate.Text;
            _certificateDTO.AutoRefresh = chkAutoRefresh.Checked;

            this.Close();
        }

        public bool ValidateForm()
        {
            string msg = null;
            if (string.IsNullOrEmpty(txtAlias.Text))
                msg = MMCUIConstants.ALIAS_ENT;
            else if (string.IsNullOrEmpty(txtCertificate.Text))
                msg = MMCUIConstants.CERT_SEL;
            if (msg != null)
            {
                MMCDlgHelper.ShowWarning(msg);
                return false;
            }
            return true;
        }

        private void btnBrowseCertificate_Click(object sender, EventArgs e)
        {
            txtCertificate.Text = MMCMiscUtil.SelectFile("Select Certificate", MMCUIConstants.CERT_FILTER);
        }
    }
}

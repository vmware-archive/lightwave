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
using VMCertStore.Common.DTO;
using VMCertStoreSnapIn.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;
using System.IO;
using VMIdentity.CommonUtils;

namespace VMCertStoreSnapIn.UI
{
    public partial class FormAddSecretKey : Form
    {
        SecretKeyDTO _secretKeyDTO = new SecretKeyDTO();
        public SecretKeyDTO SecretKeyDTO { get { return _secretKeyDTO; } }

        public FormAddSecretKey()
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

            _secretKeyDTO.Alias = txtAlias.Text;
            _secretKeyDTO.SecretKey = txtSecretKey.Text;
            _secretKeyDTO.Password = txtPassword.Text;

            this.Close();
        }

        public bool ValidateForm()
        {
            string msg = null;
            if (string.IsNullOrWhiteSpace(txtAlias.Text))
                msg = MMCUIConstants.ALIAS_ENT;
            else if (string.IsNullOrWhiteSpace(txtSecretKey.Text))
                msg = MMCUIConstants.SEC_KEY_SEL;
            if (msg != null)
            {
                MMCDlgHelper.ShowWarning(msg);
                return false;
            }
            return true;
        }

        private void button1_Click(object sender, EventArgs e)
        {
             var keyFile= MMCMiscUtil.SelectFile("Select Secret key", MMCUIConstants.PRI_KEY_FILTER);
             try{
                 this.txtSecretKey.Text = File.ReadAllText(keyFile);
             }
            catch(Exception){}
        }

    }
}

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
using VMDir.Common;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSnapIn.UI
{
    public partial class ResetUserPwdForm : Form
    {
        public string Password;
        public string Dn;
        public ResetUserPwdForm(string dn)
        {
            this.Dn = dn;
            InitializeComponent();
            this.textBoxDn.Text = dn;
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void buttonSubmit_Click(object sender, EventArgs e)
        {
            if (!validateInput())
            {
                this.DialogResult = DialogResult.None;
                return;
            }
            Password = this.textBox1.Text;
        }

        private bool validateInput()
        {
            string msg = string.Empty;
            if (string.IsNullOrWhiteSpace(this.textBoxDn.Text))
            {
                msg = VMDirConstants.WRN_DN_ENT;
            }
            else if (string.IsNullOrWhiteSpace(this.textBox1.Text))
            {
                msg = VMDirConstants.WRN_NEW_PWD_ENT;
            }
            else if (!string.Equals(this.textBox1.Text, this.textBox2.Text))
            {
                msg = VMDirConstants.WRN_PWD_NO_MATCH;
            }
            if (!string.IsNullOrWhiteSpace(msg))
            {
                MMCDlgHelper.ShowWarning(msg);
                return false;
            }
            return true;

        }
    }
}

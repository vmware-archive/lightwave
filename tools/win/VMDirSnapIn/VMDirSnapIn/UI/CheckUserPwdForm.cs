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
    public partial class CheckUserPwdForm : Form
    {
        public string UPN;
        public string Password;
        public CheckUserPwdForm(string upn)
        {
            this.UPN = upn;
            InitializeComponent();
            this.textBoxUpn.Text = upn;
        }
        private bool validateInput()
        {
            if (string.IsNullOrWhiteSpace(this.textBoxUpn.Text))
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_UPN_ENT);
                return false;
            }
            else if (string.IsNullOrWhiteSpace(this.textBoxPwd.Text))
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_PWD_ENT);
                return false;
            }
            return true;

        }
        private void buttonSubmit_Click(object sender, EventArgs e)
        {
            if (!validateInput())
            {
                this.DialogResult = DialogResult.None;
                return;
            }
            Password = this.textBoxPwd.Text;
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}

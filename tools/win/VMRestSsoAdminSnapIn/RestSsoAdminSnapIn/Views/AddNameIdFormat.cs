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
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class AddNameIdFormat : Form
    {
        public string NameIdFormatString;
        public bool _validateForSsl;
        public AddNameIdFormat(string caption, bool validateForSsl=false)
        {
            InitializeComponent();
            Text = caption;
            lblText.Text = caption;
            _validateForSsl = validateForSsl;
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            NameIdFormatString = txtName.Text.Trim();
            if(string.IsNullOrEmpty(txtName.Text.Trim()))
            {
                MMCDlgHelper.ShowWarning("Enter a valid value");
                return;
            }           
            if (_validateForSsl && !string.IsNullOrWhiteSpace(txtName.Text) && !txtName.Text.StartsWith("https:"))
            {
                MMCDlgHelper.ShowWarning("Enter a valid HTTPS " + this.Text);
                return;
            }
            DialogResult = DialogResult.OK;
            Close();

            string siteName;
            int error = Test(out siteName);
        }

        private int Test(out string name)
        {
            name = "";
            return 1;
        }
    }
}

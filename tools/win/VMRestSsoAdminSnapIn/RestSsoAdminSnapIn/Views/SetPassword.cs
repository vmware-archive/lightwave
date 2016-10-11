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
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class SetPassword : Form, IView
    {
        public PasswordResetRequestDto PasswordResetDto;

        public SetPassword(UserDto user)
        {
            Text = "Set password for " + user.Name;
            InitializeComponent();
        }

        public override sealed string Text
        {
            get { return base.Text; }
            set { base.Text = value; }
        }

        private bool ValidateControls()
        {
            var status = true;
            if (string.IsNullOrWhiteSpace(txtPassword.Text))
            {
                status = false;
                MMCDlgHelper.ShowWarning(MMCUIConstants.INVALID_NEW_PASSWORD);
            }
            else if (!string.Equals(txtPassword.Text, txtConfirmPass.Text, StringComparison.InvariantCulture))
            {
                status = false;
                MMCDlgHelper.ShowWarning(MMCUIConstants.PASSWORD_MISMATCH);
            }
            return status;
        }


        private void btnCreate_Click(object sender, EventArgs e)
        {
            if (ValidateControls())
            {
                PasswordResetDto = new PasswordResetRequestDto { NewPassword = txtPassword.Text};//, CurrentPassword = TxtCurrentPassword.Text };
                DialogResult = DialogResult.OK;
            }
            else
                DialogResult = DialogResult.None;
        }

        public IDataContext DataContext
        {
            get { return PasswordResetDto; }
        }
    }
}

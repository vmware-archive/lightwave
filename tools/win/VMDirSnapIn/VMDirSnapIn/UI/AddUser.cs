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
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VMDir.Common;
using VMDir.Common.DTO;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSnapIn.UI
{
    public partial class AddUser : Form
    {
        UserDTO _dto;
        public AddUser(UserDTO dto)
        {
            InitializeComponent();
            _dto = dto;
            _dto.objectClass = VMDirConstants.USER_OC;
        }
        private bool DoValidateControls()
        {
            string msg = string.Empty;

            if (string.IsNullOrWhiteSpace(cnTextBox.Text))
                msg = VMDirConstants.WRN_CN_ENT;
            else if (string.IsNullOrWhiteSpace(firstNameTextBox.Text))
                msg = VMDirConstants.WRN_FN_ENT;
            else if (string.IsNullOrWhiteSpace(lastNameTextBox.Text))
                msg = VMDirConstants.WRN_LN_ENT;
            else if (string.IsNullOrWhiteSpace(samAccNameTextBox.Text))
                msg = VMDirConstants.WRN_SAM_NAME_ENT;
            else if (string.IsNullOrWhiteSpace(upnTextBox.Text))
                msg = VMDirConstants.WRN_UPN_ENT;

            if (!string.IsNullOrWhiteSpace(msg))
            {
                MMCDlgHelper.ShowWarning(msg);
                return false;
            }
            return true;
        }
        private void submitButton_Click(object sender, EventArgs e)
        {
            if (!DoValidateControls())
            {
                this.DialogResult = DialogResult.None;
                return;
            }
            _dto.Cn = firstNameTextBox.Text;
            _dto.FirstName = firstNameTextBox.Text;
            _dto.LastName = lastNameTextBox.Text;
            _dto.SAMAccountName = samAccNameTextBox.Text;
            _dto.UPN = upnTextBox.Text;
            this.Close();
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}

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
    public partial class AddGroup : Form
    {
        private GroupDTO _dto;
        public AddGroup(GroupDTO dto)
        {
            InitializeComponent();
            _dto = dto;
            _dto.groupType = VMDirConstants.GROUPTYPE_ACCOUNT;
            _dto.objectClass = VMDirConstants.GROUP_OC;
        }

        private bool DoValidateControls()
        {
            string msg = string.Empty;

            if (String.IsNullOrWhiteSpace(groupNameTextBox.Text))
                msg = VMDirConstants.WRN_GRP_NAME_ENT;
            else if (String.IsNullOrWhiteSpace(samAccNametextBox.Text))
                msg = VMDirConstants.WRN_SAM_NAME_ENT;

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
            _dto.cn = groupNameTextBox.Text;
            _dto.sAMAccountName = samAccNametextBox.Text;
            this.Close();
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}

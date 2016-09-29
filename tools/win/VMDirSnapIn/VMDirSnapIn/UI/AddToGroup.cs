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
using VMDirSnapIn.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;
using VmdirUtil = VMDir.Common.VMDirUtilities;

namespace VMDirSnapIn.UI
{
    public partial class AddToGroup : Form
    {
        public String DNText { get; set; }
        private VMDirServerDTO serverDTO;
        public AddToGroup(VMDirServerDTO dto)
        {
            InitializeComponent();
            serverDTO = dto;
            dnLabel.Text = "";
        }

        private void submitButton_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(DNText))
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_GRP_NAME_SEL);
                this.DialogResult = DialogResult.None;
                return;
            }
            this.Close();
        }

        private void findCnButton_Click(object sender, EventArgs e)
        {
            MiscUtilsService.CheckedExec(delegate()
            {
                if (!string.IsNullOrWhiteSpace(cnTextBox.Text))
                {
                    string[] dn = VmdirUtil.Utilities.SearchItemCN(serverDTO.BaseDN, "group", cnTextBox.Text, null, serverDTO);
                    //if only single result is found
                    if (dn.Length == 1)
                    {
                        dnLabel.Text = dn[0];
                        DNText = dn[0];
                    }
                    else if (dn.Length <= 0)
                        dnLabel.Text = "Search item not found in groups.";
                    //else if dn.length>1 TODO - Display a separate window  listing all the multiple dn found  and let the user choose one.
                }
            });
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}

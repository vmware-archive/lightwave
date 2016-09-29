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
using System.IO;
using System.Windows.Forms;
using VMCA.Client;
using VMCASnapIn.DTO;
using VMCASnapIn.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCASnapIn.UI.GridEditors
{
    public partial class frmPrivateKeyEditor : Form
    {
        public PrivateKeyDTO PrivateKeyDTO { get; protected set; }

        public frmPrivateKeyEditor(PrivateKeyDTO dto)
        {
            PrivateKeyDTO = dto;

            InitializeComponent();

            if (!string.IsNullOrEmpty(dto.PrivateKeyFileName))
            {
                rdoSelect.Checked = true;
                txtFileName.Text = dto.PrivateKeyFileName;
            }
            else if (!string.IsNullOrEmpty(dto.PrivateKeyString))
            {
                rdoPaste.Checked = true;
                txtKeyData.Text = dto.PrivateKeyString;
            }
        }

        private void rdoCreate_CheckedChanged(object sender, EventArgs e)
        {
            numKeyLength.Enabled = btnCreate.Enabled = true;

            txtKeyData.Enabled = false;
            txtFileName.Enabled = btnBrowse.Enabled = false;
        }

        private void rdoSelect_CheckedChanged(object sender, EventArgs e)
        {
            txtFileName.Enabled = btnBrowse.Enabled = true;

            txtKeyData.Enabled = false;
            numKeyLength.Enabled = btnCreate.Enabled = false;
        }

        private void rdoPaste_CheckedChanged(object sender, EventArgs e)
        {
            txtKeyData.Enabled = true;
            numKeyLength.Enabled = btnCreate.Enabled = false;
            txtFileName.Enabled = btnBrowse.Enabled = false;
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                var data = VMCAKeyPair.Create((uint)numKeyLength.Value);
                rdoPaste.Checked = true;

                if (MessageBox.Show("Do you want to save the keys created?","Confirm",MessageBoxButtons.YesNo,MessageBoxIcon.Question)==DialogResult.Yes)
                {
                    Helper.SaveKeyData(data);
                }

                txtKeyData.Text = data.PrivateKey;
            });
        }

        private void btnBrowse_Click(object sender, EventArgs e)
        {
            txtFileName.Text = MMCMiscUtil.SelectFile("Select private key file",MMCUIConstants.PRI_KEY_FILTER);
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            if (rdoPaste.Checked)
                PrivateKeyDTO.PrivateKeyString = txtKeyData.Text;
            else if (rdoSelect.Checked)
                PrivateKeyDTO.PrivateKeyFileName = txtFileName.Text;
            else
                return;
            this.Close();
        }
    }
}

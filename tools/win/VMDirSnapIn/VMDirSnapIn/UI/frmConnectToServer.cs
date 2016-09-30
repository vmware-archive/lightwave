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
using VMDir.Common.DTO;
using VMDirSnapIn.Utilities;
using VMDir.Common.VMDirUtilities;
using VMwareMMCIDP.UI.Common.Utilities;
using VMIdentity.CommonUtils;
using System.Text;

namespace VMDirSnapIn.UI
{
    public partial class frmConnectToServer : Form
    {
        VMDirServerDTO _dto = null;

        public VMDirServerDTO ServerDTO { get { return _dto; } }

        public frmConnectToServer()
        {
            InitializeComponent();
        }

        public frmConnectToServer(VMDirServerDTO dto)
            : this()
        {
            _dto = dto;
            txtDirectoryServer.Text = dto.Server;
            var tenant = MMCMiscUtil.GetBrandConfig(CommonConstants.TENANT);

            if (string.IsNullOrWhiteSpace(dto.BindDN))
            {
                txtBindUPN.Text = "Administrator@" + tenant;
            }
            else
            {
                txtBindUPN.Text = dto.BindDN;
            }
            if (string.IsNullOrWhiteSpace(dto.BaseDN))
            {
                txtBaseDN.Text = CommonConstants.GetDNFormat(tenant);
            }
            else{
                txtBaseDN.Text = dto.BaseDN;
            }
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            MiscUtilsService.CheckedExec(delegate
           {
               if (!ValidateForm())
               {
                   this.DialogResult = DialogResult.None;
                   return;
               }

               if (_dto == null)
                   _dto = VMDirServerDTO.CreateInstance();

               _dto.Server = (txtDirectoryServer.Text).Trim();
               _dto.BaseDN = (txtBaseDN.Text).Trim();
               _dto.BindDN = (txtBindUPN.Text).Trim();
               _dto.Password = txtPassword.Text;

               _dto.Connection = new LdapConnectionService(_dto.Server, _dto.BindDN, _dto.Password);

               this.Close();
           });
        }

        private bool ValidateForm()
        {
            string msg = null;
            if (string.IsNullOrWhiteSpace(txtDirectoryServer.Text))
                msg = MMCUIConstants.SERVER_ENT;
            else if (string.IsNullOrWhiteSpace(txtBindUPN.Text))
                msg = MMCUIConstants.UPN_ENT;
            else if (string.IsNullOrWhiteSpace(txtPassword.Text))
                msg = MMCUIConstants.PASSWORD_ENT;

            if (msg != null)
            {
                MMCDlgHelper.ShowWarning(msg);
                return false;
            }
            return true;
        }

        private void frmConnectToServer_Load(object sender, EventArgs e)
        {
            if (!String.IsNullOrEmpty(txtDirectoryServer.Text))
                this.txtDirectoryServer.Enabled = false;
        }
    }
}
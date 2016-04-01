/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */using System;using System.Windows.Forms;
using VMDir.Common.DTO;
using VMDirSnapIn.Services;
using VMDir.Common.VMDirUtilities;
namespace VMDirSnapIn.UI{    public partial class frmConnectToServer : Form    {        VMDirServerDTO _dto = null;
        public VMDirServerDTO ServerDTO { get { return _dto; } }
        public frmConnectToServer()        {            InitializeComponent();

            txtBaseDN.Text = "dc=local";
            txtBindUPN.Text = "Administrator@vsphere.local";        }

        public frmConnectToServer(VMDirServerDTO dto)
            : this()
        {
            _dto = dto;

            txtDirectoryServer.Text = dto.Server;
            txtBaseDN.Text = dto.BaseDN;
            txtBindUPN.Text = dto.BindDN;
        }
        private void btnOK_Click(object sender, EventArgs e)        {
            try
            {
                if (_dto == null)
                    _dto = VMDirServerDTO.CreateInstance();

                _dto.Server = txtDirectoryServer.Text;
                _dto.BaseDN = txtBaseDN.Text;
                _dto.BindDN = txtBindUPN.Text;
                _dto.Password = txtPassword.Text;

                _dto.Connection = new LdapConnectionService(_dto.Server, _dto.BindDN, _dto.Password);
                _dto.Connection.CreateConnection();

                this.DialogResult = DialogResult.OK;
            }
            catch (Exception exp)
            {
                MiscUtilsService.ShowError(exp);
            }
        }

        private void frmConnectToServer_Load(object sender, EventArgs e)
        {
            if (!String.IsNullOrEmpty(txtDirectoryServer.Text))
                this.txtDirectoryServer.Enabled = false;
        }    }}
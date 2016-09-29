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
    public partial class Probe : Form, IView
    {
        private IdentityProviderProbeDto probeDto;
        private bool close = true;
        public Probe()
        {
            InitializeComponent();
            probeDto = new IdentityProviderProbeDto();            
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            if (ValidateInputs())
            {
                probeDto.Username = txtUser.Text;
                probeDto.Password = txtPass.Text;
                probeDto.ProviderURI = txtTenantName.Text;
                probeDto.AuthenticationType = txtTenantName.Text;
                Close();
            }
            else
            {
                close = false;
            }
        }
        void EnableOk()
        {
            btnOK.Enabled = txtUser.Text.Length * txtPass.Text.Length * txtTenantName.Text.Length > 0;
        }
        private void txtUser_TextChanged(object sender, EventArgs e)
        {
            EnableOk();
        }
        private void txtPass_TextChanged(object sender, EventArgs e)
        {
            EnableOk();
        }
        private bool ValidateInputs()
        {
            if(string.IsNullOrEmpty(txtUser.Text))
            {
                MMCDlgHelper.ShowWarning("Enter a valid username.");
                return false;
            }            
            if (string.IsNullOrEmpty(txtPass.Text))
            {
                MMCDlgHelper.ShowWarning("Enter a valid password.");
                return false;
            }

            if (string.IsNullOrEmpty(txtTenantName.Text))
            {
                MMCDlgHelper.ShowWarning("Enter a valid tenant name.");
                return false;
            }
            return true;
        }

        public IDataContext DataContext
        {
            get { return probeDto; }
        }

        private void txtTenantName_TextChanged(object sender, EventArgs e)
        {
            EnableOk();
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            base.OnClosing(e);
            e.Cancel = !close;
            close = true;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {            
            Close();
        }
    }
}

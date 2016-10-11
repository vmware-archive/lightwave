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
    public partial class LoginForm : Form, IView
    {
        private readonly LoginDto _userPassDto;
        private bool close = true;
        public LoginForm(LoginDto userPassDto, string domainName, string text = "Login")
        {
            InitializeComponent();
            _userPassDto = userPassDto;
            if (_userPassDto == null)
                _userPassDto = new LoginDto();
            else
                DtoToView();
            Text = text;
            txtTenantName.Text = domainName;
        }

        private void DtoToView()
        {
            txtUser.Text = _userPassDto.User;
            txtPass.Text = _userPassDto.Pass;
            txtTenantName.Text = _userPassDto.TenantName;
        }
        private void btnOK_Click(object sender, EventArgs e)
        {
            if (ValidateInputs())
            {
                _userPassDto.User = txtUser.Text;
                _userPassDto.Pass = txtPass.Text;
                _userPassDto.DomainName = txtTenantName.Text;
                _userPassDto.TenantName = txtTenantName.Text;
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
            get { return _userPassDto; }
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

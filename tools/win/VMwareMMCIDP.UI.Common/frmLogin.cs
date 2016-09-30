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

namespace VMwareMMCIDP.UI
{
    public partial class frmLogin : Form
    {
        public string UserName { get; set; }
        public string Password { get; set; }
        public string DomainName { get; set; }
        public string Upn { get; set; }

        public frmLogin()
        {
            InitializeComponent();

        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            Upn = txtUser.Text.Trim();
            var userAndDomain = Upn.Split('@');
            UserName = userAndDomain[0];
            if(userAndDomain[1] != null)
            DomainName = userAndDomain[1];
            Password = txtPass.Text;

            this.Close();
        }

        void EnableOk()
        {
            btnOK.Enabled = txtUser.Text.Length * txtPass.Text.Length > 0;
        }

        private void txtUser_TextChanged(object sender, EventArgs e)
        {
            EnableOk();
        }

        private void txtPass_TextChanged(object sender, EventArgs e)
        {
            EnableOk();
        }

    }
}

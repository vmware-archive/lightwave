/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
using VMwareMMCIDP.UI.Common.Utilities;
using System.Net;
using VMIdentity.CommonUtils;

namespace VMwareMMCIDP.UI.Common
{
    public partial class LoginForm : Form
    {
        public string Server { get; set; }
        public string UserName { get; set; }
        public string Password { get; set; }
        public string DomainName { get; set; }
        public LoginForm()
        {
            InitializeComponent();
        }
        public LoginForm(string server, string user,string domain):this()
        {
            this.Server = server;
            this.UserName = user;
            if (!string.IsNullOrWhiteSpace(domain))
                this.DomainName = domain;
            else
                this.DomainName = MMCMiscUtil.GetBrandConfig(CommonConstants.TENANT);
        }
        public bool ValidateForm()
        {
            string msg = null;
            if (string.IsNullOrWhiteSpace(textServer.Text))
                msg = MMCUIConstants.SERVER_ENT;
            else if (string.IsNullOrWhiteSpace(textUser.Text))
                msg = MMCUIConstants.USERNAME_ENT;
            else if (string.IsNullOrWhiteSpace(textPassword.Text))
                msg = MMCUIConstants.PASSWORD_ENT;
            else if (string.IsNullOrWhiteSpace(textTenant.Text))
                msg = MMCUIConstants.TENANT_ENT;

            if (msg == null && !string.IsNullOrWhiteSpace(textServer.Text))
            {
                IPAddress address;
                if (!IPAddress.TryParse(textServer.Text, out address))
                    msg = MMCUIConstants.INVALID_IP;
            }

            if (msg != null)
            {
                MMCDlgHelper.ShowWarning(msg);
                return false;
            }
            return true;
        }

        private void buttonOk_Click(object sender, EventArgs e)
        {
            if (!ValidateForm())
            {
                this.DialogResult = DialogResult.None;
                return;
            }
            Server = (textServer.Text).Trim();
            UserName = (textUser.Text).Trim();
            Password = textPassword.Text;
            DomainName = (textTenant.Text).Trim();
            this.Close();
        }

        private void LoginForm_Load(object sender, EventArgs e)
        {
            this.textServer.Text = Server;
            this.textUser.Text = UserName;
            if(!String.IsNullOrEmpty(DomainName))
                this.textTenant.Text = DomainName;
            if (!String.IsNullOrEmpty(Server))
                this.textServer.Enabled = false;
        }
    }
}

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
        public LoginForm(string server, string user, string password,string domain):this()
        {
            this.Server = server;
            this.UserName = user;
            this.Password = password;
            this.DomainName = domain;
        }

        private void buttonOk_Click(object sender, EventArgs e)
        {
            Server = textServer.Text;
            UserName = textUser.Text;
            Password = textPassword.Text;
            DomainName = textTenant.Text;
            this.Close();
        }

        void EnableOk()
        {
            buttonOk.Enabled = textServer.Text.Length * textUser.Text.Length * textPassword.Text.Length * textTenant .Text.Length > 0;
        }

        private void textServer_Changed(object sender, EventArgs e)
        {
            EnableOk();
        }
        private void textUser_Changed(object sender, EventArgs e)
        {
            EnableOk();
        }
        private void textPassword_Changed(object sender, EventArgs e)
        {
            EnableOk();
        }
        private void textTenant_Changed(object sender, EventArgs e)
        {
            EnableOk();
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

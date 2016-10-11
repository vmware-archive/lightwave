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

namespace VMwareMMCIDP.UI.Common
{
    public partial class SelectComputerUI : Form
    {
        public string ServerName { get; set; }

        public SelectComputerUI()
        {
            InitializeComponent();

            textBoxRemote.Select();
        }

        private void radioButtonRemote_CheckedChanged(object sender, EventArgs e)
        {
            this.textBoxRemote.Enabled = this.radioButtonRemote.Checked;
            this.textBoxRemote.ReadOnly = !this.textBoxRemote.Enabled;
        }

        private void InitializeSnapin_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (this.radioButtonLocal.Checked)
            {
                this.ServerName = "localhost";
            }
            else if (this.radioButtonRemote.Checked)
            {
                this.ServerName = this.textBoxRemote.Text;
            }
        }
    }
}

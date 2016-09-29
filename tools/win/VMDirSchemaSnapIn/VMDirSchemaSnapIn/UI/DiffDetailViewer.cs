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

namespace VMDirSchemaSnapIn.UI
{
    public partial class DiffDetailViewer : Form
    {
        private string baseText;
        private string currentText;
        private string baseServer;
        private string currentServer;

        public DiffDetailViewer(string baseServer, string currentServer, string baseText, string currentText)
        {
            InitializeComponent();
            this.baseText = baseText;
            this.currentText = currentText;
            this.baseServer = VMDirSchemaConstants.BASE_TITLE + baseServer;
            this.currentServer = VMDirSchemaConstants.CURRENT_TITLE + currentServer;

            baseText = baseText.Replace(":", Environment.NewLine);
            currentText = currentText.Replace(":", Environment.NewLine);
            this.BaseTextView.Text = baseText;
            this.CurrentTextView.Text = currentText;
           // BaseLabel.StringValue = baseServer;
           // CurrentLabel.StringValue = currentServer;
        }

        private void CloseButtonClick(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            this.Close();
        }
    }
}

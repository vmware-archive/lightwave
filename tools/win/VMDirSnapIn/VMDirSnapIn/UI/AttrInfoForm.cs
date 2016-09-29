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
using System.Diagnostics;
using System.Text;
using System.Windows.Forms;
using VMDir.Common.DTO;
using VMDirSnapIn.Utilities;

namespace VMDirSnapIn.UI
{
    public partial class AttrInfoForm : Form
    {
        AttributeHelpDTO helpDTO;
        public AttrInfoForm(AttributeHelpDTO helpDTO)
        {
            this.helpDTO = helpDTO;
            InitializeComponent();
            BindUI();
        }

        private void BindUI()
        {
            this.labelSyntax.Text = string.Empty;
            this.textBoxEx.Text = string.Empty;
            linkLabel1.Enabled = false;
            this.textBoxEx.Enabled = false;

            if (helpDTO != null)
            {
                this.labelSyntax.Text = helpDTO.Value;
                StringBuilder sb = new StringBuilder();
                if (helpDTO.ExampleList != null)
                {
                    foreach (var str in helpDTO.ExampleList)
                        sb.Append(str + Environment.NewLine);
                    this.textBoxEx.Text = sb.ToString();
                    this.textBoxEx.Enabled = true;
                }
                if (!string.IsNullOrWhiteSpace(helpDTO.HelpLink))
                {
                    LinkLabel.Link link = new LinkLabel.Link();
                    link.LinkData = helpDTO.HelpLink;
                    linkLabel1.Links.Clear();
                    linkLabel1.Links.Add(link);
                    linkLabel1.Enabled = true;
                }
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            MiscUtilsService.CheckedExec(delegate
            {
                Process.Start(e.Link.LinkData as string);
            });

        }
    }
}

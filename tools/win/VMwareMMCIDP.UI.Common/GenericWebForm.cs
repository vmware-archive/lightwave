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
 */using System;using System.IO;using System.Windows.Forms;
namespace VMwareMMCIDP.UI{    public partial class GenericWebForm : Form    {        string _text;
        public GenericWebForm(string title, string xml)        {            InitializeComponent();
            this.Text = title;
            _text = xml;            //string file = Path.Combine(SSOAdminSnapInEnvironment.Instance.ApplicationPath, "temp.xml");            //File.WriteAllText(file, _text);            //webBrowser1.Navigate(file);        }
        private void btnExport_Click(object sender, EventArgs e)        {            using (var sfd = new SaveFileDialog())            {                if (sfd.ShowDialog() == DialogResult.OK)                {                    string fileName = sfd.FileName;                    string ext = Path.GetExtension(fileName);                    if (string.IsNullOrEmpty(ext))                        fileName += ".xml";
                    File.WriteAllText(fileName, _text);                }            }        }
        private void btnCopyToClipboard_Click(object sender, EventArgs e)        {            Clipboard.SetText(_text);        }    }}
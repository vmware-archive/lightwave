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

namespace VMDirSnapIn.UI
{
    public partial class SubmitModConfirm : Form
    {
        private Dictionary<string, List<string>> _modifications;
        public SubmitModConfirm(Dictionary<string, List<string>> modifications)
        {
            _modifications = modifications;
            InitializeComponent();
        }
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            foreach (var item in _modifications)
            {
                ListViewItem lvi = new ListViewItem(item.Key);
                this.listView1.Items.Add(lvi);
            }
        }
        private void buttonYes_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void buttonNo_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}

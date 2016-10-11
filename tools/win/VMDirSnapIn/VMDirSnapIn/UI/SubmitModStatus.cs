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
using VMDir.Common.DTO;

namespace VMDirSnapIn.UI
{
    public partial class SubmitModStatus : Form
    {
        private List<AttributeModStatus> _modificationStatus;

        public SubmitModStatus(List<AttributeModStatus> modificationStatus)
        {
            // TODO: Complete member initialization
            this._modificationStatus = modificationStatus;
            InitializeComponent();
        }
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            foreach (var item in _modificationStatus)
            {
                ListViewItem lvi = new ListViewItem(new string[] { item.AttributeName, item.ErrorMsg });
                lvi.ForeColor = Color.White;
                if(item.ModStatus)
                    lvi.BackColor=Color.Green;
                else
                    lvi.BackColor=Color.Red;
                this.listView1.Items.Add(lvi);
            }
        }

        private void buttonOk_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}

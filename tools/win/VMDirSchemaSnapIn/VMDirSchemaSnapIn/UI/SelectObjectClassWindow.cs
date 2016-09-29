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
    public partial class SelectObjectClassWindow : Form
    {
        private List<string> ItemsToSelect;
        private List<string> AllItems;

        public string SelectedItem { get; set; }

        public SelectObjectClassWindow(List<string> items)
        {
            InitializeComponent();
            AllItems = items;
            ItemsToSelect = items;
            this.FromItemsList.DataSource = ItemsToSelect;
        }


        private void SelectButton_Click(object sender, EventArgs e)
        {
            if ((int)this.FromItemsList.SelectedIndex >= 0)
            {
                SelectedItem = this.FromItemsList.SelectedValue.ToString();
                this.DialogResult = DialogResult.OK;
                this.Close();
            }
        }

        private void SearchButton_Click(object sender, EventArgs e)
        {
            if (!string.IsNullOrWhiteSpace(this.SearchTextBox.Text))
            {
               ItemsToSelect = ItemsToSelect.FindAll(p => p.StartsWith(this.SearchTextBox.Text));
               this.FromItemsList.DataSource = null;
               this.FromItemsList.DataSource = ItemsToSelect;
               this.FromItemsList.Refresh();
            }
        }

        private void Reset_Click(object sender, EventArgs e)
        {
            ItemsToSelect = AllItems;
            this.SearchTextBox.Text = string.Empty;
            this.FromItemsList.DataSource = null;
            this.FromItemsList.DataSource = ItemsToSelect;
            this.FromItemsList.Refresh();
        }
    }
}

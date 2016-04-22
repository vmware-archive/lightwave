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
using System.Linq;
using System.Windows.Forms;
using VMDir.Common.DTO;
using VMDirSnapIn.Utilities;
using VMDir.Common.Schema;

namespace VMDirSnapIn.UI
{
    public partial class AddOrRemoveAttributes : Form
    {
        string _objectClass;

        VMDirServerDTO _serverDTO;

        List<KeyValuePair<string, string>> rhsList = new List<KeyValuePair<string, string>>();

        List<KeyValuePair<string, string>> lhsList = new List<KeyValuePair<string, string>>();

        Dictionary<string, AttributeTypeDTO> attrDict = new Dictionary<string, AttributeTypeDTO>();

        public List<KeyValuePair<string, string>> NewOptionalAttributes { get { return rhsList; } }

        public AddOrRemoveAttributes(string objectClass, IEnumerable<KeyValuePair<string, string>> existingAttributes, VMDirServerDTO serverDTO)
        {
            _objectClass = objectClass;
            _serverDTO = serverDTO;
            InitializeComponent();
            Text = "Manage attributes for " + _objectClass;
            Bind(existingAttributes);
        }

        int ListSort(KeyValuePair<string, string> lhs, KeyValuePair<string, string> rhs)
        {
            return lhs.Key.CompareTo(rhs.Key);
        }

        void Bind(IEnumerable<KeyValuePair<string, string>> existingAttributes)
        {
            var attrList = _serverDTO.Connection.SchemaManager.GetOptionalAttributes(_objectClass);
            foreach (var item in attrList)
            {
                if (!attrDict.ContainsKey(item.Name))
                {
                    attrDict.Add(item.Name, item);
                }
            }
            lhsList = attrDict.Select(x => new KeyValuePair<string, string>(x.Key, x.Value.Description)).ToList();
            var currentAttributes = existingAttributes.ToList();
            foreach (var item in currentAttributes)
            {
                if (isSingleValue(item.Key))
                    lhsList.Remove(item);
            }

            lhsList.Sort(ListSort);
            bindListView();
        }

        private bool isSingleValue(string key)
        {
            if (attrDict.ContainsKey(key))
                return attrDict[key].SingleValue;
            else
                return false;
        }

        private void bindListView()
        {
            lstNewAttributes.Items.Clear();
            lstExistingAttributes.Items.Clear();
            foreach (var item in lhsList)
            {
                ListViewItem lvi = new ListViewItem(new[] { item.Key, item.Value });
                lstNewAttributes.Items.Add(lvi);
            }
            foreach (var item in rhsList)
            {
                ListViewItem lvi = new ListViewItem(new[] { item.Key, item.Value });
                lstExistingAttributes.Items.Add(lvi);
            }
        }

        private void lstNewAttributes_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnAddSingle.Enabled = lstNewAttributes.SelectedIndices.Count > 0;
        }

        private void lstExistingAttributes_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnRemoveSingle.Enabled = lstExistingAttributes.SelectedIndices.Count > 0;
        }

        private void btnAddSingle_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in lstNewAttributes.SelectedItems)
            {
                var kv = new KeyValuePair<string, string>(item.SubItems[0].Text, item.SubItems[1].Text);
                rhsList.Add(kv);

                if (isSingleValue(kv.Key))
                    lhsList.Remove(kv);
            }
            bindListView();
        }

        private void btnRemoveSingle_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in lstExistingAttributes.SelectedItems)
            {
                var kv = new KeyValuePair<string, string>(item.SubItems[0].Text, item.SubItems[1].Text);
                if (isSingleValue(kv.Key))
                    lhsList.Add(kv);
                rhsList.Remove(kv);
            }
            bindListView();
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
            this.Close();
        }

        private void lstNewAttributes_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            btnAddSingle_Click(lstNewAttributes, EventArgs.Empty);
        }

        private void lstExistingAttributes_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            btnRemoveSingle_Click(lstExistingAttributes, EventArgs.Empty);
        }

        private void lstNewAttributes_OnKeyPress(object sender, KeyPressEventArgs e)
        {
            SearchListByKeyPress.findAndSelect((ListView)sender, e.KeyChar);
        }
    }
}

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

namespace VMDirSnapIn.UI
{
    public partial class AddOrRemoveAttributes : Form
    {
        string _objectClass;
        VMDirServerDTO _serverDTO;

        List<KeyValuePair<string, string>> rhsList = new List<KeyValuePair<string, string>>();
        List<KeyValuePair<string, string>> lhsList = new List<KeyValuePair<string, string>>();

        public List<KeyValuePair<string, string>> OptionalAttributes { get { return rhsList; } }


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



        void Bind(IEnumerable<KeyValuePair<string, string>> currentAttributes)
        {

            rhsList = currentAttributes.ToList();

            lhsList = _serverDTO.Connection.SchemaManager.GetOptionalAttributes(_objectClass)

                .Select(x => new KeyValuePair<string, string>(x.Name, x.Description)).ToList();

            foreach (var item in rhsList)
            {

                lhsList.Remove(item);

            }

            lhsList.Sort(ListSort);



            RefreshData();

        }



        void RefreshData()
        {

            lstNewAttributes.VirtualListSize = lhsList.Count;

            lstExistingAttributes.VirtualListSize = rhsList.Count;

        }



        private void lstNewAttributes_SelectedIndexChanged(object sender, EventArgs e)
        {

            btnAddSingle.Enabled = lstNewAttributes.SelectedIndices.Count > 0;

        }



        private void lstExistingAttributes_SelectedIndexChanged(object sender, EventArgs e)
        {

            btnRemoveSingle.Enabled = lstExistingAttributes.SelectedIndices.Count > 0;

        }



        private void lstNewAttributes_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
        {

            var dto = lhsList[e.ItemIndex];

            e.Item = new ListViewItem(new string[] { dto.Key, dto.Value });

        }



        private void lstExistingAttributes_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
        {

            var dto = rhsList[e.ItemIndex];

            e.Item = new ListViewItem(new string[] { dto.Key, dto.Value });

        }



        private void btnAddSingle_Click(object sender, EventArgs e)
        {

            foreach (int index in lstNewAttributes.SelectedIndices)
            {

                rhsList.Add(lhsList[index]);

                lhsList.RemoveAt(index);

            }

            RefreshData();

        }



        private void btnAddAll_Click(object sender, EventArgs e)
        {

            rhsList.AddRange(lhsList);

            lhsList.Clear();

            RefreshData();

        }



        private void btnRemoveSingle_Click(object sender, EventArgs e)
        {

            foreach (int index in lstExistingAttributes.SelectedIndices)
            {

                lhsList.Add(rhsList[index]);

                rhsList.RemoveAt(index);

            }

            RefreshData();

        }



        private void btnRemoveAll_Click(object sender, EventArgs e)
        {

            lhsList.AddRange(rhsList);

            rhsList.Clear();

            RefreshData();

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

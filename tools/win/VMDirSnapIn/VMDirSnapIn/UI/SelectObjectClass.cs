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
using System.Linq;
using System.Windows.Forms;
using VMDir.Common.Schema;
using VMDirSnapIn.Utilities;

namespace VMDirSnapIn.UI
{

    public partial class SelectObjectClass : Form
    {
        List<ObjectClassDTO> _list;
        string _selectedObject;
        public string SelectedObject { get { return _selectedObject; } }
        private List<ListViewItem> _lviList;
        public SelectObjectClass(SchemaManager mgr)
        {
            InitializeComponent();
            _lviList = new List<ListViewItem>();
            BindList(mgr);
        }

        int SortObjectClassDTO(ObjectClassDTO lhs, ObjectClassDTO rhs)
        {
            return lhs.Name.CompareTo(rhs.Name);
        }

        void BindList(SchemaManager mgr)
        {
            var om = mgr.GetObjectClassManager();
            //Todo - list all classes now. Later fix to only list structural object classes after introducing specific fields in Schema classes.
            _list = om.Data.Values.ToList();
            _list.Sort(SortObjectClassDTO);
            ResetList();
        }
        private void ResetList()
        {
            lstObjectClasses.Items.Clear();
            _lviList.Clear();
            foreach (var item in _list)
            {
               ListViewItem lvi= new ListViewItem(new string[] { item.Name, item.Description });
               _lviList.Add(lvi);
            }
            lstObjectClasses.Items.AddRange(_lviList.ToArray());
        }
        private void btnOK_Click(object sender, EventArgs e)
        {
            if (lstObjectClasses.SelectedIndices.Count == 1)
            {
                _selectedObject = lstObjectClasses.SelectedItems[0].SubItems[0].Text;
                DialogResult = DialogResult.OK;
                this.Close();
            }
        }
        void textBoxSearch_TextChanged(object sender, System.EventArgs e)
        {
            if(string.IsNullOrWhiteSpace(textBoxSearch.Text))
                ResetList();
            else
            {
                lstObjectClasses.Items.Clear();
                _lviList.Clear();
                foreach (var item in _list)
                {
                    if (item.Name.StartsWith(textBoxSearch.Text,StringComparison.OrdinalIgnoreCase))
                    {
                        ListViewItem lvi = new ListViewItem(new string[] { item.Name, item.Description });
                        _lviList.Add(lvi);
                    }
                }
                lstObjectClasses.Items.AddRange(_lviList.ToArray());
            }
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            textBoxSearch.Text = string.Empty;
            ResetList();
        }
    }
}

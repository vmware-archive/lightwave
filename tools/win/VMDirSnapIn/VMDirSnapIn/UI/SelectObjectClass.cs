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
using VMDir.Common.Schema;
using VMDirSnapIn.Utilities;

namespace VMDirSnapIn.UI
{

    public partial class SelectObjectClass : Form
    {
        List<ObjectClassDTO> _list;
        ObjectClassDTO _selectedObject;
        public ObjectClassDTO SelectedObject { get { return _selectedObject; } }

        public SelectObjectClass(SchemaManager mgr)
        {
            InitializeComponent();
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

            lstObjectClasses.VirtualListSize = _list.Count;
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            if (lstObjectClasses.SelectedIndices.Count == 1)
            {
                _selectedObject = _list[lstObjectClasses.SelectedIndices[0]];
                DialogResult = DialogResult.OK;
                this.Close();
            }
        }

        private void lstObjectClasses_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
        {
            var dto = _list[e.ItemIndex];
            e.Item = new ListViewItem(new string[] { dto.Name, dto.Description });
        }

        private void lstObjectClasses_KeyPress(object sender, KeyPressEventArgs e)
        {
            SearchListByKeyPress.findAndSelect((ListView)sender, e.KeyChar);
        }

    }
}

/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class SelectGroups : Form
    {
        private readonly IPropertyDataManager _dataMgr;
        public SelectGroups(IPropertyDataManager mgr)
        {
            _dataMgr = mgr;
            InitializeComponent();

            BindDataAllGroups();
        }

        void BindDataAllGroups()
        {
            var groups = _dataMgr.GetData() as IList<GroupDto>;
            lstParentGroups.Items.Clear();
            if (groups != null)
            {
                foreach (var group in groups)
                {
                    var item = new ListViewItem(@group.GroupName) { Tag = @group, ImageIndex = (int)TreeImageIndex.Group };
                    lstParentGroups.Items.Add(item);
                }
            }
        }
        private void btnAdd_Click(object sender, EventArgs e)
        {
            _dataMgr.Apply(GetSelectedGroups());
            DialogResult = DialogResult.OK;
        }

        private void lstParentGroups_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnAdd.Enabled = lstParentGroups.SelectedIndices.Count > 0;
        }

        List<GroupDto> GetSelectedGroups()
        {
            return (from ListViewItem item in lstParentGroups.SelectedItems select item.Tag as GroupDto).ToList();
        }
    }
}

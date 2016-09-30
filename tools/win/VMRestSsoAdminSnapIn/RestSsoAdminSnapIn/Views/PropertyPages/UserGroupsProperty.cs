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

using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.User;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages
{
    public partial class UserGroupsProperty : UserControl
    {
        GenericPropertyPage _parent;
        private readonly bool _isSystemDomain;
        public GenericPropertyPage Page { get { return _parent; } }
        public string Title
        {
            get { return _parent.Title; }
            set { _parent.Title = value; }
        }

        readonly IPropertyDataManager _dataManager;
        List<GroupDto> _groupsDto;
        public UserGroupsProperty(IPropertyDataManager mgr, bool systemDomain)
        {
            _dataManager = mgr;
            InitializeComponent();
            _isSystemDomain = systemDomain;
            PropertyPageInit();
            BindControls();
            SetVisibility(systemDomain);
        }

        void PropertyPageInit()
        {
            var il = new ImageList();            il.Images.AddStrip(ResourceHelper.GetToolbarImage());            lstMemberOfGroups.SmallImageList = il;

            _parent = new GenericPropertyPage {Control = this};
            _parent.Apply += _parent_Apply;
        }
        private void SetVisibility(bool show)
        {
            btnAdd.Visible = show;
            btnRemove.Visible = show;
        }
        void _parent_Apply(object sender, CancelEventArgs e)
        {
            var success = _dataManager.Apply(_groupsDto);
            e.Cancel = !success;
            if (success)
            {
                BindControls();
            }
        }

        void BindControls()
        {
            _groupsDto = _dataManager.GetData() as List<GroupDto>;
            FillList();
        }

        void FillList()
        {
            lstMemberOfGroups.Items.Clear();
            foreach (var group in _groupsDto)
                lstMemberOfGroups.Items.Add(group.GroupName, (int)TreeImageIndex.Group);
        }

        private void lstMemberOfGroups_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            bool enabled = lstMemberOfGroups.SelectedItems.Count > 0;
            btnRemove.Enabled = enabled;
        }

        private void btnAdd_Click(object sender, System.EventArgs e)
        {
            var count = _groupsDto.Count;
            var groupsPropertyDataMgr = _dataManager as UserGroupsPropertyManager;
            if (groupsPropertyDataMgr != null)
            {
                var addedGroups = groupsPropertyDataMgr.AddGroups(Page.ParentSheet);
                if (addedGroups != null)
                {
                    addedGroups.ForEach(x =>
                    {
                        if (!_groupsDto.Exists(y => y.GroupName == x.GroupName))
                            _groupsDto.Add(x);
                    });
                }
            }
            if (count != _groupsDto.Count)
            {
                FillList();
                Page.Dirty = true;
            }

        }

        private void btnRemove_Click(object sender, System.EventArgs e)
        {
            int count = _groupsDto.Count;
            foreach (ListViewItem item in lstMemberOfGroups.SelectedItems)
            {
                var dto = _groupsDto.Find(x => x.GroupName == item.Text);
                if (dto != null)
                    _groupsDto.Remove(dto);
            }
            if (count != _groupsDto.Count)
            {
                FillList();
                Page.Dirty = true;
            }
        }
    }
}

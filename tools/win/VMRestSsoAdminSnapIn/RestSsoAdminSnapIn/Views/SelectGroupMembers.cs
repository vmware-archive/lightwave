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
using System.Linq;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.Groups;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class SelectGroupMembers : Form, IView
    {
        private readonly GroupPropertyDataManager _presenter;
        private GroupMembershipDto _groupMembershipDto;
        public SelectGroupMembers(GroupPropertyDataManager presenter)
        {
            _presenter = presenter;
            InitializeComponent();

            var il = new ImageList();
            il.Images.AddStrip(ResourceHelper.GetToolbarImage());
            lstGroupMembers.SmallImageList = il;
            BindData();
        }

        void BindData()
        {
            var membershipInfo = _presenter.GetAllMembers();
            lstGroupMembers.Items.Clear();

            if (membershipInfo.Users != null)
            {
                foreach (var member in membershipInfo.Users)
                {
                    var item = new ListViewItem(member.Name, 4) { Tag = member, ImageIndex = (int)TreeImageIndex.User };
                    lstGroupMembers.Items.Add(item);
                }
            }

            if (membershipInfo.SolutionUsers != null)
            {
                foreach (var member in membershipInfo.SolutionUsers)
                {
                    var item = new ListViewItem(member.Name, 4) { Tag = member, ImageIndex = (int)TreeImageIndex.SolutionUser };
                    lstGroupMembers.Items.Add(item);
                }
            }

            if (membershipInfo.Groups != null)
            {
                foreach (var member in membershipInfo.Groups)
                {
                    var item = new ListViewItem(member.GroupName, 5) { Tag = member, ImageIndex = (int)TreeImageIndex.Group };
                    lstGroupMembers.Items.Add(item);
                }
            }
        }
        private void btnAdd_Click(object sender, EventArgs e)
        {
            GetSelectedMembers();
            DialogResult = DialogResult.OK;
        }

        private void lstParentGroups_SelectedIndexChanged(object sender, EventArgs e)
        {
            btnAdd.Enabled = lstGroupMembers.SelectedIndices.Count > 0;
        }

        private void GetSelectedMembers()
        {
            _groupMembershipDto = new GroupMembershipDto();

            foreach (ListViewItem item in lstGroupMembers.SelectedItems)
            {
                if (item.Tag.GetType() == typeof(UserDto))
                {
                    var user = ((UserDto)item.Tag);
                    _groupMembershipDto.Users.Add(user);
                }

                if (item.Tag.GetType() == typeof(SolutionUserDto))
                {
                    var user = ((SolutionUserDto)item.Tag);
                    _groupMembershipDto.SolutionUsers.Add(user);
                }

                if (item.Tag.GetType() == typeof(GroupDto))
                {
                    var group = ((GroupDto)item.Tag);
                    _groupMembershipDto.Groups.Add(group);
                }
            }
        }

        public IDataContext DataContext
        {
            get
            {
                return _groupMembershipDto;
            }
        }
    }
}

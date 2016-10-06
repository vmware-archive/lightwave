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
        private enum MemberTypeFilter { All = 0, Users, Groups };
        public SelectGroupMembers(GroupPropertyDataManager presenter)
        {
            _presenter = presenter;
            InitializeComponent();
        }

        private void SearchMembers(string name, string domain, MemberTypeFilter filter)
        {
            lstGroupMembers.Items.Clear();

            if (filter != MemberTypeFilter.Groups)
            {
                var users = _presenter.GetUsers(name, domain);

                if (users != null)
                {
                    foreach (var member in users)
                    {
                        var item = new ListViewItem(member.Name, 4) { Tag = member, ImageIndex = (int)TreeImageIndex.User };
                        lstGroupMembers.Items.Add(item);
                    }
                }

            }

            if (filter != MemberTypeFilter.Users)
            {
                var groups = _presenter.GetGroups(name, domain);
                if (groups != null)
                {
                    foreach (var member in groups)
                    {
                        var item = new ListViewItem(member.GroupName, 5) { Tag = member, ImageIndex = (int)TreeImageIndex.Group };
                        lstGroupMembers.Items.Add(item);
                    }
                }
            }

            lblWarning.Text = string.Format("Warning: Showing top {0} results. Please specify a more specific search criteria..", lstGroupMembers.Items.Count);
            lblWarning.Visible = lstGroupMembers.Items.Count > 100;

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

        private void SelectGroupMembers_Load(object sender, EventArgs e)
        {
            var il = new ImageList();
            il.Images.AddStrip(ResourceHelper.GetToolbarImage());
            lstGroupMembers.SmallImageList = il;

            var domains = _presenter.GetDomains();
            cbDomain.DataSource = domains;
            cbDomain.DisplayMember = "Name";
            cbDomain.ValueMember = "Name";
            if (domains.Count > 0)
            {
                cbDomain.SelectedIndex = 0;
                cbMemberType.SelectedIndex = 0;
                SearchMembers(null, domains[0].Name, MemberTypeFilter.All);
            }
            else
            {
                btnSearch.Enabled = false;
                cbMemberType.Enabled = false;
            }
        }

        private void btnSearch_Click(object sender, EventArgs e)
        {
            if (cbMemberType.SelectedIndex > -1 && cbDomain.SelectedIndex > -1)
            {
                var domain = cbDomain.SelectedValue.ToString();
                var type = (MemberTypeFilter)cbMemberType.SelectedIndex;
                SearchMembers(txtMemberName.Text, domain, type);
            }
        }
    }
}

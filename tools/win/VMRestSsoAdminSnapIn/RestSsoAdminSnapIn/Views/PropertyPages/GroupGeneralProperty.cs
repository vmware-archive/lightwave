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
using System.Linq;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.Groups;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages
{
    public partial class GroupGeneralProperty : UserControl
    {
        private GenericPropertyPage _parent;
        private readonly GroupPropertyDataManager _presenter;
        private readonly bool _isSystemDomain;
        private GroupDto _groupDto;
        private GroupMembershipDto _groupMembershipDto;

        public GenericPropertyPage Page { get { return _parent; } }
        public string Title
        {
            get { return _parent.Title; }
            set { _parent.Title = value; }
        }

        public GroupGeneralProperty(GroupPropertyDataManager presenter, GroupDto groupDto, bool systemDomain)
        {
            _presenter = presenter;
            _groupMembershipDto = _presenter.GetMembershipInfo();
            _groupDto = groupDto;
            _isSystemDomain = systemDomain;
            InitializeComponent();
            PropertyPageInit();
            SetVisibility(systemDomain);
        }

        private void SetVisibility(bool show)
        {
            btnAdd.Visible = show;
            btnRemove.Visible = show;
        }

        void PropertyPageInit()
        {
            var il = new ImageList();
            il.Images.AddStrip(ResourceHelper.GetToolbarImage());
            lstGroupMembers.SmallImageList = il;

            _parent = new GenericPropertyPage { Control = this };
            _parent.Apply += _parent_Apply;
            _parent.Initialize += _parent_Initialize;
        }

        void _parent_Initialize(object sender, EventArgs e)
        {
            BindControls();
        }

        void _parent_Apply(object sender, CancelEventArgs e)
        {
            _groupDto.GroupDetails.Description = txtDescription.Text;
            var success = _presenter.UpdateMembers(_groupMembershipDto);
            _groupDto = _presenter.Update(_groupDto);
            e.Cancel = !success;
            _groupMembershipDto = _presenter.GetMembershipInfo();
            RefreshMembers();
        }

        void BindControls()
        {
            Cursor.Current = Cursors.WaitCursor;
            lblGroupName.Text = _groupDto.GroupName;
            txtDescription.Text = _groupDto.GroupDetails.Description;
            RefreshMembers();
            Cursor.Current = Cursors.Default;
        }

        void RefreshMembers()
        {
            lstGroupMembers.Items.Clear();
            if (_groupMembershipDto.Groups == null)
                _groupMembershipDto.Groups = new List<GroupDto>();

            if (_groupMembershipDto.Users == null)
                _groupMembershipDto.Users = new List<UserDto>();

            if (_groupMembershipDto.SolutionUsers == null)
                _groupMembershipDto.SolutionUsers = new List<SolutionUserDto>();

            foreach (var member in _groupMembershipDto.Groups.Where(x=>x.State != State.ForDelete))
            {
                var item = new ListViewItem { Text = member.GroupName, Tag = member, ImageIndex = (int)TreeImageIndex.Group };
                item.SubItems.Add(member.GroupDomain);
                lstGroupMembers.Items.Add(item);
            }
            foreach (var member in _groupMembershipDto.Users.Where(x => x.State != State.ForDelete))
            {
                var item = new ListViewItem { Text = member.Name, Tag = member, ImageIndex = (int)TreeImageIndex.User };
                item.SubItems.Add(member.Domain);
                lstGroupMembers.Items.Add(item);
            }

            foreach (var member in _groupMembershipDto.SolutionUsers.Where(x => x.State != State.ForDelete))
            {
                var item = new ListViewItem { Text = member.Name, Tag = member, ImageIndex = (int)TreeImageIndex.SolutionUser };
                item.SubItems.Add(member.Domain);
                lstGroupMembers.Items.Add(item);
            }
        }

        private void lstGroupMembers_SelectedIndexChanged(object sender, EventArgs e)
        {
            bool enabled = lstGroupMembers.SelectedItems.Count > 0;
            btnRemove.Enabled = enabled;
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            Cursor.Current = Cursors.WaitCursor;
            var count = lstGroupMembers.Items;
            GroupMembershipDto groupMembershipDto = null;
            if (_presenter != null)
            {
                ActionHelper.Execute(delegate
                {
                    var frm = new SelectGroupMembers(_presenter);
                    var dataContext = SnapInContext.Instance.NavigationController.NavigateToView(Page.ParentSheet, frm);
                    groupMembershipDto = (GroupMembershipDto)dataContext;
                }, null);
                if (groupMembershipDto != null)
                {
                    foreach (var member in groupMembershipDto.Groups)
                    {
                        var principalName = member.GroupName + "@" + member.GroupDomain;
                        if (!_groupMembershipDto.Groups.Exists(y => y.GroupName + "@" + y.GroupDomain == principalName))
                        {
                            member.State = State.ForAdd;
                            _groupMembershipDto.Groups.Add(member);
                        }
                    }
                    foreach (var member in groupMembershipDto.Users)
                    {
                        var principalName = member.Name + "@" + member.Domain;
                        if (!_groupMembershipDto.Users.Exists(y => y.Name + "@" + y.Domain == principalName))
                        {
                            member.State = State.ForAdd;
                            _groupMembershipDto.Users.Add(member);
                        }
                    }

                    foreach (var member in groupMembershipDto.SolutionUsers)
                    {
                        var principalName = member.Name + "@" + member.Domain;
                        if (!_groupMembershipDto.SolutionUsers.Exists(y => y.Name + "@" + y.Domain == principalName))
                        {
                            member.State = State.ForAdd;
                            _groupMembershipDto.SolutionUsers.Add(member);
                        }
                    }
                }
            }
            Page.Dirty = true;
            RefreshMembers();
            Cursor.Current = Cursors.Default;
        }

        private void btnRemove_Click(object sender, EventArgs e)
        {
            Page.Dirty = true;
            foreach (ListViewItem item in lstGroupMembers.SelectedItems)
            {
                if (item.Tag.GetType() == typeof(UserDto))
                {
                    var user = ((UserDto)item.Tag);
                    var dto = _groupMembershipDto.Users.Find(x => x.Name == user.Name && x.Domain == user.Domain);
                    if (dto != null)
                        dto.State = State.ForDelete;
                }

                if (item.Tag.GetType() == typeof(SolutionUserDto))
                {
                    var user = ((SolutionUserDto)item.Tag);
                    var dto = _groupMembershipDto.SolutionUsers.Find(x => x.Name == user.Name && x.Domain == user.Domain);
                    if (dto != null)
                        dto.State = State.ForDelete;
                }

                if (item.Tag.GetType() == typeof(GroupDto))
                {
                    var group = ((GroupDto)item.Tag);
                    var dto = _groupMembershipDto.Groups.Find(x => x.GroupName == group.GroupName && x.GroupDomain == group.GroupDomain);
                    if (dto != null)
                        dto.State = State.ForDelete;
                }
            }
            RefreshMembers();
        }

        private void txtDescription_TextChanged(object sender, EventArgs e)
        {
            Page.Dirty = true;
        }
    }
}

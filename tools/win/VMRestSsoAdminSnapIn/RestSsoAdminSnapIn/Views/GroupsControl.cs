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
using Microsoft.ManagementConsole;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class GroupsControl : UserControl, IFormViewControl
    {
        GroupsFormView _formView;
        IList<GroupDto> _groups = new List<GroupDto>();
        private ServiceGateway _service;
        public GroupsControl()
        {
            InitializeComponent();
        }
        private bool IsSystemDomain
        {
            get
            {
                var groupsNode = _formView.ScopeNode as GroupsNode;
                return groupsNode != null ? groupsNode.IsSystemDomain : false;
            }
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            Dock = DockStyle.Fill;
            var il = new ImageList();
            il.Images.AddStrip(ResourceHelper.GetToolbarImage());
            lstGroups.SmallImageList = il;
        }

        void IFormViewControl.Initialize(FormView parentSelectionFormView)
        {
            _formView = (GroupsFormView)parentSelectionFormView;
            _formView.SelectionData.ActionsPaneItems.Clear();
            RefreshGroups(string.Empty);
            SetContextMenu(IsSystemDomain);
        }
        private void SetContextMenu(bool show)
        {
            addToGroupToolStripMenuItem.Visible = show;
            renameToolStripMenuItem.Visible = show;
            deleteToolStripMenuItem.Visible = show;
        }
        public void RefreshGroups(string searchString)
        {
            lstGroups.Items.Clear();
            var ssoGroupsNode = _formView.ScopeNode as GroupsNode;
            var tenantName = ssoGroupsNode.TenantName;
            var server = ssoGroupsNode.ServerDto;
            var domainName = ssoGroupsNode.DomainName;
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(server, tenantName);
            ActionHelper.Execute(delegate
             {
                 var service = ScopeNodeExtensions.GetServiceGateway(server.ServerName);
                 var membershipDto = service.Tenant.Search(server, tenantName, domainName, MemberType.GROUP, SearchType.NAME, auth.Token, searchString);
                 _groups = membershipDto.Groups;
                 if (_groups != null)
                 {
                     foreach (var group in _groups)
                     {
                         var lvItem = new ListViewItem(new[] { group.GroupName, group.GroupDetails != null ? group.GroupDetails.Description : string.Empty }) { Tag = group };
                         lvItem.ImageIndex = (int)TreeImageIndex.Group;
                         lstGroups.Items.Add(lvItem);
                     }
                 }
             }, auth);
        }

        void ShowProperties()
        {
            var group = (GroupDto)lstGroups.SelectedItems[0].Tag;
            ActionHelper.Execute(delegate
            {
                _formView.ScopeNode.Tag = group;
                _formView.ScopeNode.ShowPropertySheet(group.GroupName + " properties");
            }, null);
        }

        private void addToGroupToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowProperties();
        }

        private void propertiesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowProperties();
        }

        private void contextMenuStrip1_Opening(object sender, CancelEventArgs e)
        {
            e.Cancel = lstGroups.SelectedIndices.Count == 0;
        }

        private void lstGroups_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (lstGroups.SelectedIndices.Count == 0)
                return;
            ShowProperties();
        }

        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int selectedIndex = lstGroups.SelectedIndices[0];
            if (selectedIndex == -1) return;
            Cursor.Current = Cursors.WaitCursor;
            var ssoGroupsNode = _formView.ScopeNode as GroupsNode;
            var tenantName = ssoGroupsNode.TenantName;
            var server = ssoGroupsNode.ServerDto;
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(server, tenantName);
            ActionHelper.Execute(delegate
            {
                var group = (GroupDto)lstGroups.Items[selectedIndex].Tag;
                var service = ScopeNodeExtensions.GetServiceGateway(server.ServerName);
                service.Group.Delete(server, tenantName, group, auth.Token);
                RefreshGroups(string.Empty);
            }, auth);
            Cursor.Current = Cursors.Default;

        }
    }
}

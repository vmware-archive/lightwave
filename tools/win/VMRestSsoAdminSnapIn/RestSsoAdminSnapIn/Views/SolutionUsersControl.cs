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
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using Vmware.Tools.RestSsoAdminSnapIn.Service.SolutionUser;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public sealed partial class SolutionUsersControl : UserControl, IFormViewControl
    {
        SolutionUsersFormView _formView;
        private Dto.ServerDto _serverDto;
        private AuthTokenDto _auth;

        public SolutionUsersControl()
        {
            InitializeComponent();
            Dock = DockStyle.Fill;
        }

        public Dto.ServerDto GetServerDto()
        {
            var ssoSolutionUsersNode = _formView.ScopeNode as SolutionUsersNode;
            if (ssoSolutionUsersNode != null)
                return ssoSolutionUsersNode.ServerDto;
            return null;
        }

        public string GetTenantName()
        {
            var ssoSolutionUsersNode = _formView.ScopeNode as SolutionUsersNode;
            if (ssoSolutionUsersNode != null)
                return ssoSolutionUsersNode.TenantName;
            return null;
        }

        void IFormViewControl.Initialize(FormView parentSelectionFormView)
        {
            _formView = (SolutionUsersFormView)parentSelectionFormView;
            _formView.SelectionData.ActionsPaneItems.Clear();
            _serverDto = GetServerDto();

            var ssoSolutionUsersNode = _formView.ScopeNode as SolutionUsersNode;
            _auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, ssoSolutionUsersNode.TenantName);
            RefreshUsers(string.Empty);
        }

        public void RefreshUsers(string searchString, SearchType type = SearchType.NAME)
        {
            lstUsers.Items.Clear();
            var ssoSolutionUsersNode = _formView.ScopeNode as SolutionUsersNode;
            var server = ssoSolutionUsersNode.ServerDto;
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(server, ssoSolutionUsersNode.TenantName);
            ActionHelper.Execute(delegate
            {
                var service = ScopeNodeExtensions.GetServiceGateway(server.ServerName);
                var membershipDto = service.Tenant.Search(server, ssoSolutionUsersNode.TenantName, ssoSolutionUsersNode.DomainName, MemberType.SOLUTIONUSER, type, auth.Token, searchString);
                if (membershipDto.SolutionUsers != null)
                {
                    foreach (var user in membershipDto.SolutionUsers)
                    {
                        var lvItem = new ListViewItem(new[] { user.Name, user.Description, user.Disabled ? "YES" : "NO" }) { Tag = user };
                        lvItem.ImageIndex = (int)TreeImageIndex.SolutionUser;
                        lstUsers.Items.Add(lvItem);
                    }
                }
            }, auth);
        }

        void ShowUser()
        {
            var user = lstUsers.SelectedItems[0].Tag as SolutionUserDto;

            ActionHelper.Execute(delegate
            {
                var ssoSolutionUsersNode = _formView.ScopeNode as SolutionUsersNode;
                var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                var userAccountDto = service.SolutionUser.Get(_serverDto, ssoSolutionUsersNode.TenantName, user, _auth.Token);
                if (userAccountDto != null)
                {
                    _formView.ScopeNode.Tag = userAccountDto;
                    _formView.ScopeNode.ShowPropertySheet(user + " properties");
                }
            }, _auth);
        }


        bool LoginAsSolutionUser(object obj, object objOld)
        {
            var dto = obj as SolutionUserDto;
            if (dto == null)
                return false;
            return LoginSolutionUser(dto);
        }

        bool LoginSolutionUser(SolutionUserDto dto)
        {
            return true;
        }


        private void lstUsers_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (lstUsers.SelectedIndices.Count == 0)
                return;

            ShowUser();
        }

        private void propertiesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowUser();
        }

        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (lstUsers.SelectedIndices.Count == 0)
                return;
            var user = lstUsers.SelectedItems[0].Tag as SolutionUserDto;
            string deleteUserMessage = "Delete user " + user.Name + " ? ";
            if (MMCDlgHelper.ShowConfirm(deleteUserMessage))
            {
                ActionHelper.Execute(delegate
                  {
                      Cursor.Current = Cursors.WaitCursor;
                      var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                      service.SolutionUser.Delete(_serverDto, GetTenantName(), user, _auth.Token);
                      RefreshUsers(string.Empty);
                  }, _auth);
                Cursor.Current = Cursors.Default;
            }
        }

        private void contextMenuStrip1_Opening(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (lstUsers.SelectedIndices.Count == 0)
                e.Cancel = true;
        }

        private void SolutionUsersControl_Load(object sender, EventArgs e)
        {
            var il = new ImageList();
            il.Images.AddStrip(ResourceHelper.GetToolbarImage());
            lstUsers.SmallImageList = il;
        }
    }
}

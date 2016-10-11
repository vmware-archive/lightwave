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
using System.ComponentModel;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class UsersControl : UserControl, IFormViewControl
    {
        UsersFormView _formView;

        public UsersControl()
        {
            InitializeComponent();
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            Dock = DockStyle.Fill;
            var il = new ImageList();
            il.Images.AddStrip(ResourceHelper.GetToolbarImage());
            lstUsers.SmallImageList = il;
        }

        string GetDomainName()
        {
            var ssoUsersNode = _formView.ScopeNode as UsersNode;
            return ssoUsersNode != null ? ssoUsersNode.DomainName : null;
        }

        string GetTenantName()
        {
            var ssoUsersNode = _formView.ScopeNode as UsersNode;
            return ssoUsersNode != null ? ssoUsersNode.TenantName : null;
        }

        Dto.ServerDto GetServerDto()
        {
            var ssoUsersNode = _formView.ScopeNode as UsersNode;
            return ssoUsersNode != null ? ssoUsersNode.ServerDto : null;
        }

        private bool IsSystemDomain
        {
            get
            {
                var ssoUsersNode = _formView.ScopeNode as UsersNode;
                return ssoUsersNode != null ? ssoUsersNode.IsSystemDomain : false;
            }
        }

        void IFormViewControl.Initialize(FormView parentSelectionFormView)
        {
            _formView = (UsersFormView)parentSelectionFormView;
            _formView.SelectionData.ActionsPaneItems.Clear();
            RefreshUsers(string.Empty);
            SetContextMenu(IsSystemDomain);
        }

        private void SetContextMenu(bool show)
        {
            setPasswordToolStripMenuItem.Visible = show;
            loginAsThisUserToolStripMenuItem.Visible = show;
            deleteToolStripMenuItem.Visible = show;
        }

        public void RefreshUsers(string searchString)
        {
            var serverDto = GetServerDto();
            var domainName = GetDomainName();
            var tenantName = GetTenantName();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenantName);
            ActionHelper.Execute(delegate
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                    var membership = service.Tenant.Search(serverDto, tenantName, domainName, MemberType.USER, SearchType.NAME, auth.Token, searchString);
                    lstUsers.Items.Clear();
                    if (membership.Users != null)
                    {
                        foreach (var user in membership.Users)
                        {
                            var lvItem =
                                new ListViewItem(new[]
                        {
                            user.Name, 
                            user.PersonDetails.FirstName, 
                            user.PersonDetails.LastName, 
                            user.PersonDetails.Description,
                            user.Locked ? "YES" : "NO", user.Disabled? "YES" : "NO"
                        }) { Tag = user };
                            lvItem.ImageIndex = (int)TreeImageIndex.User;
                            lstUsers.Items.Add(lvItem);
                        }
                    }
                }, auth);
        }

        void ShowUser()
        {
            var user = lstUsers.SelectedItems[0].Tag as UserDto;
            var serverDto = GetServerDto();
            var domain = GetDomainName();
            var tenantName = GetTenantName();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenantName);
            ActionHelper.Execute(delegate
            {
                var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                var dto = service.User.Get(serverDto, user, tenantName, auth.Token);
                if (dto != null)
                {
                    _formView.ScopeNode.Tag = dto;
                    var title = string.Format("{0}@{1} properties", user.Name, user.Domain);
                    _formView.ScopeNode.ShowPropertySheet(title);
                }
            }, auth);
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

            var user = (UserDto)lstUsers.Items[lstUsers.SelectedIndices[0]].Tag;
            string deleteUserMessage = string.Format("Are you sure, you want to delete {0} ({1}) ?","User ", user.Name);
            if (MMCDlgHelper.ShowConfirm(deleteUserMessage))
            {
                var serverDto = GetServerDto();
                var tenantName = GetTenantName();
                var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenantName);

                ActionHelper.Execute(delegate
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);
                    service.User.Delete(serverDto, tenantName, user, auth.Token);
                    RefreshUsers(string.Empty);
                }, auth);
            }
        }

        private void contextMenuStrip1_Opening(object sender, CancelEventArgs e)
        {
            if (lstUsers.SelectedIndices.Count == 0)
                e.Cancel = true;
        }


        private void ModifyUser(Action<UserDto, AuthTokenDto> action)
        {
            if (lstUsers.SelectedIndices.Count == 0)
                return;

            var userDto = (UserDto)lstUsers.Items[lstUsers.SelectedIndices[0]].Tag;
            var serverDto = GetServerDto();
            var tenantName = GetTenantName();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(serverDto, tenantName);
            var service = ScopeNodeExtensions.GetServiceGateway(serverDto.ServerName);

            ActionHelper.Execute(delegate
                 {
                     var x = service.User.Get(serverDto, userDto, tenantName, auth.Token);
                     var dto = new UserDto
                     {
                         PersonDetails = new PersonUserDto
                         {
                             FirstName = x.PersonDetails.FirstName,
                             LastName = x.PersonDetails.LastName,
                             EmailAddress = x.PersonDetails.EmailAddress,
                             Description = x.PersonDetails.Description,
                         },
                         Name = x.Name,
                         Domain = x.Domain,
                         PasswordDetails = x.PasswordDetails,
                         Disabled = x.Disabled,
                         Locked = x.Locked
                     };
                     action.Invoke(dto, auth);
                     RefreshUsers(string.Empty);
                 }, auth);
        }

        private void loginAsThisUserToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (lstUsers.SelectedIndices.Count == 0)
                return;
            var userDto = (UserDto)lstUsers.Items[lstUsers.SelectedIndices[0]].Tag;
            var ssoUsersNode = _formView.ScopeNode as UsersNode;
            if (ssoUsersNode != null)
            {
                var ssoServerNode = ssoUsersNode.Parent.Parent.Parent.Parent.Parent as ServerNode;
                if (ssoServerNode != null)
                {
                    var tenantName = GetTenantName();
                    ssoServerNode.LoginAsUser(tenantName, userDto);
                }
            }
        }
        void loginAsUser_Apply(object sender, object args)
        {
            if (lstUsers.SelectedIndices.Count == 0)
                return;
            var userDto = (UserDto)lstUsers.Items[lstUsers.SelectedIndices[0]].Tag;
            var ssoUsersNode = _formView.ScopeNode as UsersNode;
            if (ssoUsersNode != null)
            {
                var ssoServerNode = ssoUsersNode.Parent.Parent.Parent.Parent.Parent as ServerNode;
                if (ssoServerNode != null)
                {
                    var tenantName = GetTenantName();
                    ssoServerNode.LoginAsUser(tenantName, userDto);
                }
            }
        }
        private void setPasswordToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ModifyUser((userDto, auth) =>
            {
                var frm = new SetPassword(userDto);
                var dataContext = SnapInContext.Instance.NavigationController.NavigateToView(SnapInContext.Instance.SnapIn, frm);
                if (dataContext != null)
                {
                    var tenantName = GetTenantName();
                    var service = ScopeNodeExtensions.GetServiceGateway(auth.ServerDto.ServerName);
                    service.User.UpdatePassword(auth.ServerDto, tenantName, userDto, frm.PasswordResetDto, auth.Token);
                }
            });
        }

    }
}

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
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.User;
using Vmware.Tools.RestSsoAdminSnapIn.Views;
using Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes
{
    public class UsersNode : ScopeNode
    {
        public enum UsersNodeAction
        {
            ActionNewUser = 1,
            ActionFindUserByUserName = 2,
        }
        private readonly string _domainName;
        private readonly string _tenantName;
        private readonly string _systemDomain;
        private readonly bool _isSystemDomain;
        private readonly ServerDto _serverDto;

        public string TenantName { get { return _tenantName; } }
        public string DomainName { get { return _domainName; } }
        public string SystemDomain { get { return _systemDomain; } }
        public bool IsSystemDomain { get { return _isSystemDomain; } }
        public ServerDto ServerDto { get { return _serverDto; } }
        public UsersControl UsersControl { get; set; }

        public UsersNode(ServerDto dto, string tenantName, string domainName, bool systemDomain)
            : base(true)
        {
            ImageIndex = SelectedImageIndex = (int)TreeImageIndex.User;
            _serverDto = dto;
            _tenantName = tenantName;
            _domainName = domainName;
            _systemDomain = tenantName;
            _isSystemDomain = systemDomain;
            EnabledStandardVerbs = StandardVerbs.Refresh;
            if (systemDomain)
            {
                ActionsPaneItems.Add(new Action("New User", "New User", (int)TreeImageIndex.UserAdd, UsersNodeAction.ActionNewUser));
            }
            ActionsPaneItems.Add(new Action("Search", "Searches for user(s) by username", (int)TreeImageIndex.Search, UsersNodeAction.ActionFindUserByUserName));

            DisplayName = "Users";
            AddViewDescription();
        }
        protected override void OnAction(Action action, AsyncStatus status)
        {
            base.OnAction(action, status);
            switch ((UsersNodeAction)(int)action.Tag)
            {
                case UsersNodeAction.ActionNewUser:
                    AddNewUser();
                    break;
                case UsersNodeAction.ActionFindUserByUserName:
                    SearchUsers();
                    break;
            }
        }

        private void SearchUsers()
        {
            ActionHelper.Execute(delegate
            {
                var userForm = new SearchByNameView("Username");
                var context = this.GetApplicationContext();
                if (SnapIn.Console.ShowDialog(userForm) == DialogResult.OK)
                {
                    this.UsersControl.RefreshUsers(userForm.SearchString);
                }
            }, null);
        }
        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);
            DoRefresh();
        }
        void DoRefresh()
        {
            UsersControl.RefreshUsers(string.Empty);
        }
        void AddNewUser()
        {            
            ActionHelper.Execute(delegate
            {
                var userForm = new NewUserForm(_systemDomain, _domainName, _serverDto, _tenantName);
                var context = this.GetApplicationContext();
                var dataContext = context.NavigationController.NavigateToView(this, userForm);
                if (dataContext != null)
                {
                    var userDto = (UserDto)dataContext;
                    MMCDlgHelper.ShowMessage(string.Format("User {0} created successfully", userDto.Name));
                    DoRefresh();                    
                }
            }, null);
        }
        void AddViewDescription()
        {
            var fvd = new FormViewDescription
            {
                DisplayName = "Users",
                ViewType = typeof(UsersFormView),
                ControlType = typeof(UsersControl)
            };
            ViewDescriptions.Add(fvd);
            ViewDescriptions.DefaultIndex = 0;
        }
        protected override void OnAddPropertyPages(PropertyPageCollection propertyPageCollection)
        {
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);

            if (auth != null)
            {
                ActionHelper.Execute(delegate
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                    var dto = Tag as UserDto;
                    var generalManager = new UserGeneralPropertyManager(service, dto, _serverDto, _tenantName);
                    var userGeneralPage = new UserGeneralProperty(generalManager, _isSystemDomain) { Title = "General" };
                    propertyPageCollection.Add(userGeneralPage.Page);

                    var groupsManager = new UserGroupsPropertyManager(service, dto, _serverDto, _tenantName, _domainName);
                    var userGroupsPage = new UserGroupsProperty(groupsManager, _isSystemDomain) { Title = "Member Of" };
                    propertyPageCollection.Add(userGroupsPage.Page);

                    var rolesManager = new UserRolesPropertyManager(service, dto, _serverDto, _tenantName, _domainName);
                    var userRolesPage = new UserRolesProperty(rolesManager, _isSystemDomain) { Title = "Roles" };
                    propertyPageCollection.Add(userRolesPage.Page);
                }, auth);
            }
        }
    }
}

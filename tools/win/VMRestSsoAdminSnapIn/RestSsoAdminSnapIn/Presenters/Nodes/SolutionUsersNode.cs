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
using Vmware.Tools.RestSsoAdminSnapIn.Views;
using Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.SolutionUser;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes
{
    public class SolutionUsersNode : ScopeNode
    {
        public enum SolutionUsersNodeAction
        {
            ActionNewSolutionUser = 1,
            ActionFindSolutionUserByName = 2,
            ActionFindUserByCert = 3
        }
        private readonly string _tenantName;
        private readonly string _domainName;
        private readonly bool _systemDomain;
        private readonly ServerDto _serverDto;

        public string TenantName { get { return _tenantName; } }
        public string DomainName { get { return _domainName; } }
        public bool SystemDomain { get { return _systemDomain; } }
        public ServerDto ServerDto { get { return _serverDto; } }
        public SolutionUsersControl SolutionUsersControl { get; set; }


        public SolutionUsersNode(ServerDto serverDto, string tenantName, string name, bool systemDomain)
            : base(true)
        {
            ImageIndex = SelectedImageIndex = (int)TreeImageIndex.SolutionUser;
            _domainName = name;
            _systemDomain = systemDomain;
            _serverDto = serverDto;
            _tenantName = tenantName;
            EnabledStandardVerbs = StandardVerbs.Refresh;
            DisplayName = "Solution Users";
            AddActions();
            AddViewDescription();
        }

        private void AddActions()
        {
            var newUserAction = new Action("New User", "New User", (int)TreeImageIndex.UserAdd, SolutionUsersNodeAction.ActionNewSolutionUser);
            ActionsPaneItems.Add(newUserAction);
            var findByNAmeAction = new Action("Search", "Searches for solution user(s) by name", (int)TreeImageIndex.Search, SolutionUsersNodeAction.ActionFindSolutionUserByName);
            ActionsPaneItems.Add(findByNAmeAction);
            var findByCertAction = new Action("Find By Certificate", "Find By Certificate or certificate distinguished name", (int)TreeImageIndex.TrustedCertificate, SolutionUsersNodeAction.ActionFindUserByCert);
            ActionsPaneItems.Add(findByCertAction);
        }

        protected override void OnAction(Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((SolutionUsersNodeAction)(int)action.Tag)
            {
                case SolutionUsersNodeAction.ActionNewSolutionUser:
                    AddNewUser();
                    break;
                case SolutionUsersNodeAction.ActionFindUserByCert:
                    FindSolutionUserByCertDN();
                    break;
                case SolutionUsersNodeAction.ActionFindSolutionUserByName:
                    SearchSolutionUsers();
                    break;
            }
        }

        private void SearchSolutionUsers()
        {
            ActionHelper.Execute(delegate
            {
                var form = new SearchByNameView("User name");
                var context = this.GetApplicationContext();
                if (SnapIn.Console.ShowDialog(form) == DialogResult.OK)
                {
                    SolutionUsersControl.RefreshUsers(form.SearchString);
                }
            }, null);
        }

        void FindSolutionUserByCertDN()
        {
            ActionHelper.Execute(delegate()
            {
                var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                var frm = new FindSolutionUserByCertDN(service, _serverDto);
                if(this.SnapIn.Console.ShowDialog(frm) == DialogResult.OK)
                {
                    SolutionUsersControl.RefreshUsers(frm.CertDn, SearchType.CERT_SUBJECTDN);
                }
            }, null);
        }

        void AddNewUser()
        {
            ActionHelper.Execute(delegate
            {
                var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                var server = _serverDto;
                var userForm = new NewSolutionUserForm(service, server, _tenantName);
                var context = this.GetApplicationContext();
                var dataContext = context.NavigationController.NavigateToView(this, userForm);
                if (dataContext != null)
                {
                    var userDto = (SolutionUserDto)dataContext;
                    if (userDto != null)
                    {
                        MMCDlgHelper.ShowMessage(string.Format("Solution User {0} created successfully", userDto.Name));
                        SolutionUsersControl.RefreshUsers(string.Empty);
                    }
                }
            } , null);
        }

        void AddViewDescription()
        {
            var fvd = new FormViewDescription
            {
                DisplayName = "Solution Users",
                ViewType = typeof(SolutionUsersFormView),
                ControlType = typeof(SolutionUsersControl)
            };

            // Attach the view to the root node.
            ViewDescriptions.Add(fvd);
            ViewDescriptions.DefaultIndex = 0;
        }

        protected override void OnAddPropertyPages(PropertyPageCollection propertyPageCollection)
        {
            base.OnAddPropertyPages(propertyPageCollection);
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto, _tenantName);

            if (auth != null)
            {
                ActionHelper.Execute(delegate
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(ServerDto.ServerName);
                    var dto = Tag as SolutionUserDto;
                    var propManager = new SolutionUserPropertyManager(service, dto, ServerDto, _tenantName);

                    var generalPage = new SolutionUserGeneralProperty(propManager) { Title = "General" };
                    propertyPageCollection.Add(generalPage.Page);

                    var certPage = new SolutionUserCertProperty(propManager) { Title = "Certificate" };
                    propertyPageCollection.Add(certPage.Page);
                }, auth);
            };
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);
            DoRefresh();
        }

        void DoRefresh()
        {
            if (SolutionUsersControl != null)
            {
                SolutionUsersControl.RefreshUsers(string.Empty);
            }
        }
    }
}

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
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.Groups;
using Vmware.Tools.RestSsoAdminSnapIn.Views;
using Vmware.Tools.RestSsoAdminSnapIn.Views.PropertyPages;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes{    public class GroupsNode : ScopeNode    {
        public enum GroupsNodeAction
        {
            ActionNewGroup = 1,
            ActionFindByGroupName = 2,
        }       
        private readonly string _tenantName;
        private readonly string _domainName;
        private readonly bool _isSystemDomain;
        private readonly ServerDto _serverDto;
        public string TenantName { get { return _tenantName; } }
        public string DomainName { get { return _domainName; } }
        public bool IsSystemDomain { get { return _isSystemDomain; } }
        public ServerDto ServerDto { get { return _serverDto; } }
        public GroupsControl GroupsControl { get; set; }
        public GroupsNode(ServerDto dto, string tenantName, string domainName, bool systemDomain)
            : base(true)
        {
            ImageIndex = SelectedImageIndex = (int)TreeImageIndex.Group;            _tenantName = tenantName;            _isSystemDomain = systemDomain;            _serverDto = dto;
            _domainName = domainName;            
            DisplayName = "Groups";
            EnabledStandardVerbs = StandardVerbs.Refresh;
            if (systemDomain)
            {
                var action = new Action("New Group", "New Group", (int)TreeImageIndex.UserAdd, GroupsNodeAction.ActionNewGroup);
                ActionsPaneItems.Add(action);
            }
            ActionsPaneItems.Add(new Action("Search", "Searches for group(s) by group name", (int)TreeImageIndex.Search, GroupsNodeAction.ActionFindByGroupName));            AddViewDescription();        }
        protected override void OnAction(Action action, AsyncStatus status)        {            base.OnAction(action, status);            switch ((GroupsNodeAction)(int)action.Tag)            {
                case GroupsNodeAction.ActionNewGroup:                    AddNewGroup();                    break;
                case GroupsNodeAction.ActionFindByGroupName:
                    SearchForGroups();
                    break;                        }        }

        private void SearchForGroups()
        {
            ActionHelper.Execute(delegate
            {
                var form = new SearchByNameView("Group name");
                var context = this.GetApplicationContext();
                if (SnapIn.Console.ShowDialog(form) == DialogResult.OK)
                {
                    if (GroupsControl != null)
                        this.GroupsControl.RefreshGroups(form.SearchString);
                }
            }, null);
        }
        void AddNewGroup()        {
            var service = ScopeNodeExtensions.GetServiceGateway(ServerDto.ServerName);
            var authTokenDto = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto, _tenantName);
            ActionHelper.Execute(delegate
            {
                var groupForm = new NewGroupForm();                
                if (this.SnapIn.Console.ShowDialog(groupForm) == DialogResult.OK)
                {   
                    var group = (GroupDto) groupForm.DataContext;
                    group.GroupDomain = _domainName;                    
                    var success = service.Group.Create(ServerDto, TenantName, @group, authTokenDto.Token);
                    if (GroupsControl != null)
                        GroupsControl.RefreshGroups(string.Empty);
                }
            }, authTokenDto);        }
        void AddViewDescription()        {
            var fvd = new FormViewDescription
            {
                DisplayName = "Groups",
                ViewType = typeof (GroupsFormView),
                ControlType = typeof (GroupsControl)                
            };

            // Attach the view to the root node.            ViewDescriptions.Add(fvd);            ViewDescriptions.DefaultIndex = 0;        }

        protected override void OnAddPropertyPages(PropertyPageCollection propertyPageCollection)
        {
            base.OnAddPropertyPages(propertyPageCollection);
            var authTokenDto = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto, TenantName);
            ActionHelper.Execute(delegate
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(ServerDto.ServerName);
                    var groupDto = Tag as GroupDto;
                    var presenter = new GroupPropertyDataManager(groupDto, service, ServerDto, TenantName, _domainName);
                    var generalPage = new GroupGeneralProperty(presenter, groupDto, IsSystemDomain) { Title = "General" };
                    propertyPageCollection.Add(generalPage.Page);
                }, authTokenDto);
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);
            if (GroupsControl != null)
                this.GroupsControl.RefreshGroups(string.Empty);
        }    }}
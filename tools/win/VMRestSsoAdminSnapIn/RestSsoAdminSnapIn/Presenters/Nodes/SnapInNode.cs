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
using System.Collections.Generic;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Views;
using VMwareMMCIDP.UI.Common.Utilities;
using VMIdentity.CommonUtils;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes
{
    public class SnapInNode : ScopeNode
    {
        public enum SnapInNodeAction
        {
            AddServer = 1,
            ActionTokenWizard = 2,
            ShowHttpTransport = 3,
            OfflineSuperLog = 4
        }

        private ServerDto _dto;
        public SnapInNode(): base(false)
        {
            this.DisplayName = MMCMiscUtil.GetBrandConfig(CommonConstants.SSO_ROOT);
            AddActions();
        }

        private void AddActions()
        {
            ActionsPaneItems.Add(new Action("Add Server", "Add Server", (int)TreeImageIndex.Server, SnapInNodeAction.AddServer));
            ActionsPaneItems.Add(new Action("Diagnostics", "Diagnostics", (int)TreeImageIndex.TrustedCertificate, SnapInNodeAction.ActionTokenWizard));
            ActionsPaneItems.Add(new Action("Show HTTP Transport", "Show HTTP Transport", (int)TreeImageIndex.Report, SnapInNodeAction.ShowHttpTransport));
            ActionsPaneItems.Add(new Action("Offline SuperLog File Analyzer", "Offline SuperLog File Analyzer", (int)TreeImageIndex.Settings, SnapInNodeAction.OfflineSuperLog));
        }

        protected override void OnAction(Action action, AsyncStatus status)
        {
            base.OnAction(action, status);
            switch ((SnapInNodeAction)(int)action.Tag)
            {
                case SnapInNodeAction.AddServer:
                    _dto = NavigateToAddServerView();
                    if (_dto != null)
                    {
                        AddServerNodeAndAskForLogin(_dto);
                    }
                    break;

                case SnapInNodeAction.ShowHttpTransport:
                    NavigateToHttpTransportView();
                    break;
                case SnapInNodeAction.ActionTokenWizard:
                    ShowTokenWizard();
                    break;
                case SnapInNodeAction.OfflineSuperLog:
                    ShowSuperLog();
                    break;
            }
        }

        private void ShowSuperLog()
        {
            ActionHelper.Execute(delegate
            {
                var tenantName = DisplayName;
                var title = string.Format("Offline Super Logging");
                var frm = new SuperLogging() { Text = title };
                this.SnapIn.Console.ShowDialog(frm);
            }, null);
        }
        private void NavigateToHttpTransportView()
        {
            var form = new HttpTransportFormView();
            SnapInContext.Instance.NavigationController.NavigateToView(this, form);
        }
        private void ShowTokenWizard()
        {
            var form = new TokenWizard();
            SnapInContext.Instance.NavigationController.NavigateToView(this, form);
        }
        private ServerDto NavigateToAddServerView()
        {
            var service = ScopeNodeExtensions.GetServiceGateway();
            var form = new NewServerView(service) { Text = @"Add New Server" };
            var showAddServerUi = SnapInContext.Instance.NavigationController.NavigateToView(this, form);
            return (ServerDto)showAddServerUi;
        }
        private void AddServerNodeAndAskForLogin(ServerDto serverDto)
        {
            var node = new ServerNode(serverDto);
            var nodeAlreadyExists = AddServerNode(ref node);
            if (!nodeAlreadyExists)
            {
                // Ask for login into the new server.
                var auth = new AuthTokenDto { ServerDto = serverDto, };                
                node.Login(auth);
            }
            else
            {
                MMCDlgHelper.ShowWarning(string.Format("Server {0} already exists", node.DisplayName));
            }
        }
        private bool AddServerNode(ref ServerNode node)
        {
            var nodeWithSameNameExists = false;
            var index = 0;
            while (index < Children.Count)
            {
                var serverNode = (ServerNode)Children[index];
                if (serverNode.DisplayName == node.DisplayName)
                {
                    nodeWithSameNameExists = true;
                    break;
                }
                index++;
            }

            if (!nodeWithSameNameExists)
                Children.Add(node);
            else
                node = (ServerNode)Children[index];
            return nodeWithSameNameExists;
        }
        private void AddTenantNode(ServerNode serverNode, TenantNode node)
        {
            var nodeWithSameNameExists = false;
            var index = 0;
            while (index < serverNode.Children.Count)
            {
                var tenantNode = (TenantNode)serverNode.Children[index++];
                if (tenantNode.DisplayName == node.DisplayName)
                {
                    nodeWithSameNameExists = true;
                    break;
                }
            }

            if (!nodeWithSameNameExists)
                serverNode.Children.Add(node);
        }

        public void RefreshAll()
        {
            Children.Clear();
            var authTokens = SnapInContext.Instance.AuthTokenManager.GetAllAuthTokens();
            foreach (var token in authTokens)
            {
                var node = new ServerNode(token.ServerDto);
                AddServerNode(ref node);
                //Children.Add(node);
            }
        }

        public void RemoveChild(string displayName)
        {
            var childIndex = 0;
            foreach (ScopeNode node in Children)
            {
                if (node.DisplayName == displayName)
                {
                    Children.RemoveAt(childIndex);
                    break;
                }
                childIndex++;
            }
        }

        public ServerNode GetServerNode(ScopeNode node)
        {            
            while(node != null)
            {
                if (node is ServerNode)
                    break;
                node = node.Parent;                
            }
            return (ServerNode)node;
        }

        public ScopeNode CurrentNode { get; set; }
    }
}

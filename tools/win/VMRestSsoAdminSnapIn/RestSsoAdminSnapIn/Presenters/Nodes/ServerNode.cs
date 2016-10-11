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
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Views;
using Action = Microsoft.ManagementConsole.Action;
using Microsoft.ManagementConsole;
using System.Windows.Forms;
using System.Net;
using VMwareMMCIDP.UI.Common.Utilities;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes
{
    public class ServerNode : ScopeNode
    {
        public enum ServerNodeAction
        {
            ActionAddTenant = 1,
            ActionAddExistingTenant = 2,
            ActionLogin = 3,
            ActionLogout = 4,
            ActionViewActiveDirectory = 5,
            ActionGetComputers = 6,
            ActionTokenWizard = 7,
            ActionViewServerInfo = 8
        }

        private readonly ServerDto _serverDto;
        private ServerInfoDto _serverInfo;
        public ServerNode(ServerDto serverDto)
            : base(false)
        {
            AddLoggedOutUserActions();
            _serverDto = serverDto;
            DisplayName = serverDto.ServerName;
            Tag = new AuthTokenDto { ServerDto = _serverDto };
            ImageIndex = SelectedImageIndex = serverDto.TokenType == TokenType.SAML ? (int)TreeImageIndex.SamlServer : (int)TreeImageIndex.Server;
        }
        protected override void OnExpand(AsyncStatus status)
        {
            base.OnExpand(status);
            if (Tag == null || (Tag != null && ((AuthTokenDto)Tag).Token == null))
            {
                status.Title = "Login into server to view details";
            }
            else
            {
                if (Tag != null && ((AuthTokenDto)Tag).Token != null)
                {
                    var auth = (AuthTokenDto)Tag;
                    status.Title = "Logged in as " + auth.Login.User + "@" + auth.Login.TenantName;
                }
            }
        }
        private void ShowTokenWizard()
        {
            var form = new TokenWizard();
            SnapInContext.Instance.NavigationController.NavigateToView(this, form);
        }
        protected override void OnDelete(SyncStatus status)
        {
            var deleted = false;
            ActionHelper.Execute(delegate
            {
                if (ErrorMessageDisplayHelper.Confirm("Remove Server " + _serverDto.ServerName + "?"))
                {
                    (Parent as SnapInNode).RemoveChild(_serverDto.ServerName);
                    SnapInContext.Instance.AuthTokenManager.RemoveAuthTokens(_serverDto.ServerName);
                    deleted = true;                    
                }
            }, null);
            if (deleted)
                base.OnDelete(status);
        }

        private void RefreshParent()
        {
            var ssoAdminRootNode = Parent as SnapInNode;
            if (ssoAdminRootNode != null)
                ssoAdminRootNode.RefreshAll();
        }

        protected override void OnAction(Action action, AsyncStatus status)
        {
            base.OnAction(action, status);
            switch ((ServerNodeAction)(int)action.Tag)
            {
                case ServerNodeAction.ActionLogin:
                    var auth = (AuthTokenDto)Tag;
                    LoginUser(auth);
                    break;
                case ServerNodeAction.ActionAddExistingTenant:
                    LoginToExistingTenant();
                    break;
                case ServerNodeAction.ActionAddTenant:
                    AddTenant();
                    break;
                case ServerNodeAction.ActionLogout:
                    LogoutUser();
                    break;
                case ServerNodeAction.ActionViewActiveDirectory:
                    ShowActiveDirectory();
                    break;
                case ServerNodeAction.ActionGetComputers:
                    ShowComputers();
                    break;
                case ServerNodeAction.ActionViewServerInfo:
                    ShowServerInfo();
                    break;
                case ServerNodeAction.ActionTokenWizard:
                    ShowTokenWizard();
                    break;
            }
        }

        private void ShowServerInfo()
        {
            ActionHelper.Execute(delegate
            {
                var view = new AboutServerView(_serverInfo);
                var context = this.GetApplicationContext();
                var dataContext = context.NavigationController.NavigateToView(this, view);
            }, null);
        }
        private void ShowComputers()
        {
            ActionHelper.Execute(delegate()
            {
                var systemTenant = string.Empty;

                if(Children != null)
                {
                    foreach(TenantNode child in Children)
                    {
                        if(child.IsSystemTenant)
                        {
                            systemTenant = child.DisplayName;
                            break;
                        }
                    }
                }
                var form = new ComputersView(_serverDto, systemTenant);
                SnapIn.Console.ShowDialog(form);
            }, null);
        }
        private void ShowActiveDirectory()
        {
            var auths = SnapInContext.Instance.AuthTokenManager.GetAuthTokens(_serverDto);
            var auth = auths[0];

            ActionHelper.Execute(delegate()
            {
                var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                var adJoinInfoDto = service.Adf.GetActiveDirectory(_serverDto, auth.Token);
                if (adJoinInfoDto == null || (adJoinInfoDto != null && adJoinInfoDto.JoinStatus != "DOMAIN"))
                {
                    var form = new JoinActiveDirectoryView(_serverDto);
                    SnapIn.Console.ShowDialog(form);
                }
                else
                {
                    var form = new LeaveActiveDirectoryView(_serverDto, adJoinInfoDto);
                    SnapIn.Console.ShowDialog(form);
                }
            }, auth);
        }
        private void LogoutUser()
        {
            var auth = (AuthTokenDto)Tag;
            this.Children.Clear();
            auth.Token = null;
            auth.Login = null;            
            AddLoggedOutUserActions();
        }
        void AddTenant()
        {
            var auth = (AuthTokenDto)Tag;
            if (auth.Token == null)
            {
                MMCDlgHelper.ShowMessage("Please login with an account with administrator privileges to Add Tenant");
                return;
            }
            ActionHelper.Execute(delegate
            {
                var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                var frm = new NewSignerIdentity(service, _serverDto);
                if (this.SnapIn.Console.ShowDialog(frm) == DialogResult.OK)
                {
                    var tenantDto = (TenantDto)frm.DataContext;
                    if (tenantDto != null)
                    {
                        MMCDlgHelper.ShowMessage("Tenant " + tenantDto.Name + " created successfully.");
                    }
                }
            }, null);
        }
        private void AddTenantNode(AuthTokenDto tokenDto, string tenantName)
        {
            var tenantDto = new TenantDto { Name = tenantName };
            var node = new TenantNode(_serverDto , tenantName) { DisplayName = tenantName, Tag = tenantName };
            AddTenantNode(node);
        }
        private void AddTenantNode(AuthTokenDto tokenDto, TenantDto tenant)
        {
            var node = new TenantNode(_serverDto, tenant.Name) { DisplayName = tenant.Name, Tag = tenant.Guid };
            AddTenantNode(node);
        }

        private void AddTenantNode(TenantNode node)
        {
            var nodeWithSameNameExists = false;
            var index = 0;
            while (index < Children.Count)
            {
                var tenantNode = (TenantNode)Children[index++];
                if (tenantNode.DisplayName == node.DisplayName)
                {
                    nodeWithSameNameExists = true;
                    break;
                }
            }

            if (nodeWithSameNameExists)
                Children.RemoveAt(index - 1);
            Children.Add(node);
        }
        private void LoginUser(AuthTokenDto auth, bool hasSessionExpired = false)
        {
            LoginUser(auth, null, hasSessionExpired);
        }
        private void LoginUser(AuthTokenDto auth, string tenantName, bool hasSessionExpired = false)
        {
            if (auth.Token != null && !hasSessionExpired) return;
            ActionHelper.Execute(delegate
            {
                var text = hasSessionExpired ? "Re-login - Credentials expired or changed" : "Login";
                var tenant = tenantName == null ? _serverDto.Tenant : tenantName;
                var login = tenantName == null ? auth.Login : null;
                var loginForm = new LoginForm(login, tenant, text);
                var context = this.GetApplicationContext();
                var dataContext = context.NavigationController.NavigateToView(this, loginForm);
                if (dataContext != null)
                {
                    var service = ScopeNodeExtensions.GetServiceGateway();
                    var loginDto = (LoginDto)dataContext;
                    try
                    {
                        var authToken = service.Authentication.Login(auth.ServerDto, loginDto, Constants.ClientId);
                        AddServiceGatewayForServer(service, authToken);
                        Tag = new AuthTokenDto
                        {
                            Login = new LoginDto { User = authToken.Login.User, TenantName = authToken.Login.TenantName, Pass = authToken.Login.Pass, DomainName = authToken.Login.DomainName },
                            ServerDto = authToken.ServerDto,
                            Token = authToken.Token
                        };
                        SnapInContext.Instance.AuthTokenManager.SetAuthToken(authToken);
                        AddTenantNode(authToken, loginDto.TenantName);
                    }
                    catch (WebException exp)
                    {
                        if (exp.Response is HttpWebResponse)
                        {
                            var response = exp.Response as HttpWebResponse;
                            if (response != null && response.StatusCode == HttpStatusCode.NotFound)
                            {
                                MMCDlgHelper.ShowMessage(MMCUIConstants.INCOMPATIBLE_SERVER);
                                return;
                            }
                            else
                            {
                                MMCDlgHelper.ShowMessage(exp.Message);
                                return;
                            }
                        }
                        else
                        {
                            MMCDlgHelper.ShowMessage(exp.Message);
                            return;
                        }
                    }
                    AddLoggedInUserActions();
                }
            }, auth);
        }

        private void AddServiceGatewayForServer(ServiceGateway service, AuthTokenDto authToken)
        {
            try
            {
                _serverInfo = service.Server.GetServerInfo(authToken.ServerDto, authToken.Token);
            }
            catch (Exception exc)
            {
                // default the configuration to vsphere
                _serverInfo = new ServerInfoDto
                {
                    Release = "Vsphere",
                    ProductName = "idm"
                };
            }

            if (authToken.ServerDto.ServerName == "10.161.26.243")
                _serverInfo.Release = "Lightwave";
            var serviceConfigManager = new ServiceConfigManager(_serverInfo.Release);
            var serviceGateway = new ServiceGateway(serviceConfigManager);
            SnapInContext.Instance.ServiceGatewayManager.Add(authToken.ServerDto.ServerName, serviceGateway);
        }
        private void LoginToExistingTenant()
        {
            ActionHelper.Execute(delegate
            {
                var tenantView = new AddExistingTenant();
                var context = this.GetApplicationContext();
                var dataContext = context.NavigationController.NavigateToView(this, tenantView);
                if (dataContext != null)
                {   
                    var tenantDto = (TenantDto)dataContext;
                    AddExistingTenant(tenantDto);
                }
            }, null);
        }

        private void AddExistingTenant(TenantDto tenantDto)
        {
            var service = ScopeNodeExtensions.GetServiceGateway();
            var auths = SnapInContext.Instance.AuthTokenManager.GetAuthTokens(_serverDto);
            var auth = auths[0];
            ActionHelper.Execute(delegate
           {
               var loginDto = new LoginDto { User = auth.Login.User, Pass = auth.Login.Pass, TenantName = tenantDto.Name, DomainName = tenantDto.Name };
               var serverDto = new ServerDto { ServerName = auth.ServerDto.ServerName, Tenant = tenantDto.Name, Port = auth.ServerDto.Port, Protocol = auth.ServerDto.Protocol };
               var authToken = service.Authentication.Login(serverDto, loginDto, Constants.ClientId);
               AddServiceGatewayForServer(service, authToken);
               SnapInContext.Instance.AuthTokenManager.SetAuthToken(authToken);
               tenantDto = service.Tenant.Get(authToken.ServerDto, tenantDto.Name, authToken.Token);
               AddTenantNode(authToken, tenantDto.Name);
           }, auth, tenantDto.Name);
        }
        public void Login(AuthTokenDto auth)
        {
            LoginUser(auth);
        }

        public void Login(bool sessionExpired)
        {
            var auth = Tag as AuthTokenDto;
            //auth.Token = null;
            LoginUser(auth, sessionExpired);
        }

        public void Login(bool sessionExpired, string tenantName)
        {
            var auth = Tag as AuthTokenDto;
            LoginUser(auth, tenantName, sessionExpired);
        }

        public void LoginAsUser(string tenantName, UserDto userDto)
        {
            var auth = (AuthTokenDto)Tag;
            var userAuth = new AuthTokenDto { ServerDto = auth.ServerDto, Login = new LoginDto { User = userDto.Name, DomainName = userDto.Domain, TenantName = tenantName } };
            LoginUser(userAuth);
        }
        private void AddLoggedOutUserActions()
        {
            ActionsPaneItems.Clear();
            EnabledStandardVerbs = StandardVerbs.Delete;
            var loginAction = new Action("Login", "Login", (int)TreeImageIndex.Login, ServerNodeAction.ActionLogin);
            ActionsPaneItems.Add(loginAction);
            ActionsPaneItems.Add(new Action("Diagnostics", "Diagnostics", (int)TreeImageIndex.TrustedCertificate, ServerNodeAction.ActionTokenWizard));
        }
        private void AddLoggedInUserActions()
        {
            ActionsPaneItems.Clear();
            EnabledStandardVerbs = StandardVerbs.Delete;
            var logoutAction = new Action("Logout", "Logout", (int)TreeImageIndex.Logout, ServerNodeAction.ActionLogout);
            ActionsPaneItems.Add(logoutAction);
            var addTenant = new Action("Add New Tenant", "Add New Tenant", (int)TreeImageIndex.AddTenant, ServerNodeAction.ActionAddTenant);
            ActionsPaneItems.Add(addTenant);
            var addExistingTenant = new Action("Add Existing Tenant", "Add Existing Tenant", (int)TreeImageIndex.Tenant, ServerNodeAction.ActionAddExistingTenant);
            ActionsPaneItems.Add(addExistingTenant);
            var serverInfo = new Action("About Server", "Shows the server release and product information", (int)TreeImageIndex.Settings, ServerNodeAction.ActionViewServerInfo);
            ActionsPaneItems.Add(serverInfo);

            var isSystemTenantPresent = CheckSystemTenantIsPresent();

            if (isSystemTenantPresent)
            {
                var computer = new Action("Get Computers", "Shows the computers associated with the server", (int)TreeImageIndex.Computers, ServerNodeAction.ActionGetComputers);
                ActionsPaneItems.Add(computer);
            }
            ActionsPaneItems.Add(new Action("Diagnostics", "Diagnostics", (int)TreeImageIndex.TrustedCertificate, ServerNodeAction.ActionTokenWizard));
        }

        private bool CheckSystemTenantIsPresent()
        {
            bool present = false;
            if(Children != null)
            {
                foreach(TenantNode child in Children)
                {
                    if (child.IsSystemTenant)
                        present = true;
                }
            }
            return present;
        }
    }
}

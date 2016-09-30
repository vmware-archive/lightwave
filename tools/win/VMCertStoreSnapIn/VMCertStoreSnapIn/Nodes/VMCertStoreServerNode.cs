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
using System.Net;
using System.Net.Sockets;
using Microsoft.ManagementConsole;
using VMCertStore.Common.DTO;
using VMCertStoreSnapIn.Utilities;
using VMwareMMCIDP.UI.Common;
using System.Windows.Forms;
using System.Threading.Tasks;
using VMIdentity.CommonUtils;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCertStoreSnapIn.Nodes
{
    class VMCertStoreServerNode : ScopeNode
    {
        const int ACTION_GET_STORE_VERSION = 1;
        const int ACTION_CHECK_CONNECTIVITY = 2;
        const int ACTION_LOGIN = 3;
        const int ACTION_LOGOUT = 4;

        public VMCertStoreServerDTO ServerDTO { get; protected set; }

        public VMCertStoreServerNode(VMCertStoreServerDTO dto)
            : base()
        {
            ServerDTO = dto;
            DisplayName = ServerDTO.Server;
            this.ImageIndex = this.SelectedImageIndex = (int)VMCertStoreImageIndex.Server;

            this.EnabledStandardVerbs = StandardVerbs.Delete | StandardVerbs.Refresh;

            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Login",
                "Login", -1, ACTION_LOGIN));
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Check connectivity",
                "Check connectivity", -1, ACTION_CHECK_CONNECTIVITY));

            PopulateChildren();
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);

            PopulateChildren();
        }

        public void PopulateChildren()
        {
            this.Children.Clear();
            if (ServerDTO.CanLogin())
            {
                this.Children.Add(new VecsStoresNode(ServerDTO));
            }
        }

        protected override void OnDelete(SyncStatus status)
        {
            if (!MMCDlgHelper.ShowConfirm(CommonConstants.GetDeleteMsg("server", ServerDTO.Server)))
                return;

            base.OnDelete(status);
            VMCertStoreSnapInEnvironment.Instance.LocalData.RemoveServer(ServerDTO.GUID);
            (this.Parent as VMCertStoreRootNode).RefreshAll();
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch((int)action.Tag)
            {
                case ACTION_LOGIN:
                    DoLogin();
                    break;
                case ACTION_GET_STORE_VERSION:
                    ShowStoreVersion();
                    break;
                case ACTION_CHECK_CONNECTIVITY:
                    CheckConnectivity();
                    break;
                case ACTION_LOGOUT:
                    DoLogout();
                    break;
            }
        }

        public async Task DoLogin()
        {
            try
            {
                if (!ServerDTO.IsLoggedIn)
                {
                    if (!string.IsNullOrEmpty(ServerDTO.UserName))
                    {
                        String[] user = ServerDTO.UserName.Split('@');
                        ServerDTO.UserName = user[0];
                    }
                    var frmLogin = new LoginForm(ServerDTO.Server, ServerDTO.UserName, ServerDTO.DomainName);
                    if (this.SnapIn.Console.ShowDialog(frmLogin) == DialogResult.OK)
                    {
                        
                        ServerDTO.Server = frmLogin.Server;
                        ServerDTO.UserName = frmLogin.UserName;
                        ServerDTO.Password = frmLogin.Password;
                        ServerDTO.DomainName = frmLogin.DomainName;
                        await ServerDTO.LogintoServer(ServerDTO.UserName, ServerDTO.Password, ServerDTO.DomainName);
                        this.DisplayName = ServerDTO.Server;
                        if (ServerDTO.IsLoggedIn)
                        {
                            PopulateChildren();
                            this.ActionsPaneItems.Clear();
                            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Logout","Logout", -1, ACTION_LOGOUT));
                            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Check connectivity",
                                "Check connectivity", -1, ACTION_CHECK_CONNECTIVITY));
                            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Get Store version",
                                                       "Get Store version", -1, ACTION_GET_STORE_VERSION));
                        }
                        else
                        {
                            MMCDlgHelper.ShowError(MMCUIConstants.UNABLE_TO_LOGIN);
                        }
                    }
                }

            }
            catch (Exception e)
           {
               MMCDlgHelper.ShowException(e);
           }
        }

        public async Task DoLogout()
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                if (ServerDTO.IsLoggedIn)
                {
                    ServerDTO.Cleanup();
                    this.ActionsPaneItems.Clear();
                    this.Children.Clear();
                    this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Login",
               "Login", -1, ACTION_LOGIN));
                    this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Check connectivity",
               "Check connectivity", -1, ACTION_CHECK_CONNECTIVITY));
                }
            });
        }
        public void CheckConnectivity()
        {
            bool success = false;
            MMCActionHelper.CheckedExec(delegate()
            {
                var addresses = Dns.GetHostAddresses(ServerDTO.Server);

                using (var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
                {
                    IAsyncResult result = socket.BeginConnect(addresses[0], 2020, null, null);
                    success = result.AsyncWaitHandle.WaitOne(3 * 1000, true);
                    if (success)
                        success = socket.Connected;
                }
            });
            if (success)
                MMCDlgHelper.ShowMessage("Connection successful");
            else
                MMCDlgHelper.ShowMessage("Could not connect to server");
        }

        void ShowStoreVersion()
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                string version = "1.0.0";
                MMCDlgHelper.ShowMessage(version);
            });
        }
    }
}

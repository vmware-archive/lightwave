/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
using System;
//using System.DirectoryServices.Protocols;
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using Microsoft.ManagementConsole.Advanced;
using System.Collections.Generic;
using VMDir.Common.DTO;
using VMDirSnapIn.UI;
using VMDirSnapIn.Services;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;
using VMDir.Common.VMDirUtilities;
using VMIdentity.CommonUtils;
using VMwareMMCIDP.UI.Common.Utilities;


namespace VMDirSnapIn.ScopeNodes
{
    public class ServerNode : ScopeNode
    {
        const int ACTION_SHOW_SCHEMA = 1;
        const int ACTION_LOGIN = 2;
        const int ACTION_SEARCH = 3;
        const int ACTION_SUPERLOG = 4;
        const int ACTION_LOGOUT = 5;

        VMDirServerDTO _serverDTO;
        public VMDirServerDTO ServerDTO { get { return _serverDTO; } }
        public string ServerGUID { get; set; }
        public bool isLoggedIn { get; set; }

        public ServerNode(VMDirServerDTO dto)
        {
            _serverDTO = dto;
            DisplayName = dto.Server;

            ImageIndex = SelectedImageIndex = (int)VMDirImageIndex.Server;
            this.EnabledStandardVerbs = StandardVerbs.Delete;

            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Login",
                                       "Login", -1, ACTION_LOGIN));
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_LOGIN:
                    Login();
                    break;
                case ACTION_SHOW_SCHEMA:
                    try
                    {
                        SnapIn.Console.ShowDialog(new SchemaBrowser(_serverDTO));
                    }
                    catch (Exception exp)
                    {
                        var mbParams = new MessageBoxParameters { Caption = "Error accessing schema", Text = exp.Message };
                        SnapIn.Console.ShowDialog(mbParams);
                    }
                    break;
                case ACTION_SUPERLOG:
                    try
                    {
                        SnapIn.Console.ShowDialog(new SuperLogBrowser(_serverDTO));
                    }
                    catch (Exception exp)
                    {
                        var mbParams = new MessageBoxParameters { Caption = "Error accessing superlog info", Text = exp.Message };
                        SnapIn.Console.ShowDialog(mbParams);
                    }
                    break;
                case ACTION_LOGOUT:
                    Logout();
                    break;
            }
        }
        private void LoggedInServerActions()
        {
            this.ActionsPaneItems.Clear();
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Logout",
                                    "Logout", -1, ACTION_LOGOUT));
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Show Schema",
                                                 "Show Schema", -1, ACTION_SHOW_SCHEMA));
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Superlog",
                                       "Superlog", -1, ACTION_SUPERLOG));
        }

        private void LoggedOutServerActions()
        {
            this.ActionsPaneItems.Clear();
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Login",
                                        "Login", -1, ACTION_LOGIN));
           
        }
        private void Logout()
        {
            MiscUtilsService.CheckedExec(delegate()
            {
                _serverDTO.Connection = new LdapConnectionService(_serverDTO.Server, _serverDTO.BindDN, _serverDTO.Password);
                _serverDTO.Connection.CloseConnection();
                _serverDTO.Connection = null;
                Children.Clear();
                isLoggedIn = false;
                LoggedOutServerActions();
            });
        }

        protected override void OnDelete(SyncStatus status)
        {
            base.OnDelete(status);
            RemoveServer();
        }

        public void Login()
        {
            this.Children.Clear();
            try
            {
                var frm = new frmConnectToServer(_serverDTO);
                if (MMCDlgHelper.ShowForm(frm))
                {
                    if (_serverDTO.Connection.CreateConnection() == 1)
                    {
                        isLoggedIn = true;
                        this.DisplayName = ServerDTO.Server;
                        this.Children.Add(new DirectoryNode(_serverDTO.BaseDN, ServerDTO));
                        LoggedInServerActions();
                    }
                    else
                    {
                        throw new Exception("Invalid credentials or server is not reachable!");
                    }
                }
            }
            catch (Exception exp)
            {
                _serverDTO.Connection = null;
                MMCDlgHelper.ShowException(exp);
            }
        }

        void RemoveServer()
        {
            if (MiscUtilsService.Confirm(CommonConstants.GetDeleteMsg("server",ServerDTO.Server)))
            {
                VMDirEnvironment.Instance.LocalData.RemoveServer(ServerDTO.GUID);
                if (ServerDTO.Connection != null)
                    ServerDTO.Connection.CloseConnection();

                var parent = this.Parent as VMDirRootNode;
                if (parent != null)
                    this.SnapIn.BeginInvoke(parent.RefreshDelegate, new object[] {ServerDTO, this });
            }
        }
    }
}

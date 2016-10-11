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
using VMDir.Common.DTO;
using System.Threading.Tasks;
using VMDir.Common.VMDirUtilities;
using Microsoft.ManagementConsole;
using VMDirSchemaSnapIn;
using VMwareMMCIDP.UI;
using System.Windows.Forms;
using VMwareMMCIDP.UI.Common.Utilities;
using VMwareMMCIDP.UI.Common;
using VMDirSchemaSnapIn.UI;
using VMIdentity.CommonUtils;
using VMDirSchemaSnapIn.Nodes;

namespace VMDirSchemaEditorSnapIn.Nodes
{
    public class VMDirSchemaServerNode:ScopeNode
    {
        const int ACTION_LOGIN = 1;
        const int ACTION_REFRESH = 2;
        const int ACTION_SCHEMACOMPARE = 3;
        const int ACTION_LOGOUT = 4;
        public bool IsLoggedIn { get; set; }
        Microsoft.ManagementConsole.Action LoginAction;

        private int ret = 0;

        public VMDirServerDTO ServerDTO{ get; set; }

        public VMDirSchemaServerNode(VMDirServerDTO serverDTO)
        {
            this.ServerDTO = serverDTO;
            IsLoggedIn = false;
            DisplayName = ServerDTO.Server;
            this.ImageIndex = this.SelectedImageIndex = (int)VMDirSchemaTreeImageIndex.Server;

            AddLoginActions();
        }

        protected async override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);
            switch ((int)action.Tag)
            {
                case ACTION_LOGIN:
                    await DoLogin();
                    break;
                case ACTION_SCHEMACOMPARE:
                     var frm = new SchemaMetadataComparisionWindow(this);
                     SnapIn.Console.ShowDialog(frm);
                    break;
                case ACTION_LOGOUT:
                    Logout();
                    break;

            }
        }

        public void AddLoginActions()
        {
            ActionsPaneItems.Clear();
            EnabledStandardVerbs = StandardVerbs.Delete;
            LoginAction = new Microsoft.ManagementConsole.Action(MMCUIConstants.LOGIN, MMCUIConstants.LOGIN, -1, ACTION_LOGIN);
            ActionsPaneItems.Add(LoginAction);
        }

        public void AddLogoutActions()
        {
            EnabledStandardVerbs = StandardVerbs.Delete | StandardVerbs.Refresh;
            var logoutAction = new Microsoft.ManagementConsole.Action(MMCUIConstants.LOGOUT, MMCUIConstants.LOGOUT, -1, ACTION_LOGOUT);
            ActionsPaneItems.Add(logoutAction);
        }

        protected override void OnDelete(SyncStatus status)
        {
            base.OnDelete(status);
            RemoveServer();
        }

        void RemoveServer()
        {
            var result = MMCDlgHelper.ShowConfirm(MMCUIConstants.CONFIRM);
            if (result)
            {
                CloseConnection();
                if (VMDirSchemaSnapInEnvironment.Instance.LocalData.SerializableList.Contains(ServerDTO.Server))
                {
                    VMDirSchemaSnapInEnvironment.Instance.LocalData.SerializableList.Remove(ServerDTO.Server);
                }

                var parent = this.Parent as VMSchemaRootNode;

                if (parent != null)
                    parent.Children.Remove(this);

            }
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);
            ServerDTO.Connection.SchemaManager.RefreshSchema();
            PopulateChildren();
        }

        public void Logout()
        {
            CloseConnection();
            AddLoginActions();
        }

        private void CloseConnection()
        {
            if (this.Children != null)
            {
                this.Children.Clear();
            }

            ret = 0;
            IsLoggedIn = false;
            if (ServerDTO != null)
            {
                ServerDTO.Password = null;
                ServerDTO.BindDN = null;
            }
        }

        void PopulateChildren()
        {
            if (IsLoggedIn)
            {
                this.Children.Clear();
                var classNode = new VMDirSchemaClassBaseNode(this);
                this.Children.Add(classNode);
                var attrNode = new VMDirSchemaAttributeBaseNode(this);
                this.Children.Add(attrNode);
            }
            else
            {

                MMCDlgHelper.ShowException(new Exception(MMCUIConstants.UNABLE_TO_LOGIN));
            }
        }

        public async Task DoLogin()
        {
            try
            {
                string user=string.Empty;
                string domain=string.Empty;
                if(!string.IsNullOrWhiteSpace(ServerDTO.BindDN))
                {
                    var userAndDomain = ServerDTO.BindDN.Split('@');
                    user = userAndDomain[0];
                    if(userAndDomain[1] != null)
                        domain = userAndDomain[1];
                }
                var frmLogin = new LoginForm(ServerDTO.Server,user,domain);
                if (SnapIn.Console.ShowDialog(frmLogin) == DialogResult.OK)
                {
                    string Upn = frmLogin.UserName;

                    ServerDTO.Server = frmLogin.Server;
                    ServerDTO.BindDN = frmLogin.UserName+"@"+frmLogin.DomainName;
                    ServerDTO.Password = frmLogin.Password;
                    if (string.IsNullOrWhiteSpace(ServerDTO.BindDN) || string.IsNullOrWhiteSpace(ServerDTO.Password))
                        throw new Exception(MMCUIConstants.VALUES_EMPTY);
                    ServerDTO.Password = frmLogin.Password;
                    Task t = new Task(ServerConnect);
                    t.Start();
                    if (await Task.WhenAny(t, Task.Delay(VMDirSchemaConstants.VMDIRSERVER_TIMEOUT)) == t)
                    {
                        if (ret == 1)
                        {

                            IsLoggedIn = true;
                        }

                        else

                            MMCDlgHelper.ShowException(new Exception(CommonConstants.INVALID_CREDENTIAL));
                    }
                    else
                    {

                        MMCDlgHelper.ShowException(new Exception(VMDirSchemaConstants.TIME_OUT));
                    }
                }
                if (IsLoggedIn)
                {
                    this.DisplayName = ServerDTO.Server;
                    this.ActionsPaneItems.Remove(LoginAction);

                    // this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(VMDirSchemaConstants.COMPARE_SCHEMA, VMDirSchemaConstants.COMPARE_SCHEMA, -1, ACTION_SCHEMACOMPARE));

                    AddLogoutActions();

                    PopulateChildren();

                }
             }
            catch (Exception e)
            {
                MMCDlgHelper.ShowException(e);
            }

            
        }

        public async void ServerConnect()
        {
            try
            {
                ServerDTO.Connection = new LdapConnectionService(ServerDTO.Server, ServerDTO.BindDN, ServerDTO.Password);
                ret = ServerDTO.Connection.CreateConnection();
            }
            catch (Exception e)
            {
                ret = 0;
            }
        }
    }
}
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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.ManagementConsole;
using System.IO;
using VMDirSnapIn.UI;
using System.Windows.Forms;
using VMDir.Common.DTO;
using VMDirSnapIn.Services;

namespace VMDirSnapIn.ScopeNodes
{
    public class VMDirRootNode : ScopeNode
    {
        public delegate void Refresh(VMDirServerDTO dto, ServerNode node);
        public Refresh RefreshDelegate;

        const int ACTION_CONNECT_TO_COMPUTER = 1;

        public string _name;

        public VMDirRootNode()
        {
            DisplayName = "Lightwave Directory Servers";

            ImageIndex = SelectedImageIndex = (int)VMDirImageIndex.Directory;

            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Connect to ldap",
                                       "Connect to ldap", -1, ACTION_CONNECT_TO_COMPUTER));

            RefreshDelegate = new Refresh(RefreshMethod);
        }

        protected override void OnExpand(AsyncStatus status)
        {
            base.OnExpand(status);

            AddAllServers();
        }

        void AddAllServers()
        {
            foreach (var dto in VMDirEnvironment.Instance.LocalData.ServerList)
                this.Children.Add(new ServerNode(dto));
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_CONNECT_TO_COMPUTER:
                    ShowSelectComputerUI();
                    break;
            }
        }

        public bool ShowSelectComputerUI()
        {
            try
            {
                var serverDTO = VMDirServerDTO.CreateInstance();
                serverDTO.Server = "";
                var node = new ServerNode(serverDTO);
                node.Login();
                if (node.isLoggedIn)
                {
                    VMDirEnvironment.Instance.LocalData.AddServer(serverDTO);
                    this.Children.Add(node);
                }
            }
            catch (Exception exp)
            {
                MiscUtilsService.ShowError(exp);
            }
            return false;
        }

        public void RefreshMethod(VMDirServerDTO dto, ServerNode node)
        {
            this.Children.Remove(node);
        }

        void AddServerNode(VMDirServerDTO dto)
        {
            var node = new ServerNode(dto);
            this.Children.Add(node);
        }
    }
}

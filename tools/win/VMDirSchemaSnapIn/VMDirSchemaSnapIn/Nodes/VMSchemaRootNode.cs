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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using VMDir.Common.DTO;
using VMDirSchemaEditorSnapIn.Nodes;
using VMwareMMCIDP.UI.Common;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSchemaSnapIn.Nodes
{
    public class VMSchemaRootNode : ScopeNode
    {
        public delegate void Refresh(VMSchemaRootNode dto);
        public  VMDirSchemaServerNode ServerNode { get; set; }

        protected const int ACTION_ADD_SERVER = 1;
        protected const int ACTION_PROPERTIES = 2;
        protected const int ACTION_RECORDADD = 4;
        protected const int ACTION_ADD_FORWARDZONE = 5;
        protected const int ACTION_ADD_REVERSEZONE = 6;

        public VMSchemaRootNode()
            : base()
        {
            DisplayName = "VMware Schema Editor";

            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Add Server",
                                       "Add Server", -1, ACTION_ADD_SERVER));
        }

        public VMSchemaRootNode(VMDirSchemaServerNode node)
            : base()
        {
            this.ServerNode = node;
        }

        protected override void OnExpand(AsyncStatus status)
        {
            base.OnExpand(status);

            AddAllServers();
        }

        void AddAllServers()
        {
            foreach (var server in VMDirSchemaSnapInEnvironment.Instance.LocalData.GetServerArray())
            {
                VMDirServerDTO dto = VMDirServerDTO.CreateInstance();
                dto.Server = server;
                AddServerNode(dto);
            }
        }

        public void RefreshMethod(VMDirServerDTO dto)
        {
            if (dto == null)
                RefreshAll();
            else
                AddServerNode(dto);
        }

        void AddServerNode(VMDirServerDTO dto)
        {
            var node = new VMDirSchemaServerNode(dto);
            this.ServerNode = node;
            this.Children.Add(node);
        }

        public void RefreshAll()
        {
            this.Children.Clear();

            AddAllServers();
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_ADD_SERVER:
                    AddServer();
                    break;
            }
        }

        async void AddServer()
        {
            try
            {
                var ServerDTO = VMDirServerDTO.CreateInstance();
                ServerDTO.Server = "";
                var node = new VMDirSchemaServerNode(ServerDTO);
                await node.DoLogin();
                if (node.IsLoggedIn)
                {
                    VMDirSchemaSnapInEnvironment.Instance.LocalData.AddServer(ServerDTO.Server);
                    this.Children.Add(node);
                }
            }
            catch (Exception e)
            {
                MMCDlgHelper.ShowException(e);
            }
            finally
            {
                VMDirSchemaSnapInEnvironment.Instance.SaveLocalData();
            }
            /*
            var frm = new SelectComputerUI();
            // frm.Text = "Add Afd server";
            if (SnapIn.Console.ShowDialog(frm) == DialogResult.OK)
            {
                var serverDTO = VMDirServerDTO.CreateInstance();
                serverDTO.Server = frm.ServerName;
                VMDirSchemaSnapInEnvironment.Instance.LocalData.AddServer(serverDTO.Server);
                RefreshMethod(serverDTO);
            }*/
        }
    }
}

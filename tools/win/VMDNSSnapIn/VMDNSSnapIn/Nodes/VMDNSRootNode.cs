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
using VMDNS.Common;
using VMwareMMCIDP.UI.Common;
using VMwareMMCIDP.UI.Common.Utilities;
using VMIdentity.CommonUtils;

namespace VMDNSSnapIn.Nodes
{
    public class VMDNSRootNode : ScopeNode
    {
        public delegate void Refresh(VMDNSRootNode dto);
        public VMDNSServerNode ServerNode { get; set; }
        
        protected const int ACTION_ADD_SERVER = 1;
        protected const int ACTION_PROPERTIES = 2;
        protected const int ACTION_RECORDADD = 4;
        protected const int ACTION_ADD_FORWARDZONE = 5;
        protected const int ACTION_ADD_REVERSEZONE = 6;

        public VMDNSRootNode()
            : base()
        {
            DisplayName = MMCMiscUtil.GetBrandConfig(CommonConstants.DNS_ROOT);

            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Add Server",
                                       "Add Server", -1, ACTION_ADD_SERVER));
        }

        public VMDNSRootNode(VMDNSServerNode node)
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
            //TODO - convert to linq
            foreach (var server in VMDNSSnapInEnvironment.Instance.LocalData.GetServerArray())
            {
                VMDNSServerDTO dto = VMDNSServerDTO.CreateInstance();
                dto.Server = server;
                AddServerNode(dto);
            }
        }

        public void RefreshMethod(VMDNSServerDTO dto)
        {
            if (dto == null)
                RefreshAll();
            else
                AddServerNode(dto);
        }

        void AddServerNode(VMDNSServerDTO dto)
        {
            var node = new VMDNSServerNode(dto);
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

        void AddServer()
        {
            var frm = new SelectComputerUI();
           frm.Text = "Add DNS server";
            if (SnapIn.Console.ShowDialog(frm)== DialogResult.OK)
            {
                var serverDTO = VMDNSServerDTO.CreateInstance();
                serverDTO.Server = frm.ServerName;
                VMDNSSnapInEnvironment.Instance.LocalData.AddServer(serverDTO.Server);
                VMDNSSnapInEnvironment.Instance.SaveLocalData();
                RefreshMethod(serverDTO);
            }
        }
    }
}

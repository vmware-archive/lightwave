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
using VMCertStoreSnapIn.UI;
using System;
using VMCertStoreSnapIn.Utilities;
using System.Runtime.InteropServices;
using System.IO;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using VMCertStore.Common.DTO;
using VMCertStoreSnapIn.ListViews;
using VMwareMMCIDP.UI.Common;
using VMwareMMCIDP.UI.Common.Utilities;
using VMIdentity.CommonUtils;


namespace VMCertStoreSnapIn.Nodes
{
    class VMCertStoreRootNode : ScopeNode
    {
        public delegate void Refresh(VMCertStoreServerDTO dto);
        public Refresh RefreshDelegate;

        const int ACTION_ADD_SERVER = 1;

        public VMCertStoreRootNode()
            : base()
        {
            DisplayName = MMCMiscUtil.GetBrandConfig(CommonConstants.CS_ROOT);

            RefreshDelegate = new Refresh(RefreshMethod);

            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Add Server",
                                       "Add Server", -1, ACTION_ADD_SERVER));
        }

        protected override void OnExpand(AsyncStatus status)
        {
            base.OnExpand(status);

            AddAllServers();
        }

        void AddAllServers()
        {
            foreach (var dto in VMCertStoreSnapInEnvironment.Instance.LocalData.ServerList)
                AddServerNode(dto);
        }

        public void RefreshMethod(VMCertStoreServerDTO dto)
        {
            if (dto == null)
                RefreshAll();
            else
                AddServerNode(dto);
        }

        void AddServerNode(VMCertStoreServerDTO dto)
        {
            var node = new VMCertStoreServerNode(dto);
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

            switch((int)action.Tag)
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
                var ServerDTO = VMCertStoreServerDTO.CreateInstance();
                ServerDTO.Server = "";
                var node = new VMCertStoreServerNode(ServerDTO);
                await node.DoLogin();
                if (node.ServerDTO.IsLoggedIn)
                {
                    VMCertStoreSnapInEnvironment.Instance.LocalData.AddServer(node.ServerDTO);
                    this.Children.Add(node);
                }
            }
            catch (Exception e)
            {
                MMCDlgHelper.ShowException(e);
            }
            finally
            {
                VMCertStoreSnapInEnvironment.Instance.SaveLocalData();
            }
        }
    }
}

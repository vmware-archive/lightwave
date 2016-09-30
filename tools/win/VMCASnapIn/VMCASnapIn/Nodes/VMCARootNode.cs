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
using VMCASnapIn.DTO;
using VMCASnapIn.UI;
using VMwareMMCIDP.UI.Common;
using VMCASnapIn.Utilities;
using System;
using VMwareMMCIDP.UI.Common.Utilities;
using VMIdentity.CommonUtils;

namespace VMCASnapIn.Nodes
{
    class VMCARootNode : ScopeNode
    {
        public delegate void Refresh(VMCAServerDTO dto);
        public Refresh RefreshDelegate;

        const int ACTION_ADD_SERVER = 1;

        public VMCARootNode()
            : base()
        {
            DisplayName = MMCMiscUtil.GetBrandConfig(CommonConstants.CA_ROOT);
            RefreshDelegate = new Refresh(RefreshMethod);

            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Add Server",
                                       "Add Server", -1, ACTION_ADD_SERVER));
            ImageIndex = SelectedImageIndex = (int)VMCAImageIndex.CertificateAuthority;
        }

        protected override void OnExpand(AsyncStatus status)
        {
            base.OnExpand(status);

            AddAllServers();
        }

        void AddAllServers()
        {
            foreach (var dto in VMCASnapInEnvironment.Instance.LocalData.ServerQueue)
                this.Children.Add(new VMCAServerNode(dto));
        }

        public void RefreshMethod(VMCAServerDTO dto)
        {
            if (dto == null)
                RefreshAll();
            else
                AddServerNode(dto);
        }

        void AddServerNode(VMCAServerDTO dto)
        {
            var node = new VMCAServerNode(dto);
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
                var ServerDTO = VMCAServerDTO.CreateInstance();
                ServerDTO.Server = "";
                var node = new VMCAServerNode(ServerDTO);
                await node.DoLogin();
                if (node.ServerDTO.IsLoggedIn)
                {
                    VMCASnapInEnvironment.Instance.LocalData.AddServer(node.ServerDTO);
                    this.Children.Add(node);
                }
            }
            catch (Exception e)
            {
                MMCDlgHelper.ShowException(e);
            }
            finally
            {
                VMCASnapInEnvironment.Instance.SaveLocalData();
            }
        }
    }
}

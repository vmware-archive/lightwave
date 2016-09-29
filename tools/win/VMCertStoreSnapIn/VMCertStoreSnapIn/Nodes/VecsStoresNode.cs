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
using VMCertStore.Common.DTO;
using VMCertStoreSnapIn.ListViews;
using VMCertStoreSnapIn.Utilities;
using VMCertStoreSnapIn.UI;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCertStoreSnapIn.Nodes
{
    public class VecsStoresNode : ScopeNode
    {
        public VMCertStoreServerDTO ServerDTO { get; protected set; }
        const int ACTION_CREATE_STORE = 1;

        public VecsStoresNode(VMCertStoreServerDTO dto)
        {
            ServerDTO = dto;
            DisplayName = "Vecs Stores";
            ImageIndex = SelectedImageIndex = (int)VMCertStoreImageIndex.CertificateStore;
            this.EnabledStandardVerbs = StandardVerbs.Refresh;

            RefreshStores();

            this.ActionsPaneItems.Add(
                new Microsoft.ManagementConsole.Action(
                        "Create Store",
                        "Create Store",
                        -1,
                        ACTION_CREATE_STORE));
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);

            RefreshStores();
        }

        void RefreshStores()
        {
            this.Children.Clear();
            MMCActionHelper.CheckedExec(delegate()
            {
                var client = new Vecs.VecsClient(ServerDTO.Server, ServerDTO.UserName, ServerDTO.Password);
                var stores = client.GetStores();

                foreach (var store in stores)
                {
                    var activeNode = new VecsStoreNode(ServerDTO, store);
                    this.Children.Add(activeNode);
                }
            });
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_CREATE_STORE:
                    CreateStore();
                    break;
            }
        }

        void CreateStore()
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                var frm = new FormVecsCreateCertStore();
                if (MMCDlgHelper.ShowForm(frm))
                {
                    var dto = frm.CertStoreDTO;
                    ServerDTO.VecsClient.CreateStore(dto.StoreName, dto.Password);
                    var node = new VecsStoreNode(ServerDTO, dto.StoreName);
                    this.Children.Add(node);
                }
            });
        }

    }
}

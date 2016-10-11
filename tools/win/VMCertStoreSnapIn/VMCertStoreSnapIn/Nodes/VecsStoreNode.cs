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
using VMCertStore.Common.DTO;
using VMCertStoreSnapIn.ListViews;
using VMCertStoreSnapIn.Utilities;
using System;
using VMIdentity.CommonUtils;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCertStoreSnapIn.Nodes
{
    public class VecsStoreNode : ScopeNode
    {
        public VecsTrustedCertsListView ListView { get; set; }
        public VMCertStoreServerDTO ServerDTO { get; protected set; }
        public string StoreName { get; protected set; }

        public VecsStoreNode(VMCertStoreServerDTO dto, string storeName)
        {
            DisplayName = storeName;
            StoreName = storeName;
            ServerDTO = dto;
            ImageIndex = SelectedImageIndex = (int)VMCertStoreImageIndex.Store;
            if (MiscUtilsService.IsSystemStore(StoreName))
            {
                this.EnabledStandardVerbs = StandardVerbs.Refresh;
            }
            else
            {
                this.EnabledStandardVerbs = StandardVerbs.Refresh | StandardVerbs.Delete;
            }

            InitConsole();
        }

        void InitConsole()
        {
            this.Children.Add(new VecsPrivateKeysNode(ServerDTO, StoreName));
            this.Children.Add(new VecsSecretKeysNode(ServerDTO, StoreName));
            this.Children.Add(new VecsTrustedCertsNode(ServerDTO, StoreName));
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);
        }

        protected override void OnDelete(SyncStatus status)
        {
            if (!MMCDlgHelper.ShowConfirm(CommonConstants.GetDeleteMsg("store", StoreName)))
                return;

            MMCActionHelper.CheckedExec(delegate
            {
                base.OnDelete(status);
                ServerDTO.VecsClient.DeleteStore(StoreName);
                Parent.Children.Remove(this);
            });
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);
        }
    }
}

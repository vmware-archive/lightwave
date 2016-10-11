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


using VMDir.Common.DTO;
using VMDirSnapIn.UI;
using VMDirSnapIn.Utilities;
using VMDirSnapIn.Views;
using VMIdentity.CommonUtils;
using VMIdentity.CommonUtils.Log;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSnapIn.TreeNodes
{
    class RootNode : BaseTreeNode
    {
        public RootNode(PropertiesControl control)
            : base(null,control)
        {
            this.Text = MMCMiscUtil.GetBrandConfig(CommonConstants.DIR_ROOT);
            ImageIndex = SelectedImageIndex = (int)VMDirIconIndex.Directory;
            this.Tag = "root";
        }
        public override void DoExpand()
        {
            if (Nodes.Count > 0)
                return;
            foreach (var dto in VMDirEnvironment.Instance.LocalData.ServerList)
            {
                this.Nodes.Add(new ServerNode(dto, PropertiesCtl));
            }
            Expand();
        }

        public override void DoSelect()
        {
            if (PropertiesCtl==null)
                VMDirEnvironment.Instance.Logger.Log("PropertiesCtl is null", LogLevel.Error);
            PropertiesCtl.SetEditState(false);
            DoExpand();
        }

        public void AddNewServer()
        {
             MiscUtilsService.CheckedExec(delegate
            {
                var serverDTO = VMDirServerDTO.CreateInstance();
                serverDTO.Server = "";
                var node = new ServerNode(serverDTO, PropertiesCtl);
                node.Login();
                if (node.ServerDTO.IsLoggedIn)
                {
                    VMDirEnvironment.Instance.LocalData.AddServer(serverDTO);
                    this.Nodes.Add(node);
                }
            });
        }
    }
}

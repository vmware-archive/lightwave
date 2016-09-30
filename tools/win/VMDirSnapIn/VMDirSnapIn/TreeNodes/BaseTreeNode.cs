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
using VMDir.Common.DTO;
using VMDirSnapIn.UI;
using VMDirSnapIn.Views;

namespace VMDirSnapIn.TreeNodes
{
    public class BaseTreeNode : TreeNode
    {
        public VMDirServerDTO ServerDTO { get; protected set; }
        public PropertiesControl PropertiesCtl { get; protected set; }
        public BaseTreeNode(VMDirServerDTO serverDTO, PropertiesControl propertiesCtl)
        {
            ServerDTO = serverDTO;
            PropertiesCtl = propertiesCtl;
        }
        public virtual void DoRefresh()
        {
        }
        public virtual void DoBeforeExpand()
        {
        }
        public virtual void DoExpand()
        {
        }
        public virtual void DoAfterExpand()
        {
        }
        public virtual void DoSelect()
        {
        }
        public void AddDummyNode()
        {
            this.Nodes.Add(new TreeNode("") { Tag = null });
        }
        public void ClearDummyNode()
        {
            if (Nodes.Count == 1 && Nodes[0].Tag == null)
            {
                this.Nodes.Clear();
            }
        }
    }
}

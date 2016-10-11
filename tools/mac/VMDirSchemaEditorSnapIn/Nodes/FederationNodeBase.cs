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
using VmIdentity.UI.Common;
using VMDirSchemaEditorSnapIn.Nodes;
using System.Collections.Generic;
using VmIdentity.UI.Common.Utilities;

namespace VMDirSchemaEditorSnapIn
{
    public class FederationNodeBase : ScopeNodeBase
    {
        private VMDirSchemaServerNode serverNode;

        public FederationNodeBase(string name, VMDirSchemaServerNode serverNode)
            : base()
        {
            this.DisplayName = serverNode.ServerDTO.Server;
            this.serverNode = serverNode;
            AddChildren();
        }

        public void AddChildren()
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    this.Children.Clear();
                    IDictionary<string,bool> serverArray = serverNode.ServerDTO.Connection.SchemaConnection.GetAllServerStatus();
                    foreach (var item in serverArray)
                    {
                        var node = new FederationNode(item);
                        this.Children.Add(node);
                    }
                });
        }
    }
}


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
using VMDir.Common.Schema;
using System.Collections.Generic;
using AppKit;
using VmIdentity.UI.Common.Utilities;
using Foundation;
using System.Linq;
using VMDir.Common;
using VMDirSchema;

namespace VMDirSchemaEditorSnapIn.Nodes
{
    public class VMDirSchemaClassBaseNode:VMDirSchemaRootScopeNode
    {
        private ObjectClassManager objectClassManager;
        private List<VMDirSchemaClassEntryNode> objectClasses;

        public VMDirSchemaClassBaseNode(VMDirSchemaServerNode node)
            : base(node)
        {
            this.DisplayName = VMDirSchemaConstants.VMDIRSCHEMA_CLASSES;
            objectClasses = new List<VMDirSchemaClassEntryNode>();
            PopulateChildren();
        }

        public void PopulateChildren()
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    this.Children.Clear();
                    objectClassManager = this.ServerNode.ServerDTO.Connection.SchemaManager.GetObjectClassManager();
                    var data = objectClassManager.Data;
                    foreach (var entry in data)
                    {
                        objectClasses.Add(new VMDirSchemaClassEntryNode(this.ServerNode, entry.Value));
                    }
                    objectClasses = objectClasses.OrderBy(o => o.DisplayName).ToList();
                    this.Children.AddRange(objectClasses);
                });
        }

        public void AddObjectClass(object sender, EventArgs args)
        {
            ObjectClassWindowController obc = new ObjectClassWindowController(this.ServerNode.ServerDTO.Connection.SchemaManager);
            nint ret = NSApplication.SharedApplication.RunModalForWindow(obc.Window);
            if (ret == VMIdentityConstants.DIALOGOK)
            {
                UIErrorHelper.CheckedExec(delegate()
                    {
                        this.ServerNode.ServerDTO.Connection.AddObjectClass(obc.ObjectDTO);
                        this.ServerNode.ServerDTO.Connection.SchemaManager.RefreshSchema();
                        NSNotificationCenter.DefaultCenter.PostNotificationName(VMIdentityConstants.REFRESH_UI, this);
                        UIErrorHelper.ShowAlert(VMDirSchemaConstants.CLASS_ADD_MESSAGE, VMIdentityConstants.SUCCESS);
                    });

            }
        }
    }
}


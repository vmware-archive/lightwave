﻿/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using VMDirSchema;
using Foundation;
using System.Linq;

namespace VMDirSchemaEditorSnapIn.Nodes
{
    public class VMDirSchemaAttributeBaseNode:VMDirSchemaRootScopeNode
    {
        private AttributeTypeManager attrTypeManager;

        public List<VMDirSchemaAttributeEntryNode> Attributes { get; set; }

        public VMDirSchemaAttributeBaseNode(VMDirSchemaServerNode node)
            : base(node)
        {
            this.DisplayName = VMDirSchemaConstants.VMDIRSCHEMA_ATTRIBUTES;
            FillAttributes();
        }

        public void Refresh()
        {
            FillAttributes();
        }

        private void FillAttributes()
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    Attributes = new List<VMDirSchemaAttributeEntryNode>();
                    attrTypeManager = this.ServerNode.ServerDTO.Connection.SchemaManager.GetAttributeTypeManager();
                    var data = attrTypeManager.Data;
                    foreach (var entry in data)
                    {
                        Attributes.Add(new VMDirSchemaAttributeEntryNode(this.ServerNode, entry.Value));
                    }
                    Attributes = Attributes.OrderBy(x => x.DisplayName).ToList();
                });
        }


        public void AddNewAttribute(object sender, EventArgs args)
        {
            AttributeTypeWindowController awc = new AttributeTypeWindowController();
            nint ret = NSApplication.SharedApplication.RunModalForWindow(awc.Window);
            if (ret == VMIdentityConstants.DIALOGOK)
            {
                UIErrorHelper.CheckedExec(delegate()
                    {
                        this.ServerNode.ServerDTO.Connection.AddAttributeType(awc.AttributeDTO);
                        this.ServerNode.ServerDTO.Connection.SchemaManager.RefreshSchema();
                        NSNotificationCenter.DefaultCenter.PostNotificationName(VMIdentityConstants.RELOAD_TABLEVIEW, this);
                        UIErrorHelper.ShowAlert(VMDirSchemaConstants.ATTR_ADD_MESSAGE, VMIdentityConstants.SUCCESS);
                    });
            }
        }
    }
}


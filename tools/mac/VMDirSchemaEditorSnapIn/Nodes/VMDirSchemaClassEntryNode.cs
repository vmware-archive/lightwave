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
using VMDir.Common.Schema;
using AppKit;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;
using Foundation;
using VMDirSchema;

namespace VMDirSchemaEditorSnapIn.Nodes
{
    public class VMDirSchemaClassEntryNode:VMDirSchemaRootScopeNode
    {
        public ObjectClassDTO ObjectClassDto { get; set; }

        public VMDirSchemaClassEntryNode(VMDirSchemaServerNode node, ObjectClassDTO dto)
            : base(node)
        {
            this.DisplayName = dto.Name;
            ObjectClassDto = dto;
        }

        public void ShowProperties(object sender, EventArgs args)
        {
            ObjectClassWindowController obc = new ObjectClassWindowController(ObjectClassDto, this.ServerNode.ServerDTO.Connection.SchemaManager);
            nint ret = NSApplication.SharedApplication.RunModalForWindow(obc.Window);
            if (ret == VMIdentityConstants.DIALOGOK)
            {
                UIErrorHelper.CheckedExec(delegate()
                    {
                        this.ServerNode.ServerDTO.Connection.ModifyObjectClass(obc.ObjectClassModDTO);
                        this.ServerNode.ServerDTO.Connection.SchemaManager.RefreshSchema();
                        NSNotificationCenter.DefaultCenter.PostNotificationName(VMIdentityConstants.REFRESH_UI, this);
                        UIErrorHelper.ShowAlert(VMDirSchemaConstants.CLASS_MODIFY_MESSAGE, VMIdentityConstants.SUCCESS);
                    });

            }
        }
    }
}


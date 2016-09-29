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
using System;
using System.Windows.Forms;
using VMDir.Common.Schema;
using VMDirSchemaSnapIn;
using VMDirSchemaSnapIn.ListViews;
using VMDirSchemaSnapIn.Nodes;
using VMDirSchemaSnapIn.UI;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSchemaEditorSnapIn.Nodes
{
    public class VMDirSchemaClassEntryNode:VMSchemaRootNode
    {
        public ObjectClassDTO ObjectClassDto { get; set; }
        public ClassAttributesListView ListView { get; set; }
        protected const int ACTION_SCHEMAPROPERTIES = 5;

        public VMDirSchemaClassEntryNode(VMDirSchemaServerNode node, ObjectClassDTO dto)
            : base(node)
        {
            this.DisplayName = dto.Name;
            ObjectClassDto = dto;
            InitConsole();
        }


        private void InitConsole()
        {
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(VMDirSchemaConstants.VMDIRSCHEMA_PROPERTIES, VMDirSchemaConstants.VMDIRSCHEMA_PROPERTIES, -1, ACTION_SCHEMAPROPERTIES));
            this.ImageIndex = this.SelectedImageIndex = (int)VMDirSchemaTreeImageIndex.ObjectClass;
            var lvd = new MmcListViewDescription();
            lvd.DisplayName = VMDirSchemaConstants.VMDIRSCHEMA_CLASSES;
            lvd.ViewType = typeof(ClassAttributesListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes | MmcListViewOptions.SingleSelect;
            AddViewDescription(this, lvd);
        }

        private void AddViewDescription(ScopeNode node, MmcListViewDescription lvd)
        {
            node.ViewDescriptions.Add(lvd);
            node.ViewDescriptions.DefaultIndex = 0;
        }

        protected override void OnExpand(AsyncStatus status)
        {
            //override and do nothing
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_SCHEMAPROPERTIES:
                    ShowProperties();
                    break;
            }
        }

        public void ShowProperties()
        {
            var frm = new ObjectClassWindow(ObjectClassDto, this.ServerNode.ServerDTO.Connection.SchemaManager);
            if (SnapIn.Console.ShowDialog(frm) == DialogResult.OK)
            {
                UIErrorHelper.CheckedExec(delegate(){
                 this.ServerNode.ServerDTO.Connection.ModifyObjectClass(frm.ObjectClassModDTO);
                 this.ServerNode.ServerDTO.Connection.SchemaManager.RefreshSchema();
                 (this.Parent as VMDirSchemaClassBaseNode).RefreshChildren();
                 MMCDlgHelper.ShowInformation(VMDirSchemaConstants.CLASS_MODIFY_MESSAGE);
                 });
            }
        }
    }
}


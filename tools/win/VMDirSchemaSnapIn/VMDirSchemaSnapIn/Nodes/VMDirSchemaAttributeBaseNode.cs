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
using System.Collections.Generic;
using System.Linq;
using VMDirSchemaSnapIn;
using VMDirSchemaSnapIn.Nodes;
using Microsoft.ManagementConsole;
using VMDirSchemaSnapIn.ListViews;
using VMDirSchemaSnapIn.UI;
using System.Windows.Forms;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSchemaEditorSnapIn.Nodes
{
    public class VMDirSchemaAttributeBaseNode:VMSchemaRootNode
    {
        private AttributeTypeManager attrTypeManager;

        public List<VMDirSchemaAttributeEntryNode> Attributes { get; set; }
        public AttributesListView ListView { get; set; }
        protected const int ACTION_SCHEMA_ADDATTRIBUTE = 3;

        public VMDirSchemaAttributeBaseNode(VMDirSchemaServerNode node)
            : base(node)
        {
            this.DisplayName = VMDirSchemaConstants.VMDIRSCHEMA_ATTRIBUTES;
            FillAttributes();
            InitConsole();
        }

        public  void RefreshNode()
        {
            FillAttributes();
        }
        protected override void OnExpand(AsyncStatus status)
        {
            //override and do nothing
        }


        private void AddViewDescription(ScopeNode node, MmcListViewDescription lvd)
        {
            node.ViewDescriptions.Add(lvd);
            node.ViewDescriptions.DefaultIndex = 0;
        }

        private void InitConsole()
        {
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(VMDirSchemaConstants.VMDIRSCHEMA_ADDATTRIBUTE, VMDirSchemaConstants.VMDIRSCHEMA_ADDATTRIBUTE, -1, ACTION_SCHEMA_ADDATTRIBUTE));
            this.ImageIndex = this.SelectedImageIndex = (int)VMDirSchemaTreeImageIndex.Attribute;
            var lvd = new MmcListViewDescription();
            lvd.DisplayName = "Attributes";
            lvd.ViewType = typeof(AttributesListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes | MmcListViewOptions.SingleSelect;
            AddViewDescription(this, lvd);
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_SCHEMA_ADDATTRIBUTE:
                    AddNewAttribute();
                    break;
            }
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


        public void AddNewAttribute()
        {
            var frm = new AttributeTypeWindow();
            if (MMCDlgHelper.ShowForm(frm))
            {
              UIErrorHelper.CheckedExec(delegate()
              {
                  this.ServerNode.ServerDTO.Connection.AddAttributeType(frm.AttributeDTO);
                  this.ServerNode.ServerDTO.Connection.SchemaManager.RefreshSchema();
                  this.RefreshNode();
                  this.ListView.Refresh();
                  MMCDlgHelper.ShowMessage(VMDirSchemaConstants.ATTR_ADD_MESSAGE);
              });
            }
        }
    }
}


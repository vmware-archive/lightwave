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
using VMDir.Common;
using Microsoft.ManagementConsole;
using VMDirSchemaSnapIn;
using VMDirSchemaSnapIn.Nodes;
using VMDirSchemaSnapIn.ListViews;
using VMDirSchemaSnapIn.UI;
using System.Windows.Forms;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSchemaEditorSnapIn.Nodes
{
    public class VMDirSchemaClassBaseNode : VMSchemaRootNode
    {
        private ObjectClassManager objectClassManager;
        private List<VMDirSchemaClassEntryNode> objectClasses;
        public ObjectClassesListView ListView { get; set; }
        protected const int ACTION_SCHEMA_ADDCLASS = 4;

        public VMDirSchemaClassBaseNode(VMDirSchemaServerNode node)
            : base(node)
        {
            this.DisplayName = VMDirSchemaConstants.VMDIRSCHEMA_CLASSES;
            objectClasses = new List<VMDirSchemaClassEntryNode>();
            RefreshChildren();
            InitConsole();
        }

        protected override void OnExpand(AsyncStatus status)
        {
            //override and do nothing
        }

        private void InitConsole()
        {
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(VMDirSchemaConstants.VMDIRSCHEMA_ADDCLASS, VMDirSchemaConstants.VMDIRSCHEMA_ADDCLASS, -1, ACTION_SCHEMA_ADDCLASS));
            this.ImageIndex = this.SelectedImageIndex = (int)VMDirSchemaTreeImageIndex.ObjectClassBase;
            var lvd = new MmcListViewDescription();
            lvd.DisplayName = "ObjectClasses";
            lvd.ViewType = typeof(ObjectClassesListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes | MmcListViewOptions.SingleSelect;
            AddViewDescription(this, lvd);
        }

        private void AddViewDescription(ScopeNode node, MmcListViewDescription lvd)
        {
            node.ViewDescriptions.Add(lvd);
            node.ViewDescriptions.DefaultIndex = 0;
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_SCHEMA_ADDCLASS:
                    AddObjectClass();
                    break;
            }
        }

        public void RefreshChildren()
        {
            UIErrorHelper.CheckedExec(delegate()
            {
                this.Children.Clear();
                this.objectClasses.Clear();
                objectClassManager = this.ServerNode.ServerDTO.Connection.SchemaManager.GetObjectClassManager();
                var data = objectClassManager.Data;
                foreach (var entry in data)
                {
                    objectClasses.Add(new VMDirSchemaClassEntryNode(this.ServerNode, entry.Value));
                }
                objectClasses = objectClasses.OrderBy(o => o.DisplayName).ToList();
                this.Children.AddRange(objectClasses.ToArray());
            });
        }

        public void AddObjectClass()
        {
            var frm = new ObjectClassWindow(this.ServerNode.ServerDTO.Connection.SchemaManager);
            if (SnapIn.Console.ShowDialog(frm) == DialogResult.OK)
            {
              UIErrorHelper.CheckedExec(delegate()
              {
                  this.ServerNode.ServerDTO.Connection.AddObjectClass(frm.ObjectDTO);
                  this.ServerNode.ServerDTO.Connection.SchemaManager.RefreshSchema();
                  this.RefreshChildren();
                  this.ListView.Refresh();

                  MMCDlgHelper.ShowMessage(VMDirSchemaConstants.CLASS_ADD_MESSAGE);
              });
 
            }
        }
    }
}


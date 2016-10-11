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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VMDir.Common.Schema;
using VMDirSchemaEditorSnapIn.Nodes;
using VMDirSchemaSnapIn.UI;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSchemaSnapIn.ListViews
{
    public class ObjectClassesListView : MmcListView
    {
        const int ACTION_SHOW_PROPERTIES = 3;
        public IList<VMDirSchemaClassEntryNode> Entries { get; set; }

        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);

            this.Columns[0].Title = VMDirSchemaConstants.VMDIRSCHEMA_NAME;
            this.Columns[0].SetWidth(130);

            this.Columns.Add(new MmcListViewColumn(VMDirSchemaConstants.VMDIRSCHEMA_CLASS_TYPE, 130));
            this.Columns.Add(new MmcListViewColumn(VMDirSchemaConstants.VMDIRSCHEMA_DESC, 130));
            this.Mode = MmcListViewMode.Report;

            (this.ScopeNode as VMDirSchemaClassBaseNode).ListView = this;

            UIErrorHelper.CheckedExec(delegate()
            {
                Refresh();
            });
        }

        public void Refresh()
        {
            ResultNodes.Clear();
            int nodeCounter = 0;
            this.Entries = this.ScopeNode.Children.OfType<VMDirSchemaClassEntryNode>().ToList();
            foreach (var item in this.Entries)
            {
                var resultNode = new ResultNode { DisplayName = item.DisplayName };
                resultNode.ImageIndex = (int)VMDirSchemaTreeImageIndex.ObjectClass;
                resultNode.Tag = nodeCounter++;
                resultNode.SubItemDisplayNames.Add(item.ObjectClassDto.GetObjectClassTypeAsString());
                resultNode.SubItemDisplayNames.Add(item.ObjectClassDto.Description);

                this.ResultNodes.Add(resultNode);
            }
            this.Sort(0);
            this.DescriptionBarText = this.ResultNodes.Count.ToString();
        }


        protected override void OnSelectionAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            switch ((int)action.Tag)
            {
                case ACTION_SHOW_PROPERTIES:
                    ShowProperties();
                    break;
            }
        }

        protected override void OnSelectionChanged(SyncStatus status)
        {
            if (this.SelectedNodes.Count == 0)
            {
                this.SelectionData.Clear();
            }
            else
            {
                this.SelectionData.Update(GetDetails(), this.SelectedNodes.Count > 1, null, null);
                this.SelectionData.ActionsPaneItems.Clear();
                this.SelectionData.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action(VMDirSchemaConstants.VMDIRSCHEMA_PROPERTIES,
                                                      VMDirSchemaConstants.VMDIRSCHEMA_PROPERTIES, 6, ACTION_SHOW_PROPERTIES));
            }
        }

        private string GetDetails()
        {
            StringBuilder nodedetails = new StringBuilder();

            foreach (ResultNode resultNode in this.SelectedNodes)
            {
                nodedetails.Append(resultNode.DisplayName + ":   " + resultNode.SubItemDisplayNames[0].ToString() + "\n");
            }
            return nodedetails.ToString();
        }

        void ShowProperties()
        {
            var frm = new ObjectClassWindow(this.Entries[(int)this.SelectedNodes[0].Tag].ObjectClassDto, (this.ScopeNode as VMDirSchemaClassBaseNode).ServerNode.ServerDTO.Connection.SchemaManager);
            if (SnapIn.Console.ShowDialog(frm) == DialogResult.OK)
            {
                UIErrorHelper.CheckedExec(delegate()
                {
                    (this.ScopeNode as VMDirSchemaClassBaseNode).ServerNode.ServerDTO.Connection.ModifyObjectClass(frm.ObjectClassModDTO);
                    (this.ScopeNode as VMDirSchemaClassBaseNode).ServerNode.ServerDTO.Connection.SchemaManager.RefreshSchema();
                    (this.ScopeNode as VMDirSchemaClassBaseNode).RefreshChildren();
                    this.Refresh();

                    MMCDlgHelper.ShowMessage(VMDirSchemaConstants.CLASS_MODIFY_MESSAGE);
                });
            }
        }
    }
}

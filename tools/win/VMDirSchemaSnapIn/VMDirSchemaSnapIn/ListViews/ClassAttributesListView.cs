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
using VMDir.Common.Schema;
using VMDirSchemaEditorSnapIn.Nodes;

namespace VMDirSchemaSnapIn.ListViews
{
    public class ClassAttributesListView : MmcListView
    {
        const int ACTION_SHOW_PROPERTIES = 3;
        public List<AttributeTypeDTO> Entries { get; set; }
        public VMDirSchemaClassEntryNode ClassNode { get; set; }

        protected override void OnInitialize(AsyncStatus status)
        {
            base.OnInitialize(status);

            this.Columns[0].Title = VMDirSchemaConstants.VMDIRSCHEMA_NAME;
            this.Columns[0].SetWidth(130);

            this.Columns.Add(new MmcListViewColumn(VMDirSchemaConstants.VMDIRSCHEMA_OPTIONAL_ATTR, 130));
            this.Columns.Add(new MmcListViewColumn(VMDirSchemaConstants.VMDIRSCHEMA_ATTR_SYNTAX, 130));
            this.Mode = MmcListViewMode.Report;
            ClassNode = this.ScopeNode as VMDirSchemaClassEntryNode;
            ClassNode.ListView = this;

            UIErrorHelper.CheckedExec(delegate()
            {
                Refresh();
            });
        }

        public void Refresh()
        {
            ResultNodes.Clear();
            int nodeCounter = 0;

            List<AttributeTypeDTO> required = ClassNode.ServerNode.ServerDTO.Connection.SchemaManager.GetRequiredAttributes(ClassNode.DisplayName);
            required.ForEach(x => x.IsOptional = false);
            List<AttributeTypeDTO> optional = ClassNode.ServerNode.ServerDTO.Connection.SchemaManager.GetOptionalAttributes(ClassNode.DisplayName);
            optional.ForEach(x => x.IsOptional = true);
            Entries = required;
            Entries.AddRange(optional);

            foreach (var item in Entries)
            {
                var resultNode = new ResultNode { DisplayName = item.Name };
                resultNode.ImageIndex = (int)VMDirSchemaTreeImageIndex.Attribute;
                resultNode.Tag = nodeCounter++;
                resultNode.SubItemDisplayNames.Add(Convert.ToString(item.IsOptional));
                resultNode.SubItemDisplayNames.Add(item.Type);

                this.ResultNodes.Add(resultNode);
            }
            this.Sort(0);
            this.DescriptionBarText = this.ResultNodes.Count.ToString();
        }

    }
}

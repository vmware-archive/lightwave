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
using AppKit;
using VMDirSchemaEditorSnapIn.Nodes;
using System.Collections.Generic;
using Foundation;
using VMDirSchema;
using VMDir.Common.Schema;

namespace VMDirSchemaEditorSnapIn.ListViews
{
    public class ClassAttributesEntryListView : NSTableViewDataSource
    {
        public List<AttributeTypeDTO> AttributeEntries { get; set; }

        public VMDirSchemaClassEntryNode ClassNode { get; set; }

        public ClassAttributesEntryListView(VMDirSchemaClassEntryNode node)
        {
            ClassNode = node;
            //fetch attributes from schema manager for this object class.
            if (node != null)
            {
                List<AttributeTypeDTO> required = node.ServerNode.ServerDTO.Connection.SchemaManager.GetRequiredAttributes(ClassNode.DisplayName);
                required.ForEach(x => x.IsOptional = false);
                List<AttributeTypeDTO> optional = node.ServerNode.ServerDTO.Connection.SchemaManager.GetOptionalAttributes(ClassNode.DisplayName);
                optional.ForEach(x => x.IsOptional = true);
                AttributeEntries = required;
                AttributeEntries.AddRange(optional);
				AttributeEntries.Sort((x, y) => string.Compare(x.Name,y.Name));
            }
        }

        // This method will be called by the NSTableView control to learn the number of rows to display.
        [Export("numberOfRowsInTableView:")]
        public int NumberOfRowsInTableView(NSTableView table)
        {
            return (AttributeEntries != null) ? AttributeEntries.Count : 0;
        }

        // This method will be called by the control for each column and each row.
        [Export("tableView:objectValueForTableColumn:row:")]
        public NSObject ObjectValueForTableColumn(NSTableView table, NSTableColumn col, int row)
        {
            NSString value = NSString.Empty;
            try
            {
                if (AttributeEntries != null)
                {
                    AttributeTypeDTO record = AttributeEntries[row];
                    switch (col.Identifier)
                    {
                        case VMDirSchemaConstants.VMDIRSCHEMA_NAME:
                            value = (NSString)record.Name;
                            break;
                        case VMDirSchemaConstants.VMDIRSCHEMA_OPTIONAL_ATTR:
                            value = (NSString)Convert.ToString(record.IsOptional);
                            break;
                        case VMDirSchemaConstants.VMDIRSCHEMA_ATTR_SYNTAX:
                            value = (NSString)record.Type;
                            break;
                    }
                }
            }
            catch (Exception)
            {
                value = NSString.Empty;
            }
            return value;
        }
    }
}


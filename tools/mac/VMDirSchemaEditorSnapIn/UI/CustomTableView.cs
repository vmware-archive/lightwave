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
using AppKit;
using CoreGraphics;
using Foundation;
using VMDirSchemaEditorSnapIn.ListViews;
using VMDirSchemaEditorSnapIn.Nodes;
using VMDirSchema;

namespace VMDirSchemaEditorSnapIn.UI
{
    [Register("CustomTableView")]
    public class CustomTableView:NSTableView
    {
        [Export("init")]
        public CustomTableView()
            : base()
        {
        }

        [Export("initWithCoder:")]
        public CustomTableView(NSCoder coder)
            : base(coder)
        {
        }

        public CustomTableView(IntPtr handle)
            : base(handle)
        {

        }

        public override NSMenu MenuForEvent(NSEvent theEvent)
        {
            NSTableViewDataSource ds = (NSTableViewDataSource)this.DataSource;
            NSMenu menu = new NSMenu();
            if (SelectedRow >= (nint)0)
            {
                if (ds is AttributesEntryListView)
                {
                    VMDirSchemaAttributeEntryNode node = (ds as AttributesEntryListView).Entries[(int)SelectedRow];
                    NSMenuItem properties = new NSMenuItem(VMDirSchemaConstants.VMDIRSCHEMA_PROPERTIES, node.ShowProperties);
                    menu.AddItem(properties);
                }
                else if (ds is ObjectClassesListView)
                {
                    VMDirSchemaClassEntryNode node = (ds as ObjectClassesListView).Entries[(int)SelectedRow];
                    NSMenuItem properties = new NSMenuItem(VMDirSchemaConstants.VMDIRSCHEMA_PROPERTIES, node.ShowProperties);
                    menu.AddItem(properties);
                }
                else
                {
                    //do nothing
                }
            }
            NSMenu.PopUpContextMenu(menu, theEvent, theEvent.Window.ContentView);
            return base.MenuForEvent(theEvent);
        }
    }
}


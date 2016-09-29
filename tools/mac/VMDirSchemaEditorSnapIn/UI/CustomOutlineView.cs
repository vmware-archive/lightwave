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
using VMDirSchemaEditorSnapIn.Nodes;
using VMDirSchema;

namespace VMDirSchemaEditorSnapIn.UI
{
    [Register("CustomOutlineView")]
    public class CustomOutlineView:NSOutlineView
    {
        [Export("init")]
        public CustomOutlineView()
            : base()
        {
        }

        [Export("initWithCoder:")]
        public CustomOutlineView(NSCoder coder)
            : base(coder)
        {
        }

        public CustomOutlineView(IntPtr handle)
            : base(handle)
        {
        }

        public override NSMenu MenuForEvent(NSEvent theEvent)
        {
            if (SelectedRow >= (nint)0)
            {
                NSObject obj = this.ItemAtRow(SelectedRow);
                if (obj != null)
                {
                    NSMenu menu = new NSMenu();

                    if (obj is VMDirSchemaClassEntryNode)
                    {
                        var node = obj as VMDirSchemaClassEntryNode;
                        NSMenuItem addAttribute = new NSMenuItem(VMDirSchemaConstants.VMDIRSCHEMA_PROPERTIES, node.ShowProperties);
                        menu.AddItem(addAttribute);
                    }
                    else if (obj is VMDirSchemaClassBaseNode)
                    {
                        var node = obj as VMDirSchemaClassBaseNode;
                        NSMenuItem addObjectClass = new NSMenuItem(VMDirSchemaConstants.VMDIRSCHEMA_ADDCLASS, node.AddObjectClass);
                        menu.AddItem(addObjectClass);
                    }
                    else if (obj is VMDirSchemaAttributeBaseNode)
                    {
                        var node = obj as VMDirSchemaAttributeBaseNode;
                        NSMenuItem addAttribute = new NSMenuItem(VMDirSchemaConstants.VMDIRSCHEMA_ADDATTRIBUTE, node.AddNewAttribute);
                        menu.AddItem(addAttribute);
                    }
                    else
                    {
                        //do nothing
                    }
                    NSMenu.PopUpContextMenu(menu, theEvent, theEvent.Window.ContentView);
                }
            }
            return base.MenuForEvent(theEvent);
        }
    }
}


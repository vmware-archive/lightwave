/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using AppKit;
using Foundation;
using VMDirSnapIn.Nodes;

namespace VMDirSnapIn.UI
{
    [Register ("CustomOutlineView")]
    public class OutlineView : NSOutlineView
    {
        [Foundation.Export ("init")]
        public OutlineView ()
        {
        }

        [Foundation.Export ("initWithCoder:")]
        public OutlineView (NSCoder coder) : base (coder)
        {
        }

        public OutlineView (IntPtr handle) : base (handle)
        {
        }

        //Right click menu event for the outlineview.
        public override NSMenu MenuForEvent (NSEvent theEvent)
        {
            int row = (int)this.SelectedRow;
            if (row >= (nint)0) {
                NSObject obj = this.ItemAtRow (row);
                if (obj != null) {
                    NSMenu menu = new NSMenu ();
                    if (obj is DirectoryNode) {
                        DirectoryNode node = obj as DirectoryNode;
                        if (node.NodeType == DirectoryNode.DirectoryNodeType.User) {
                            NSMenuItem addUsertoGroup = new NSMenuItem ("Add to a Group", node.AddUserToGroup);
                            menu.AddItem (addUsertoGroup);
                            NSMenuItem resetPassword = new NSMenuItem ("Set Password", node.RestUserPassword);
                            menu.AddItem (resetPassword);
                        } else if (node.NodeType == DirectoryNode.DirectoryNodeType.Groups) {
                            NSMenuItem addGrouptoGroup = new NSMenuItem ("Add to a Group", node.AddUserToGroup);
                            menu.AddItem (addGrouptoGroup);
                        } else {
                            NSMenuItem addUser = new NSMenuItem ("Add User", node.AddUser);
                            menu.AddItem (addUser);
                            NSMenuItem addGroup = new NSMenuItem ("Add Group", node.AddGroup);
                            menu.AddItem (addGroup);
                            NSMenuItem add = new NSMenuItem ("Add Object", node.Add);
                            menu.AddItem (add);
                        }
                        NSMenuItem delete = new NSMenuItem ("Delete", node.Delete);
                        menu.AddItem (delete);
                        NSMenuItem properties = new NSMenuItem ("Properties", node.ViewProperties);
                        menu.AddItem (properties);
                        NSMenuItem refresh = new NSMenuItem ("Refresh", node.RefreshNode);
                        menu.AddItem (refresh);
                    }
                    NSMenu.PopUpContextMenu (menu, theEvent, theEvent.Window.ContentView);
                }
            }
            return base.MenuForEvent (theEvent);
        }
    }
}


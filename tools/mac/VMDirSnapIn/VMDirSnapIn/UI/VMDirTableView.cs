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
using VMDirSnapIn.DataSource;
using VMDirSnapIn.Nodes;
using Foundation;
using AppKit;
using CoreGraphics;

namespace VMDirSnapIn.UI
{
    [Register ("CustomTableView")]
    public class VMDirTableView : NSTableView
    {
        private nint _selectedRow;

        [Export ("init")]
        public VMDirTableView () : base ()
        {
        }

        [Export ("initWithCoder:")]
        public VMDirTableView (NSCoder coder) : base (coder)
        {
        }

        public VMDirTableView (IntPtr handle) : base (handle)
        {

        }

        //Handle right click event for the TableView
        public override NSMenu MenuForEvent (NSEvent theEvent)
        {
            CGPoint pt = this.ConvertPointFromView (theEvent.LocationInWindow, null);
            _selectedRow = this.GetRow (pt);
            NSTableViewDataSource ds = (NSTableViewDataSource)this.DataSource;
            NSMenu menu = new NSMenu ();
            if (_selectedRow >= (nint)0) {
                if (ds is NodesListView) {
                    DirectoryNode node = ((ds as NodesListView).Entries [(int)_selectedRow] as DirectoryNode);
                    if (node != null) {
                        if (node.NodeType == DirectoryNode.DirectoryNodeType.User) {
                            NSMenuItem ResetPassword = new NSMenuItem ("Set Password", node.RestUserPassword); 
                            menu.AddItem (ResetPassword);
                            NSMenuItem delete = new NSMenuItem ("Delete", node.Delete); 
                            menu.AddItem (delete);
                            NSMenuItem Properties = new NSMenuItem ("Properties", node.ViewProperties); 
                            menu.AddItem (Properties);
                        } else if (node.NodeType == DirectoryNode.DirectoryNodeType.Groups) {
                            NSMenuItem addUser = new NSMenuItem ("Add user to group", node.AddUserToGroup); 
                            menu.AddItem (addUser);
                        }
                       
                    }
                }
            }
            NSMenu.PopUpContextMenu (menu, theEvent, theEvent.Window.ContentView);
            return base.MenuForEvent (theEvent);
        }


    }
}


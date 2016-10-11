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
using Foundation;
using VMDirSnapIn.Nodes;
using VMDir.Common;

namespace VMDirSnapIn.UI
{
    [Register ("VmdirOutlineView")]
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

						NSMenuItem search = new NSMenuItem("Search", node.Search);
						menu.AddItem(search);
						NSMenuItem fetchNextPage = new NSMenuItem("Fetch Next Page", node.FetchNextPage);
						menu.AddItem(fetchNextPage);
						NSMenuItem refresh = new NSMenuItem("Refresh", node.RefreshNode);
						menu.AddItem(refresh);
						NSMenuItem delete = new NSMenuItem("Delete", node.Delete);
						menu.AddItem(delete);

						if (node.ObjectClass.Contains(VMDirConstants.USER_OC)){
							menu.AddItem(NSMenuItem.SeparatorItem);
							NSMenuItem addUsertoGroup = new NSMenuItem("Add to a Group", node.AddUserToGroup);
							menu.AddItem(addUsertoGroup);
							NSMenuItem resetPassword = new NSMenuItem("Reset Password", node.RestUserPassword);
							menu.AddItem(resetPassword);
							NSMenuItem verifyUserPassword = new NSMenuItem("Verify Password", node.VerifyUserPassword);
							menu.AddItem(verifyUserPassword);
							}
						else if (node.ObjectClass.Contains(VMDirConstants.GROUP_OC))
						{
							menu.AddItem(NSMenuItem.SeparatorItem);
							NSMenuItem addGrouptoGroup = new NSMenuItem("Add to a Group", node.AddUserToGroup);
							menu.AddItem(addGrouptoGroup);
						}
						menu.AddItem(NSMenuItem.SeparatorItem);
						NSMenuItem addUser = new NSMenuItem("Add User", node.AddUser);
						menu.AddItem(addUser);
						NSMenuItem addGroup = new NSMenuItem("Add Group", node.AddGroup);
						menu.AddItem(addGroup);
						NSMenuItem add = new NSMenuItem("Add Object", node.Add);
						menu.AddItem(add);
                    }
                    NSMenu.PopUpContextMenu (menu, theEvent, theEvent.Window.ContentView);
                }
            }
            return base.MenuForEvent (theEvent);
        }
    }
}


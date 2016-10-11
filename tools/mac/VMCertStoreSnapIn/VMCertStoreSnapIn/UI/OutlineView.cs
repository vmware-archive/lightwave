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
using VMCertStoreSnapIn.Nodes;
using AppKit;
using Foundation;
using CoreGraphics;

namespace VMCertStoreSnapIn
{
    [Register ("CustomOutlineView")]
    public class OutlineView : NSOutlineView
    {
        [Export ("init")]
        public OutlineView ()
        {
        }

        [Export ("initWithCoder:")]
        public OutlineView (NSCoder coder) : base (coder)
        {
        }

        public OutlineView (IntPtr handle) : base (handle)
        {
        }

        public override NSMenu MenuForEvent (NSEvent theEvent)
        {
            CGPoint pt = this.ConvertPointFromView (theEvent.LocationInWindow, null);
            nint row = this.GetRow (pt);
            if (row >= (nint)0) {
                NSObject obj = this.ItemAtRow (row);
                if (obj != null) {
                    NSMenu menu = new NSMenu ();
                    if (obj is VecsStoresNode) {
                        VecsStoresNode node = obj as VecsStoresNode;
                        NSMenuItem createStore = new NSMenuItem ("Create Store", node.CreateStore);
                        menu.AddItem (createStore);
                    } else if (obj is VecsPrivateKeysNode) {
                        VecsPrivateKeysNode node = obj as VecsPrivateKeysNode;
                        NSMenuItem addPrivate = new NSMenuItem ("Add Private Key", node.AddPrivateKeyHandler);
                        menu.AddItem (addPrivate);
                    } else if (obj is VecsSecretKeysNode) {
                        VecsSecretKeysNode node = obj as VecsSecretKeysNode;
                        NSMenuItem addSecret = new NSMenuItem ("Add  Secret Key", node.AddSecretKey);
                        menu.AddItem (addSecret);
                    } else if (obj is VecsTrustedCertsNode) {
                        VecsTrustedCertsNode node = obj as VecsTrustedCertsNode;
                        NSMenuItem createCertificate = new NSMenuItem ("Add Certificate", node.AddCertificate);
                        menu.AddItem (createCertificate);
                    } else if (obj is VecsStoreNode) {
                        VecsStoreNode node = obj as VecsStoreNode;
                        NSMenuItem deleteStore = new NSMenuItem ("Delete Store", node.DeleteStore);
                        //disable delete option for the following three certstores.
                        if (node.StoreName.Equals ("MACHINE_SSL_CERT") || node.StoreName.Equals ("TRUSTED_ROOTS") || node.StoreName.Equals ("TRUSTED_ROOT_CRLS"))
                            deleteStore.Hidden = true;
                        menu.AddItem (deleteStore);

                    }
                    NSMenu.PopUpContextMenu (menu, theEvent, theEvent.Window.ContentView);
                }
            }
            return base.MenuForEvent (theEvent);
        }
    }
}


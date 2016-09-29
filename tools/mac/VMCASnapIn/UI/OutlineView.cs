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
using CoreGraphics;
using AppKit;
using Foundation;
using VMCASnapIn.Nodes;

namespace VMCASnapIn.UI
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

        public override NSMenu MenuForEvent (NSEvent theEvent)
        {
            CGPoint pt = this.ConvertPointToView (theEvent.LocationInWindow, null);
            nint row = this.GetRow (pt);
            if (row >= (nint)0) {
                NSObject obj = this.ItemAtRow ((int)row);
                if (obj != null) {
                    NSMenu menu = new NSMenu ();
                    if (obj is VMCAServerNode) {
                        VMCAServerNode serverNode = obj as VMCAServerNode;
                        if (serverNode.IsLoggedIn) {
                            NSMenuItem getVersion = new NSMenuItem ("Get Server Version", serverNode.GetServerVersion);
                            menu.AddItem (getVersion);
                            NSMenuItem showRoot = new NSMenuItem ("Show Root Certificate", serverNode.ShowRootCertificate);
                            menu.AddItem (showRoot);
                            NSMenuItem addCert = new NSMenuItem ("Add Root Certificate", serverNode.AddRootCertificate);
                            menu.AddItem (addCert);
                        }
                    } else if (obj is VMCAKeyPairNode) {
                        VMCAKeyPairNode node = obj as VMCAKeyPairNode;
                        NSMenuItem createKeyPair = new NSMenuItem ("Create KeyPair", node.CreateKeyPair);
                        menu.AddItem (createKeyPair);
                    } else if (obj is VMCACSRNode) {
                        VMCACSRNode node = obj as VMCACSRNode;
                        NSMenuItem createSigningRequest = new NSMenuItem ("Create Signing Request", node.HandleSigningRequest);
                        menu.AddItem (createSigningRequest);
                    } else if (obj is VMCAPersonalCertificatesNode) {
                        VMCAPersonalCertificatesNode node = obj as VMCAPersonalCertificatesNode;
                        NSMenuItem createCertificate = new NSMenuItem ("Create Self Signed Certificate", node.CreateCertificate);
                        menu.AddItem (createCertificate);
                        NSMenuItem createCASingedCertificate = new NSMenuItem ("Create CA Signed Certificate", node.CreateCASignedCertificate);
                        menu.AddItem (createCASingedCertificate);
                    }
                    NSMenu.PopUpContextMenu (menu, theEvent, theEvent.Window.ContentView);
                }
            }
            return base.MenuForEvent (theEvent);
        }
    }
}


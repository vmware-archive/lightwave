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
using VmIdentity.UI.Common.Utilities;
using VmIdentity.UI.Common;
using AppKit;
using VMCertStore.Common.DTO;
using Vecs;
using Foundation;

namespace VMCertStoreSnapIn.Nodes
{
    public class VecsStoresNode : ChildScopeNode
    {
        public VecsStoresNode (VMCertStoreServerNode serverNode) : base (serverNode)
        {
            DisplayName = "Vecs Stores";
            RefreshStores ();
        }

        public void RefreshStores ()
        {
            this.Children.Clear ();
            UIErrorHelper.CheckedExec (delegate() {
                var stores = ServerNode.ServerDTO.VecsClient.GetStores ();

                foreach (var store in stores) {
                    var activeNode = new VecsStoreNode (ServerNode, store, this);
                    this.Children.Add (activeNode);
                }
            });
        }

        public void CreateStore (object sender, EventArgs args)
        {
            CreateStore ();
        }

        public void CreateStore ()
        {
            UIErrorHelper.CheckedExec (delegate() {
                CreateCertificateStoreWindowController cwc = new CreateCertificateStoreWindowController ();
                NSApplication.SharedApplication.BeginSheet (cwc.Window, VMCertStoreSnapInEnvironment.Instance.mainWindow, () => {
                });
                nint result = (nint)NSApplication.SharedApplication.RunModalForWindow (cwc.Window);
                try {
                    if (result == VMIdentityConstants.DIALOGOK) {
                        var dto = cwc.CertStoreDTO;
                        ServerNode.ServerDTO.VecsClient.CreateStore (dto.StoreName, dto.Password);
                        RefreshStores ();
                        ServerNode.UpdateStoreInfo (dto.StoreName);
                        NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
                    }
                } finally {
                    VMCertStoreSnapInEnvironment.Instance.mainWindow.EndSheet (cwc.Window);
                    cwc.Dispose ();
                }
            });
        }
    }
}


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

using VmIdentity.UI.Common.Utilities;
using System;
using AppKit;
using VMCertStore.Common.DTO;
using Vecs;
using VmIdentity.UI.Common;
using Foundation;

namespace VMCertStoreSnapIn.Nodes
{
    public class VecsSecretKeysNode : VecsStoreEntriesNode
    {
        public VecsSecretKeysNode (VMCertStoreServerNode node, string storeName)
            : base (node, storeName)
        {
            DisplayName = "Secret Keys";
            this.Tag = storeName;
        }

        public void AddSecretKey (object sender, EventArgs args)
        {
            UIErrorHelper.CheckedExec (delegate() {
                AddSecretKeyWindowController cwc = new AddSecretKeyWindowController ();
                NSApplication.SharedApplication.BeginSheet (cwc.Window, VMCertStoreSnapInEnvironment.Instance.mainWindow, () => {
                });
                nint result = NSApplication.SharedApplication.RunModalForWindow (cwc.Window);
                try {
                    if (result == (nint)VMIdentityConstants.DIALOGOK) {
                        var dto = cwc.SecretKeyDTO;
                        string storeName = (string)Tag;
                        string storePass = "";
                        using (var session = new VecsStoreSession (ServerNode.ServerDTO.VecsClient, storeName, storePass)) {
                            session.AddSecretKeyEntry (dto.Alias, dto.SecretKey, dto.Password, null);
                        }
                        ServerNode.UpdateStoreInfo (storeName);
                        NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadTableView", this);
                    }
                } finally {
                    VMCertStoreSnapInEnvironment.Instance.mainWindow.EndSheet (cwc.Window);
                    cwc.Dispose ();
                }
            });
        }
    }
}


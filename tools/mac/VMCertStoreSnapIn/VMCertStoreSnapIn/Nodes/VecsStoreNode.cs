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

using VMCertStore.Common.DTO;
using VmIdentity.UI.Common;
using System;
using AppKit;
using Foundation;
using VmIdentity.UI.Common.Utilities;

namespace VMCertStoreSnapIn.Nodes
{
    public class VecsStoreNode:ChildScopeNode
    {
        public string StoreName { get; protected set; }

        public VecsStoreNode (VMCertStoreServerNode node, string storeName, ScopeNode parent) : base (node)
        {
            DisplayName = storeName;
            StoreName = storeName;
            Parent = parent;
            AddChildren ();

        }

        void AddChildren ()
        {
            this.Children.Add (new VecsPrivateKeysNode (ServerNode, StoreName));
            this.Children.Add (new VecsSecretKeysNode (ServerNode, StoreName));
            this.Children.Add (new VecsTrustedCertsNode (ServerNode, StoreName));
        }

        public void DeleteStore (object sender, EventArgs args)
        {
            DeleteStore ();
        }

        private void DeleteStore ()
        {
            UIErrorHelper.CheckedExec (delegate() {
                if (UIErrorHelper.ConfirmDeleteOperation ("Are you sure?") == true) {
                    ServerNode.ServerDTO.VecsClient.DeleteStore (StoreName);
                    this.Children.Clear ();
                    ScopeNode node = this.Parent;
                    if (node != null) {
                        node.Children.Remove (this);
                        ServerNode.RemoveStoreInfo (StoreName);
                        NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", node);
                    }
                }
            });

        }
    }
}


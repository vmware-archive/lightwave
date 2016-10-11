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
using System.Collections.Generic;
using System.Linq;
using Foundation;
using AppKit;
using VMCertStoreSnapIn.Nodes;
using Vecs;
using VmIdentity.UI.Common.Utilities;
using System.Threading.Tasks;

namespace VMCertStoreSnapIn
{
    public partial class ServerInfoPopOverController : AppKit.NSViewController
    {
        VMCertStoreServerNode serverNode;

        #region Constructors

        // Called when created from unmanaged code
        public ServerInfoPopOverController (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public ServerInfoPopOverController (NSCoder coder) : base (coder)
        {
        }

        // Call to load from the XIB/NIB file
        public ServerInfoPopOverController () : base ("ServerInfoPopOver", NSBundle.MainBundle)
        {
        }

        public ServerInfoPopOverController (VMCertStoreServerNode node) : base ("ServerInfoPopOver", NSBundle.MainBundle)
        {
            this.serverNode = node;
            if (serverNode.IsDetailsLoaded == false)
                serverNode.FillServerInfo ();
        }

        #endregion

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();
            //set window background color
            this.View.WantsLayer = true;
            this.View.Layer.BackgroundColor = new CoreGraphics.CGColor (1, 1, (float)1, (float)1);

            NoStoresLabel.StringValue = serverNode.NoStores + " Stores";

            PrivateKeysLabel.StringValue = serverNode.NoPrivateKeys + " Private Keys";
            SecretKeysLabel.StringValue = serverNode.NoSecretKeys + " Secret Keys";
            TrustedCertsLabel.StringValue = serverNode.NoCertificates + " Trusted Certs";

        }

        #region Events


        #endregion

        //strongly typed view accessor
        public new ServerInfoPopOver View {
            get {
                return (ServerInfoPopOver)base.View;
            }
        }
    }
}

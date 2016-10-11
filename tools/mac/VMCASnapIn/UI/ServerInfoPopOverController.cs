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
using VMCASnapIn.Nodes;

namespace VMCASnapIn.UI
{
    public partial class ServerInfoPopOverController : AppKit.NSViewController
    {
        NSPopover parentPopover;
        VMCAServerNode serverNode;

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

        public ServerInfoPopOverController (VMCAServerNode node, NSPopover popover) : base ("ServerInfoPopOver", NSBundle.MainBundle)
        {
            this.parentPopover = popover;
            this.serverNode = node;
        }

        #endregion

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();
            //set window background color
            this.View.WantsLayer = true;
            this.View.Layer.BackgroundColor = new CoreGraphics.CGColor (1, 1, (float)1, (float)1);
            ExpiringCertificatesLabel.StringValue = serverNode.NoOfExpiringCert60days + " Certificates Expiring in 60 days";
            if (serverNode.NoOfExpiringCert60days > 0)
                ExpiringCertificatesLabel.TextColor = NSColor.FromCGColor (new CoreGraphics.CGColor ((float)0.4, (float)0.3, (float)1, (float)1));
            ActiveCertificatesLabel.StringValue = serverNode.NoOfActiveCerts + " Certificates Active";
            ExpiredCertificatesLabel.StringValue = serverNode.NoOfExpiredCerts + " Certificates Expired";
            RevokedCertificatesLabel.StringValue = serverNode.NoOfRevokedCerts + " Certificates Revoked";
        }

        #region Events



        partial void ShowRootCertificate (Foundation.NSObject sender)
        {
            serverNode.ShowRootCertificate ();
        }


        partial void GetServerVersion (Foundation.NSObject sender)
        {
            serverNode.ShowServerVersion ();
        }

        #endregion

        //strongly typed view accessor
        public new ServerInfoPopOver View {
            get {
                return (ServerInfoPopOver)base.View;
            }
        }
    }
}

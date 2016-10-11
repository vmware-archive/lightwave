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
using Foundation;
using AppKit;
using CoreGraphics;
using System.Net.NetworkInformation;

namespace VmIdentity.UI.Common
{
    public partial class MainWindowCommonController : NSWindowController
    {
        public MainWindowCommonController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public MainWindowCommonController(NSCoder coder)
            : base(coder)
        {
        }

        public MainWindowCommonController(string windowName)
            : base(windowName)
        {
        }

        public MainWindowCommonController()
            : base("MainWindowCommon")
        {
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            NetworkChange.NetworkAvailabilityChanged += (object sender, NetworkAvailabilityEventArgs e) =>
            {
                NetworkStatus.StringValue = e.IsAvailable ? "Connected" : "Disconnected";
                NetworkStatus.TextColor = e.IsAvailable
					? NSColor.FromSrgb((nfloat)3.0 / 255, (nfloat)161 / 255, (nfloat)27 / 255, 1)
					: NSColor.Red;
            };
        }

        public virtual void InitialiseViews()
        {
        }

        public void SetSubView(NSView view)
        {
            this.ContainerView.AddSubview(view);
        }

        public void ReplaceSubview(NSView sourceView, NSView targetView)
        {
            this.ContainerView.ReplaceSubviewWith(sourceView, targetView);
            targetView.Frame = this.ContainerView.Bounds;
        }

        public new MainWindowCommon Window
        {
            get { return (MainWindowCommon)base.Window; }
        }
    }
}

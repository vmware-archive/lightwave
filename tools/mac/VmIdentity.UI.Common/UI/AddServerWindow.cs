/*
 * Copyright 2015 VMware, Inc.  All rights reserved.
 */

using System;
using Foundation;
using AppKit;

namespace VmIdentity.UI.Common
{
    public partial class AddServerWindow : AppKit.NSWindow
    {
        #region Constructors

        // Called when created from unmanaged code
        public AddServerWindow (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public AddServerWindow (NSCoder coder) : base (coder)
        {
        }

        [Export ("windowWillClose:")]
        public void WindowWillClose (NSNotification notification)
        {
            this.Close ();
            NSApplication.SharedApplication.StopModal ();
        }

        #endregion
    }
}


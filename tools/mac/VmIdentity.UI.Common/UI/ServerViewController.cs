/*
 * Copyright 2015 VMware, Inc.  All rights reserved.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using Foundation;
using AppKit;

namespace VmIdentity.UI.Common
{
    public partial class ServerViewController : AppKit.NSViewController
    {
        #region Constructors

        // Called when created from unmanaged code
        public ServerViewController (IntPtr handle) : base (handle)
        {
            Initialize ();
        }
		
        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public ServerViewController (NSCoder coder) : base (coder)
        {
            Initialize ();
        }
		
        // Call to load from the XIB/NIB file
        public ServerViewController () : base ("ServerView", NSBundle.MainBundle)
        {
            Initialize ();
        }
		
        // Shared initialization code
        void Initialize ()
        {
        }

        #endregion

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();
            this.ServerName.StringValue = "Test";

        }

        //strongly typed view accessor
        public new ServerView View {
            get {
                return (ServerView)base.View;
            }
        }
    }
}


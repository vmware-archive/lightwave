/*
 * Copyright 2015 VMware, Inc.  All rights reserved.
 */

using System;
using AppKit;
using Foundation;
using VmIdentity.UI.Common.Utilities;

namespace VmIdentity.UI.Common
{
    public partial class AddServerWindowController : AppKit.NSWindowController
    {
        #region Constructors

        // Called when created from unmanaged code
        public AddServerWindowController (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public AddServerWindowController (NSCoder coder) : base (coder)
        {
        }

        // Call to load from the XIB/NIB file
        public AddServerWindowController () : base ("AddServerWindow")
        {
        }


        #endregion

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();

            //Events
            this.OKButton.Activated += OnClickOKButton;
            this.CancelButton.Activated += OnClickCancelButton;
        }

        public void OnClickOKButton (object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty (IPTxtField.StringValue)) {
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (1);
            } else {
                UIErrorHelper.ShowAlert ("Please enter values in the specified field", "Alert");
            }
        }

        public void OnClickCancelButton (object sender, EventArgs e)
        {
            this.Close ();
            NSApplication.SharedApplication.StopModalWithCode (1);
        }

        //strongly typed window accessor
        public new AddServerWindow Window {
            get {
                return (AddServerWindow)base.Window;
            }
        }
    }
}


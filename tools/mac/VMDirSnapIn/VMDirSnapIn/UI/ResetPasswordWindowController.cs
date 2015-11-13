/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;

using Foundation;
using AppKit;
using VmIdentity.UI.Common.Utilities;

namespace VMDirSnapIn.UI
{
    public partial class ResetPasswordWindowController : NSWindowController
    {
        public string Password { get; set; }

        public ResetPasswordWindowController (IntPtr handle) : base (handle)
        {
        }

        [Export ("initWithCoder:")]
        public ResetPasswordWindowController (NSCoder coder) : base (coder)
        {
        }

        public ResetPasswordWindowController () : base ("ResetPasswordWindow")
        {
        }

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();
        }

        void ValidateControls ()
        {
            if (String.IsNullOrWhiteSpace (NewPasswordTextField.StringValue) || String.IsNullOrWhiteSpace (ConfirmPasswordTextField.StringValue)) {
                throw new Exception ("Please enter values in both the fields");
            }
            if (!this.NewPasswordTextField.StringValue.Equals (this.ConfirmPasswordTextField.StringValue)) {
                throw new Exception ("Passwords do not match");
            } else
                Password = NewPasswordTextField.StringValue;
            return;
        }

        partial void OnOKButton (Foundation.NSObject sender)
        {
            UIErrorHelper.CheckedExec (delegate() {
                ValidateControls ();
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (1);
            });
        }

        partial void OnCancelButton (Foundation.NSObject sender)
        {
            this.Close ();
            NSApplication.SharedApplication.StopModalWithCode (0);
        }

        public new ResetPasswordWindow Window {
            get { return (ResetPasswordWindow)base.Window; }
        }
    }
}

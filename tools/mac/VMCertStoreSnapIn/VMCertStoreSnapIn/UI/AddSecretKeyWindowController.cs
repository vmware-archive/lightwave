/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using Foundation;
using AppKit;
using VMCertStore.Common.DTO;

namespace VMCertStoreSnapIn
{
    public partial class AddSecretKeyWindowController :NSWindowController
    {
        SecretKeyDTO _secretKeyDTO = new SecretKeyDTO ();

        public SecretKeyDTO SecretKeyDTO { get { return _secretKeyDTO; } }

        #region Constructors

        // Called when created from unmanaged code
        public AddSecretKeyWindowController (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public AddSecretKeyWindowController (NSCoder coder) : base (coder)
        {
        }

        // Call to load from the XIB/NIB file
        public AddSecretKeyWindowController () : base ("AddSecretKeyWindow")
        {
        }

        #endregion

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();

            AddButton.Activated += OnAddSecretKey;
            CancelButton.Activated += (object sender, EventArgs e) => {
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (0);
            };
        }

        bool ValidateControls ()
        {
            try {
                if (string.IsNullOrEmpty (AliasTextField.StringValue))
                    throw new Exception ("Please enter an alias");
                if (string.IsNullOrEmpty (SecretKeyView.Value))
                    throw new Exception ("Please enter a secret key");
                return true;
            } catch (Exception exp) {
                UIErrorHelper.ShowAlert (exp.Message, "Alert");
            }
            return false;
        }

        public void OnAddSecretKey (object sender, EventArgs args)
        {
            if (!ValidateControls ())
                return;

            _secretKeyDTO.Alias = AliasTextField.StringValue;
            _secretKeyDTO.SecretKey = SecretKeyView.Value;
            _secretKeyDTO.Password = PasswordField.StringValue;

            this.Close ();
            NSApplication.SharedApplication.StopModalWithCode (1);
        }

        //strongly typed window accessor
        public new AddSecretKeyWindow Window {
            get {
                return (AddSecretKeyWindow)base.Window;
            }
        }
    }
}


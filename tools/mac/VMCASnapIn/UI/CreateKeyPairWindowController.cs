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
using AppKit;
using Foundation;
using VMCA.Client;
using VMCASnapIn.DTO;
using VMCASnapIn.Services;
using VmIdentity.UI.Common.Utilities;

namespace VMCASnapIn.UI
{
    public partial class CreateKeyPairWindowController : AppKit.NSWindowController
    {
        public KeyPairDTO DTO { get; protected set; }

        #region Constructors

        // Called when created from unmanaged code
        public CreateKeyPairWindowController (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public CreateKeyPairWindowController (NSCoder coder) : base (coder)
        {
        }

        // Call to load from the XIB/NIB file
        public CreateKeyPairWindowController () : base ("CreateKeyPairWindow")
        {
            DTO = new KeyPairDTO ();
        }

        #endregion

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();

            // Events Registration
            CreateKeyButton.Activated += OnCreateKey;
            CancelButton.Activated += (object sender, EventArgs e) => {
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (Constants.DIALOGCANCEL);
            };
        }

        public void OnCreateKey (object sender, EventArgs e)
        {
            var numKeyLength = KeyLengthOptions.SelectedValue;
            if (numKeyLength != null) {
                var keyPair = VMCAKeyPair.Create (Convert.ToUInt32 (numKeyLength.ToString ()));
                DTO.CreatedDateTime = DateTime.UtcNow;
                DTO.KeyLength = (int)Convert.ToUInt32 (numKeyLength.ToString ());
                DTO.PrivateKey = keyPair.PrivateKey;
                DTO.PublicKey = keyPair.PublicKey;

                //TODO -show Save Key options here
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (Constants.DIALOGOK);
            } else {
                UIErrorHelper.ShowAlert ("", "Please enter a value for Key Length");
            }
        }


        //strongly typed window accessor
        public new CreateKeyPairWindow Window {
            get {
                return (CreateKeyPairWindow)base.Window;
            }
        }
    }
}


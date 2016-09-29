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
using VmIdentity.UI.Common.Utilities;
using VMCertStore.Common.DTO;

namespace VMCertStoreSnapIn
{
    public partial class AddPrivateKeyWindowController : AppKit.NSWindowController
    {
        PrivateKeyDTO _privateKeyDTO = new PrivateKeyDTO ();

        public PrivateKeyDTO PrivateKeyDTO { get { return _privateKeyDTO; } }

        #region Constructors

        // Called when created from unmanaged code
        public AddPrivateKeyWindowController (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public AddPrivateKeyWindowController (NSCoder coder) : base (coder)
        {
        }

        // Call to load from the XIB/NIB file
        public AddPrivateKeyWindowController () : base ("AddPrivateKeyWindow")
        {
        }


        #endregion

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();

            OpenCertificateFileButton.Activated += (object sender, EventArgs e) => {
                NSOpenPanel openDlg = new NSOpenPanel ();
                openDlg.CanChooseFiles = true;
                nint result = openDlg.RunModal ();
                if (result == (nint)1) {
                    CertificateField.StringValue = openDlg.Url.RelativePath;
                } 
            };
            OpenPrivateKeyFileButton.Activated += (object sender, EventArgs e) => {
                NSOpenPanel openDlg = new NSOpenPanel ();
                openDlg.CanChooseFiles = true;
                nint result = openDlg.RunModal ();
                if (result == (nint)1) {
                    PrivateKeyField.StringValue = openDlg.Url.RelativePath;
                } 
            };
            AddButton.Activated += OnAddPrivateKey;
            CancelButton.Activated += (object sender, EventArgs e) => {
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (0);
            };
        }

        bool ValidateControls ()
        {
            try {
                if (string.IsNullOrEmpty (AliasField.StringValue))
                    throw new Exception ("Please enter an alias");
                if (string.IsNullOrEmpty (PrivateKeyField.StringValue))
                    throw new Exception ("Please select a private key file");
                return true;
            } catch (Exception exp) {
                UIErrorHelper.ShowAlert (exp.Message, "Alert");
            }
            return false;
        }

        public void OnAddPrivateKey (object sender, EventArgs eventargs)
        {
            if (!ValidateControls ())
                return;

            _privateKeyDTO.Alias = AliasField.StringValue;
            _privateKeyDTO.PrivateKey = PrivateKeyField.StringValue;
            _privateKeyDTO.Certificate = CertificateField.StringValue;
            _privateKeyDTO.Password = PasswordField.StringValue;

            this.Close ();
            NSApplication.SharedApplication.StopModalWithCode (1);
        }

        //strongly typed window accessor
        public new AddPrivateKeyWindow Window {
            get {
                return (AddPrivateKeyWindow)base.Window;
            }
        }
    }
}


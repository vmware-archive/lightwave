/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
using AppKit;
using Foundation;
using VMCA.Client;
using VMCASnapIn.DTO;
using VmIdentity.UI.Common;
using VMCASnapIn.Services;
using VmIdentity.UI.Common.Utilities;
using System.IO;
using VMCASnapIn.Nodes;

namespace VMCASnapIn.UI
{
    public partial class PrivateKeyEditorWindowController : AppKit.NSWindowController
    {
        public PrivateKeyDTO PrivateKeyDTO { get; protected set; }

        #region Constructors

        // Called when created from unmanaged code
        public PrivateKeyEditorWindowController (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public PrivateKeyEditorWindowController (NSCoder coder) : base (coder)
        {
        }

        // Call to load from the XIB/NIB file
        public PrivateKeyEditorWindowController (PrivateKeyDTO privateKey) : base ("PrivateKeyEditorWindow")
        {
            PrivateKeyDTO = privateKey;
        }

        #endregion

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();

            KeyLength.SelectItem (0);
            FilePath.Enabled = false;
            OpenFileButton.Enabled = false;

            //Events
            CreateKey.Activated += OnClickCreateKey;
            OpenFileButton.Activated += OpenKeyFile;
            OkButton.Activated += OnClickOkButton;



            CancelButton.Activated += (object sender, EventArgs e) => {
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (0);
            };
        }

        public void OnClickOkButton (object sender, EventArgs eventargs)
        {
            try {
                if (PrivateKeyOptions.SelectedTag == 2) {
                    if (string.IsNullOrEmpty (FilePath.StringValue))
                        throw new Exception ("One or more field empty");
                    PrivateKeyDTO.PrivateKeyFileName = FilePath.StringValue;
                } else {
                    if (string.IsNullOrEmpty (PrivateKeyTextView.Value))
                        throw  new Exception ("One or more field empty");
                    PrivateKeyDTO.PrivateKeyString = PrivateKeyTextView.Value;
                }
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (1);
            } catch (Exception e) {
                UIErrorHelper.ShowAlert ("", e.Message);
            }
        }

        public void OpenKeyFile (object sender, EventArgs eventargs)
        {
            NSOpenPanel openDlg = new NSOpenPanel ();
            openDlg.CanChooseFiles = true;
            openDlg.AllowedFileTypes = new string[]{ "pem", "key" };
            nint result = openDlg.RunModal ();
            if (result == (int)1) {
                FilePath.StringValue = openDlg.Url.RelativePath;
                UIErrorHelper.ShowAlert (openDlg.Filename, "File");
            }
        }

        partial void rowChanged (Foundation.NSObject sender)
        {
            switch (PrivateKeyOptions.SelectedTag) {
            case 1:
                CreateKey.Enabled = true;
                KeyLength.Enabled = true;
                FilePath.Enabled = false;
                OpenFileButton.Enabled = false;
                break;
            case 2:
                KeyLength.Enabled = false;
                CreateKey.Enabled = false;
                FilePath.Enabled = true;
                OpenFileButton.Enabled = true;
                PrivateKeyTextView.Hidden = true;
                break;
            case 3:
                PrivateKeyTextView.Hidden = false;
                KeyLength.Enabled = false;
                CreateKey.Enabled = false;
                FilePath.Enabled = false;
                OpenFileButton.Enabled = false;
                break;

            }
        }

        public void OnClickCreateKey (object sender, EventArgs eventargs)
        {
            if (KeyLength.SelectedValue != null) {
                var numKeyLength = KeyLength.SelectedValue.ToString ();
                var keyPair = VMCAKeyPair.Create (Convert.ToUInt32 (numKeyLength)); 
                PrivateKeyTextView.Value = keyPair.PrivateKey;
                ConfirmationDialogController dlg = new ConfirmationDialogController ("Do you want to save the Keys?");
                //dlg.ConfirmLabel.StringValue = ;
                int result = (int)NSApplication.SharedApplication.RunModalForWindow (dlg.Window);
                if (result == Constants.DIALOGOK)
                    VMCAKeyPairNode.SaveKeyData (keyPair);
            } else {
                UIErrorHelper.ShowAlert ("", "Please enter a value for Key Length");
            }
        }

        //strongly typed window accessor
        public new PrivateKeyEditorWindow Window {
            get {
                return (PrivateKeyEditorWindow)base.Window;
            }
        }
    }
}


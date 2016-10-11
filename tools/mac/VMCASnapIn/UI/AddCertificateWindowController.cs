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
using System.IO;
using AppKit;
using Foundation;
using VMCASnapIn.DTO;
using VMCASnapIn.Services;
using VmIdentity.UI.Common.Utilities;

namespace VMCASnapIn.UI
{
    public partial class AddCertificateWindowController : AppKit.NSWindowController
    {
        public AddCertificateDTO AddCertificateDto { get; set; }

        #region Constructors

        // Called when created from unmanaged code
        public AddCertificateWindowController (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public AddCertificateWindowController (NSCoder coder) : base (coder)
        {
        }

        // Call to load from the XIB/NIB file
        public AddCertificateWindowController (AddCertificateDTO dto) : base ("AddCertificateWindow")
        {
            AddCertificateDto = dto;
        }

        #endregion

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();

            //Register Events
            OpenFileButton.Activated += OpenCertFile;
            BrowseButton.Activated += CreatePrivateKey;
            AddButton.Activated += OnAddCertificate;
            CancelButton.Activated += OnCancelCertificate;
        }

        public void OpenCertFile (object sender, EventArgs eventargs)
        {
            NSOpenPanel openDlg = new NSOpenPanel ();
            openDlg.CanChooseFiles = true;
            //TODO - what are the cert file types to be shown?
            //openDlg.AllowedFileTypes = new string[] { "crt" };
            nint result = openDlg.RunModal ();
            if (result == (nint)1) {
                CertPathTxtField.StringValue = openDlg.Url.RelativePath;
                AddCertificateDto.Certificate = openDlg.Url.RelativePath;
            } 
        }

        public void CreatePrivateKey (object sender, EventArgs eventargs)
        {
            PrivateKeyEditorWindowController pwc = new PrivateKeyEditorWindowController (AddCertificateDto.PrivateKey);
            nint result = NSApplication.SharedApplication.RunModalForWindow (pwc.Window);
            if (result == (nint)Constants.DIALOGOK) {
                if (!string.IsNullOrEmpty (AddCertificateDto.PrivateKey.PrivateKeyFileName)) {
                    AddCertificateDto.PrivateKey.PrivateKeyString = File.ReadAllText (AddCertificateDto.PrivateKey.PrivateKeyFileName);
                }
                PrivateKeyTxtField.StringValue = AddCertificateDto.PrivateKey.PrivateKeyString;
            }
        }

        public void ValidateControls ()
        {
            if (string.IsNullOrEmpty (CertPathTxtField.StringValue) || string.IsNullOrEmpty (PrivateKeyTxtField.StringValue))
                throw new Exception ("Please enter values for the fields specified");
        }

        public void OnAddCertificate (object sender, EventArgs eventargs)
        {
            UIErrorHelper.CheckedExec (delegate() {
                ValidateControls ();
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (Constants.DIALOGOK);
            });
        }

        public void OnCancelCertificate (object sender, EventArgs eventargs)
        {
            this.Close ();
            NSApplication.SharedApplication.StopModalWithCode (Constants.DIALOGCANCEL);
        }

        //strongly typed window accessor
        public new AddCertificateWindow Window {
            get {
                return (AddCertificateWindow)base.Window;
            }
        }
    }
}


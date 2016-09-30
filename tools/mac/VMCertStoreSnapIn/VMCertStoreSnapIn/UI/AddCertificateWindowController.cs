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
    public partial class AddCertificateWindowController : AppKit.NSWindowController
    {
        AddCertificateDTO _certificateDTO = new AddCertificateDTO ();

        public AddCertificateDTO CertificateDTO { get { return _certificateDTO; } }

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
        public AddCertificateWindowController () : base ("AddCertificateWindow")
        {
        }

        #endregion

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();

            OpenFileButton.Activated += OpenCertificateFile;
            AddButton.Activated += OnAddCertificate;
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
                if (string.IsNullOrEmpty (CertificateField.StringValue))
                    throw new Exception ("Please select a certificate file");
                return true;
            } catch (Exception exp) {
                UIErrorHelper.ShowAlert (exp.Message, "Alert");
            }
            return false;
        }

        void OnAddCertificate (object sender, EventArgs args)
        {
            if (!ValidateControls ())
                return;

            _certificateDTO.Alias = AliasField.StringValue;
            _certificateDTO.Certificate = CertificateField.StringValue;
            //_certificateDTO.AutoRefresh = chkAutoRefresh.Checked;

            this.Close ();
            NSApplication.SharedApplication.StopModalWithCode (1);
        }

        void OpenCertificateFile (object sender, EventArgs args)
        {
            NSOpenPanel openDlg = new NSOpenPanel ();
            openDlg.CanChooseFiles = true;
            nint result = openDlg.RunModal ();
            if (result == (nint)1) {
                CertificateField.StringValue = openDlg.Url.RelativePath;
            } 
        }

        //strongly typed window accessor
        public new AddCertificateWindow Window {
            get {
                return (AddCertificateWindow)base.Window;
            }
        }
    }
}


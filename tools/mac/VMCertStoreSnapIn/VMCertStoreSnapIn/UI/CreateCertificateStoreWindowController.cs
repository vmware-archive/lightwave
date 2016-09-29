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
using VmIdentity.UI.Common.Utilities;
using AppKit;
using VMCertStore.Common.DTO;

namespace VMCertStoreSnapIn
{
    public partial class CreateCertificateStoreWindowController : AppKit.NSWindowController
    {
        CreateCertStoreDTO _DTO = new CreateCertStoreDTO ();

        public CreateCertStoreDTO CertStoreDTO { get { return _DTO; } }

        #region Constructors

        // Called when created from unmanaged code
        public CreateCertificateStoreWindowController (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public CreateCertificateStoreWindowController (NSCoder coder) : base (coder)
        {
        }

        // Call to load from the XIB/NIB file
        public CreateCertificateStoreWindowController () : base ("CreateCertificateStoreWindow")
        {
        }

        #endregion

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();

            CreateButton.Activated += OnCreateCertificate;
            CancelButton.Activated += (object sender, EventArgs e) => {
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (0);
            };
        }

        public void OnCreateCertificate (object sender, EventArgs args)
        {
            if (string.IsNullOrEmpty (StoreNameField.StringValue))
                UIErrorHelper.ShowAlert ("Please enter values for one or more fields specified", "Alert");
            else {
                _DTO.StoreName = StoreNameField.StringValue;
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (1);
            }
        }

        //strongly typed window accessor
        public new CreateCertificateStoreWindow Window {
            get {
                return (CreateCertificateStoreWindow)base.Window;
            }
        }
    }
}


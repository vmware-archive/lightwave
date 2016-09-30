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
        }

        bool ValidateControls ()
        {
            try {
				if (string.IsNullOrWhiteSpace (AliasTextField.StringValue))
                    throw new Exception ("Please enter an alias");
				if (string.IsNullOrWhiteSpace (SecretKeyTextField.StringValue))
                    throw new Exception ("Please enter a secret key");
                return true;
            } catch (Exception exp) {
                UIErrorHelper.ShowAlert (exp.Message, "Alert");
            }
            return false;
        }

		partial void OnBrowse(NSObject sender)
		{
			try
			{
				SecretKeyTextField.StringValue=FileIOUtil.ReadAllTextFromFile("Select Secret Key", new string[]{"key","pem" });
			}
			catch (Exception exp)
			{
				UIErrorHelper.ShowError(exp.Message);
			}
		}
		partial void OnAdd(NSObject sender)
		{
			if (!ValidateControls())
				return;

			_secretKeyDTO.Alias = AliasTextField.StringValue;
			_secretKeyDTO.SecretKey = SecretKeyTextField.StringValue;
			_secretKeyDTO.Password = PasswordField.StringValue;

			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(1);
		}
		partial void OnCancel(NSObject sender)
		{
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(0);
		}
        //strongly typed window accessor
        public new AddSecretKeyWindow Window {
            get {
                return (AddSecretKeyWindow)base.Window;
            }
        }
    }
}


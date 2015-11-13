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
    public class AddNewUserDTO
    {
        public string FirstName { get; set; }

        public string LastName { get; set; }

        public string Cn { get; set; }

        public string UPN { get; set; }

        public string SAMAccountName { get; set; }
    }

    public partial class AddNewUserController : NSWindowController
    {
        AddNewUserDTO _dto;

        public AddNewUserController (IntPtr handle) : base (handle)
        {
        }

        [Export ("initWithCoder:")]
        public AddNewUserController (NSCoder coder) : base (coder)
        {
        }

        public AddNewUserController (AddNewUserDTO dto) : base ("AddNewUser")
        {
            _dto = dto;
        }

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();
        }

        private void DoValidateControls ()
        {
            if (string.IsNullOrWhiteSpace (CNTextField.StringValue) || string.IsNullOrWhiteSpace (FirstNameTextField.StringValue) ||
                string.IsNullOrWhiteSpace (LastNameTextField.StringValue) || string.IsNullOrWhiteSpace (sAMAccountNameTextField.StringValue) ||
                string.IsNullOrWhiteSpace (UPNTextField.StringValue))
                throw new Exception ("Please enter values in all the fields.");
        }

        partial void OnCreateUser (Foundation.NSObject sender)
        {
            UIErrorHelper.CheckedExec (delegate () {
                DoValidateControls ();
                _dto.Cn = CNTextField.StringValue;
                _dto.FirstName = FirstNameTextField.StringValue;
                _dto.LastName = LastNameTextField.StringValue;
                _dto.SAMAccountName = sAMAccountNameTextField.StringValue;
                _dto.UPN = UPNTextField.StringValue;
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (1);
            });
        }

        partial void OnCancel (Foundation.NSObject sender)
        {
            this.Close ();
            NSApplication.SharedApplication.StopModalWithCode (0);
        }

        public new AddNewUser Window {
            get { return (AddNewUser)base.Window; }
        }
    }
}

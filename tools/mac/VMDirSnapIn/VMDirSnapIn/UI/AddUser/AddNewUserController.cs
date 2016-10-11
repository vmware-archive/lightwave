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
using VMDir.Common;
using VMDir.Common.DTO;

namespace VMDirSnapIn.UI
{
	public partial class AddNewUserController : NSWindowController
	{
		UserDTO _dto;

		public AddNewUserController(IntPtr handle) : base(handle)
		{
		}

		[Export("initWithCoder:")]
		public AddNewUserController(NSCoder coder) : base(coder)
		{
		}

		public AddNewUserController(UserDTO dto) : base("AddNewUser")
		{
			_dto = dto;
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
		}

		private bool DoValidateControls()
		{
			string msg = string.Empty;

			if (string.IsNullOrWhiteSpace(CNTextField.StringValue))
				msg = VMDirConstants.WRN_CN_ENT;
			else if (string.IsNullOrWhiteSpace(FirstNameTextField.StringValue))
				msg = VMDirConstants.WRN_FN_ENT;
			else if (string.IsNullOrWhiteSpace(LastNameTextField.StringValue))
				msg = VMDirConstants.WRN_LN_ENT;
			else if (string.IsNullOrWhiteSpace(sAMAccountNameTextField.StringValue))
				msg = VMDirConstants.WRN_SAM_NAME_ENT;
			else if (string.IsNullOrWhiteSpace(UPNTextField.StringValue))
				msg = VMDirConstants.WRN_UPN_ENT;

			if (!string.IsNullOrWhiteSpace(msg))
			{
				UIErrorHelper.ShowWarning(msg);
				return false;
			}
			return true;
		}

		partial void OnCreateUser(Foundation.NSObject sender)
		{
			UIErrorHelper.CheckedExec(delegate ()
			{
				if (!DoValidateControls())
					return;
				_dto.Cn = CNTextField.StringValue;
				_dto.FirstName = FirstNameTextField.StringValue;
				_dto.LastName = LastNameTextField.StringValue;
				_dto.SAMAccountName = sAMAccountNameTextField.StringValue;
				_dto.UPN = UPNTextField.StringValue;
				this.Close();
				NSApplication.SharedApplication.StopModalWithCode(1);
			});
		}

		partial void OnCancel(Foundation.NSObject sender)
		{
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(0);
		}

		public new AddNewUser Window
		{
			get { return (AddNewUser)base.Window; }
		}
	}
}

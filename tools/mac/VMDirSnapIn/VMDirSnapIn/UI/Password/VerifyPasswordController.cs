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

namespace UI.Password
{
	public partial class VerifyPasswordController : NSWindowController
	{
		public string Upn { get; set; }
		public string Password { get; set; }

		public VerifyPasswordController(IntPtr handle) : base(handle)
		{
		}

		[Export("initWithCoder:")]
		public VerifyPasswordController(NSCoder coder) : base(coder)
		{
		}

		public VerifyPasswordController(string upn) : base("VerifyPassword")
		{
			this.Upn = upn;
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			this.UpnTextField.StringValue = Upn;
		}

		private bool ValidateControls()
		{
			string msg = string.Empty;
			if (string.IsNullOrWhiteSpace(this.UpnTextField.StringValue))
				msg = VMDirConstants.WRN_UPN_ENT;
			else if (String.IsNullOrWhiteSpace(this.PwdTextField.StringValue))
			{
				msg = VMDirConstants.WRN_PWD_ENT;
			}

			if (!string.IsNullOrWhiteSpace(msg))
			{
				UIErrorHelper.ShowWarning(msg);
				return false;
			}

			return true;
		}

		partial void OnOkButton(Foundation.NSObject sender)
		{
			UIErrorHelper.CheckedExec(delegate ()
			{
				if (!ValidateControls())
					return;

				Password = PwdTextField.StringValue;
				Upn = UpnTextField.StringValue;
				this.Close();
				NSApplication.SharedApplication.StopModalWithCode(1);
			});
		}

		partial void OnCancelButton(Foundation.NSObject sender)
		{
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(0);
		}
		public new VerifyPassword Window
		{
			get { return (VerifyPassword)base.Window; }
		}
	}
}

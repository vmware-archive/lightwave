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

namespace VMDirSnapIn.UI
{
	public partial class ResetPasswordWindowController : NSWindowController
	{
		public string Password { get; set; }
		public string Dn { get; set; }

		public ResetPasswordWindowController(IntPtr handle) : base(handle)
		{
		}

		[Export("initWithCoder:")]
		public ResetPasswordWindowController(NSCoder coder) : base(coder)
		{
		}

		public ResetPasswordWindowController(string dn) : base("ResetPasswordWindow")
		{
			this.Dn = dn;
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			this.DnTextField.StringValue = Dn;
		}

		private bool ValidateControls()
		{
			var msg = string.Empty;
			if (string.IsNullOrWhiteSpace(this.DnTextField.StringValue))
				msg = VMDirConstants.WRN_PWD_ENT;
			else if (String.IsNullOrWhiteSpace(NewPasswordTextField.StringValue))
				msg = VMDirConstants.WRN_NEW_PWD_ENT;
			else if (!string.Equals(this.NewPasswordTextField.StringValue, this.ConfirmPasswordTextField.StringValue))
				msg = VMDirConstants.WRN_PWD_NO_MATCH;

			if (!string.IsNullOrWhiteSpace(msg))
			{
				UIErrorHelper.ShowWarning(msg);
				return false;
			}
			return true;
		}

		partial void OnOKButton(Foundation.NSObject sender)
		{
			UIErrorHelper.CheckedExec(delegate ()
			{
				if (!ValidateControls())
					return;

				Dn = DnTextField.StringValue;
				Password = NewPasswordTextField.StringValue;
				this.Close();
				NSApplication.SharedApplication.StopModalWithCode(1);
			});
		}

		partial void OnCancelButton(Foundation.NSObject sender)
		{
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(0);
		}

		public new ResetPasswordWindow Window
		{
			get { return (ResetPasswordWindow)base.Window; }
		}
	}
}

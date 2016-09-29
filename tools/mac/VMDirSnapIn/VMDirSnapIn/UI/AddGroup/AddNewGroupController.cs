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
using VMDir.Common;
using VmIdentity.UI.Common.Utilities;
using VMDir.Common.DTO;

namespace VMDirSnapIn.UI
{
	public partial class AddNewGroupController : NSWindowController
	{
		GroupDTO _dto;

		public AddNewGroupController(IntPtr handle) : base(handle)
		{
		}

		[Export("initWithCoder:")]
		public AddNewGroupController(NSCoder coder) : base(coder)
		{
		}

		public AddNewGroupController(GroupDTO dto) : base("AddNewGroup")
		{
			_dto = dto;
			_dto.groupType = VMDirConstants.GROUPTYPE_ACCOUNT;
			_dto.objectClass = "group";
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
		}

		partial void OnCancelButton(Foundation.NSObject sender)
		{
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(0);
		}

		private bool DoValidateControls()
		{
			string msg = string.Empty;

			if (String.IsNullOrWhiteSpace(GroupNameTextField.StringValue))
				msg = VMDirConstants.WRN_GRP_NAME_ENT;
			else if (String.IsNullOrWhiteSpace(SAMAccountNameTextField.StringValue))
				msg = VMDirConstants.WRN_SAM_NAME_ENT;

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
				if (!DoValidateControls())
					return;
				_dto.cn = GroupNameTextField.StringValue;
				_dto.sAMAccountName = SAMAccountNameTextField.StringValue;
				this.Close();
				NSApplication.SharedApplication.StopModalWithCode(1);
			});
		}

		public new AddNewGroup Window
		{
			get { return (AddNewGroup)base.Window; }
		}
	}
}

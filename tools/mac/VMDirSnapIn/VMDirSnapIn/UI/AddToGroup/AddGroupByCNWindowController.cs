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
using VMDir.Common.VMDirUtilities;
using VMDir.Common.DTO;
using VMDir.Common;

namespace VMDirSnapIn.UI
{
	public partial class AddGroupByCNWindowController : NSWindowController
	{
		public String DNText { get; set; }

		VMDirServerDTO serverDTO;

		public AddGroupByCNWindowController(IntPtr handle) : base(handle)
		{
		}

		[Export("initWithCoder:")]
		public AddGroupByCNWindowController(NSCoder coder) : base(coder)
		{
		}

		public AddGroupByCNWindowController(VMDirServerDTO dto) : base("AddGroupByCNWindow")
		{
			serverDTO = dto;
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

		partial void OnOKButton(Foundation.NSObject sender)
		{
			if (!string.IsNullOrWhiteSpace(DnLabel.StringValue))
			{
				DNText = DnLabel.StringValue;
				this.Close();
				NSApplication.SharedApplication.StopModalWithCode(1);
			}
			else
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_GRP_NAME_SEL);
		}

		partial void OnFindDnButton(Foundation.NSObject sender)
		{
			UIErrorHelper.CheckedExec(delegate ()
			{
				if (!string.IsNullOrWhiteSpace(DNTextField.StringValue))
				{
					DNText = DNTextField.StringValue;
					string[] dn = Utilities.SearchItemCN(serverDTO.BaseDN, "group", DNText, null, serverDTO);
					//if only single result is found
					if (dn.Length == 1)
						DnLabel.StringValue = dn[0];
					else if (dn.Length <= 0)
						DnLabel.StringValue = "Search item not found in groups.";
					//else if dn.length>1 TODO - Display a separate window  listing all the multiple dn found  and let the user choose one.
				}
			});
		}

		public new AddGroupByCNWindow Window
		{
			get { return (AddGroupByCNWindow)base.Window; }
		}
	}
}

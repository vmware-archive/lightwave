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
	public partial class PageSizeController : NSWindowController
	{
		public int PageSize;
		public PageSizeController(IntPtr handle) : base(handle)
		{
		}

		[Export("initWithCoder:")]
		public PageSizeController(NSCoder coder) : base(coder)
		{
		}

		public PageSizeController(int pageSize) : base("PageSize")
		{
			PageSize = pageSize;
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			PageSizeTextField.StringValue = PageSize.ToString();
		}

		public new PageSize Window
		{
			get { return (PageSize)base.Window; }
		}

		partial void OnCancelButton(NSObject sender)
		{
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(0);
		}

		private bool Validate()
		{
			int ps;
			if (string.IsNullOrWhiteSpace(PageSizeTextField.StringValue))
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_PAGE_SIZE);
				return false;
			}
			if (int.TryParse(PageSizeTextField.StringValue, out ps))
			{
				if (ps <= 0)
				{
					UIErrorHelper.ShowWarning(VMDirConstants.WRN_PAGE_SIZE_MINVAL);
					return false;
				}
				else if (ps>VMDirConstants.DEFAULT_PAGE_SIZE*10)
				{
					UIErrorHelper.ShowWarning(VMDirConstants.WRN_PAGE_SIZE_MAXVAL);
					return false;
				}
				else
					return true;
			}
			else {
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_INT_VAL);
				return false;
			}
		}

		partial void OnSubmitButton(NSObject sender)
		{
			if (!Validate())
				return;
			int.TryParse(PageSizeTextField.StringValue, out PageSize);
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(1);
		}
	}
}

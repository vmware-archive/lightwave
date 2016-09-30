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
using VMDir.Common.DTO;
using System.Text;

namespace VMDirSnapIn.UI
{
	public partial class SyntaxHelpWindowController : NSWindowController
	{
		private AttributeHelpDTO helpDTO;

		public SyntaxHelpWindowController(IntPtr handle) : base(handle)
		{
		}

		[Export("initWithCoder:")]
		public SyntaxHelpWindowController(NSCoder coder) : base(coder)
		{
		}

		public SyntaxHelpWindowController(AttributeHelpDTO helpDTO) : base("SyntaxHelpWindow")
		{
			this.helpDTO = helpDTO;
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();

			NameTextField.StringValue = string.Empty;
			SyntaxTextField.StringValue = string.Empty;
			ExampleTextView.Value = string.Empty;
			MoreInfoTextField.StringValue = string.Empty;

			if (helpDTO != null)
			{
				NameTextField.StringValue = helpDTO.Name;
				SyntaxTextField.StringValue = helpDTO.Value;

				StringBuilder sb = new StringBuilder();
				if (helpDTO.ExampleList != null)
				{
					foreach (var str in helpDTO.ExampleList)
						sb.Append(str + Environment.NewLine);
					ExampleTextView.Value = sb.ToString();
				}

				if (!string.IsNullOrWhiteSpace(helpDTO.HelpLink))
				{
					MoreInfoTextField.StringValue = helpDTO.HelpLink;
				}
			}
		}

		public new SyntaxHelpWindow Window
		{
			get { return (SyntaxHelpWindow)base.Window; }
		}
		partial void OnCancel(NSObject sender)
		{
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(0);
		}
	}
}

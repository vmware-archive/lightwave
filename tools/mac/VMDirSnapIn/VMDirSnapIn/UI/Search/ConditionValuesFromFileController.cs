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
using System.Collections.Generic;
using VMDirSnapIn.DataSource;
using VMDir.Common;
using VmIdentity.UI.Common.Utilities;
using VMDir.Common.DTO;

namespace VMDirSnapIn.UI
{
	public partial class ConditionValuesFromFileController : NSWindowController
	{
		private List<string> _attrList;
		public List<string> ValuesList;
		public string Attribute { get; set; }
		public Condition Condition { get; set; }


		public ConditionValuesFromFileController(IntPtr handle) : base(handle)
		{
		}

		[Export("initWithCoder:")]
		public ConditionValuesFromFileController(NSCoder coder) : base(coder)
		{
		}

		public ConditionValuesFromFileController(List<string> attrList) : base("ConditionValuesFromFile")
		{
			_attrList = attrList;
			ValuesList = new List<string>();
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			foreach (var item in _attrList)
				this.AttributeComboBox.Add(new NSString(item));
			foreach (var item in VMDirConstants.ConditionList)
				this.ConditionComboBox.Add(new NSString(item));
		}

		private bool ValidateApply()
		{
			if (AttributeComboBox.SelectedValue == null)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_ATTR);
				return false;
			}
			if (ConditionComboBox.SelectedValue == null)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_COND);
				return false;
			}
			if (string.IsNullOrWhiteSpace(ValuesTextField.StringValue))
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_VAL);
				return false;
			}
			return true;
		}

		public new ConditionValuesFromFile Window
		{
			get { return (ConditionValuesFromFile)base.Window; }
		}
		partial void OnApply(NSObject sender)
		{
			if (!ValidateApply())
				return;
			Attribute = this.AttributeComboBox.SelectedValue.ToString();
			Condition = (Condition)(int)ConditionComboBox.SelectedIndex;
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(1);
		}
		partial void OnBrowse(NSObject sender)
		{
			UIErrorHelper.CheckedExec(delegate
			{
				var values = FileIOUtil.ReadAllTextFromFile("Load Values From File", new string[] { "txt","csv" });
				ValuesTextField.StringValue = values;
				var charArr = new char[] { '\r', '\n' };
				foreach (var item in ValuesTextField.StringValue.Split('\n'))
				{
					var val = item.Trim(charArr);
					if(!string.IsNullOrWhiteSpace(val))
						ValuesList.Add(val);
				}
			});
		}
		partial void OnCancel(NSObject sender)
		{
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(0);
		}

		[Export("windowWillClose:")]
		public void WindowWillClose(NSNotification notification)
		{
			NSApplication.SharedApplication.StopModalWithCode(0);
		}
	}
}

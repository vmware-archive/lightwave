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
using System.Linq;

namespace VMDirSnapIn.UI
{
	public partial class ModSubmitConfirmController : NSWindowController
	{
		private Dictionary<string, List<string>> _modifications;
		private AttributeTableViewDataSource _modAttrDs;
		public ModSubmitConfirmController(IntPtr handle) : base(handle)
		{
		}

		[Export("initWithCoder:")]
		public ModSubmitConfirmController(NSCoder coder) : base(coder)
		{
		}

		public ModSubmitConfirmController(Dictionary<string, List<string>> modifications) : base("ModSubmitConfirm")
		{
			_modifications = modifications;
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			_modAttrDs = new AttributeTableViewDataSource();
			_modAttrDs.attrList.AddRange(_modifications.Select(x=>x.Key));
			this.ModifiedAttributeTableView.DataSource = _modAttrDs;
		}

		public new ModSubmitConfirm Window
		{
			get { return (ModSubmitConfirm)base.Window; }
		}
		partial void OnYes(NSObject sender)
		{
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(1);
		}
		partial void OnNo(NSObject sender)
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

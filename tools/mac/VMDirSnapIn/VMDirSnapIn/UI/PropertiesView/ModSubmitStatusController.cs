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
using VMDir.Common.DTO;
using VMDirSnapIn.DataSource;
using VMDirSnapIn.Delegate;

namespace VMDirSnapIn.UI
{
	public partial class ModSubmitStatusController : NSWindowController
	{
		private List<AttributeModStatus> _modStatus;
		private ModificationStatusTableViewDataSource _modStatusDs;
		public ModSubmitStatusController(IntPtr handle) : base(handle)
		{
		}

		[Export("initWithCoder:")]
		public ModSubmitStatusController(NSCoder coder) : base(coder)
		{
		}

		public ModSubmitStatusController(List<AttributeModStatus> modStatus) : base("ModSubmitStatus")
		{
			_modStatus = modStatus;
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			_modStatusDs = new ModificationStatusTableViewDataSource(_modStatus);
			ModificationStatusTableView.DataSource = _modStatusDs;
			ModificationStatusTableView.Delegate = new ModificationStatusTableDelegate(_modStatusDs);
		}

		public new ModSubmitStatus Window
		{
			get { return (ModSubmitStatus)base.Window; }
		}

		partial void OnOk(NSObject sender)
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

/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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

namespace LWRaftSnapIn.UI
{
	public partial class MainWindow : AppKit.NSWindow
	{
		#region Constructors

		// Called when created from unmanaged code
		public MainWindow(IntPtr handle) : base(handle)
		{
		}

		// Called when created directly from a XIB file
		[Export("initWithCoder:")]
		public MainWindow(NSCoder coder) : base(coder)
		{
		}

		#endregion

		[Export("windowWillClose:")]
		public void WindowWillClose(NSNotification notification)
		{
            LWRaftSnapInEnvironment.Instance.SaveLocalData();
			Cleanup();
			NSApplication.SharedApplication.Terminate(this);
		}

		public void Cleanup()
		{
			NSNotificationCenter.DefaultCenter.PostNotificationName("CloseApplication", this);
		}
	}
}


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

// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace LWRaftSnapIn.UI
{
	[Register("AddNewGroupController")]
	partial class AddNewGroupController
	{
		[Outlet]
		AppKit.NSTextField GroupNameTextField { get; set; }

		[Outlet]
		AppKit.NSTextField SAMAccountNameTextField { get; set; }

		[Action("OnCancelButton:")]
		partial void OnCancelButton(Foundation.NSObject sender);

		[Action("OnOKButton:")]
		partial void OnOKButton(Foundation.NSObject sender);

		void ReleaseDesignerOutlets()
		{
			if (GroupNameTextField != null)
			{
				GroupNameTextField.Dispose();
				GroupNameTextField = null;
			}

			if (SAMAccountNameTextField != null)
			{
				SAMAccountNameTextField.Dispose();
				SAMAccountNameTextField = null;
			}
		}
	}
}

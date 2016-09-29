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

// WARNING
//
// This file has been generated automatically by Xamarin Studio Business to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace RestSsoAdminSnapIn
{
	[Register ("AddUserController")]
	partial class AddUserController
	{
		[Outlet]
		AppKit.NSButton BtnAdd { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton chkDisabled { get; set; }

		[Outlet]
		AppKit.NSSecureTextField TxtConfirmPassword { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDescription { get; set; }

		[Outlet]
		AppKit.NSTextField TxtEmail { get; set; }

		[Outlet]
		AppKit.NSTextField TxtFirstName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtLastName { get; set; }

		[Outlet]
		AppKit.NSSecureTextField TxtPassword { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUserName { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnAdd != null) {
				BtnAdd.Dispose ();
				BtnAdd = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (TxtConfirmPassword != null) {
				TxtConfirmPassword.Dispose ();
				TxtConfirmPassword = null;
			}

			if (TxtDescription != null) {
				TxtDescription.Dispose ();
				TxtDescription = null;
			}

			if (TxtEmail != null) {
				TxtEmail.Dispose ();
				TxtEmail = null;
			}

			if (TxtFirstName != null) {
				TxtFirstName.Dispose ();
				TxtFirstName = null;
			}

			if (TxtLastName != null) {
				TxtLastName.Dispose ();
				TxtLastName = null;
			}

			if (TxtPassword != null) {
				TxtPassword.Dispose ();
				TxtPassword = null;
			}

			if (TxtUserName != null) {
				TxtUserName.Dispose ();
				TxtUserName = null;
			}

			if (chkDisabled != null) {
				chkDisabled.Dispose ();
				chkDisabled = null;
			}
		}
	}

	[Register ("AddUser")]
	partial class AddUser
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

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
	[Register ("UserDetailsViewController")]
	partial class UserDetailsViewController
	{
		[Outlet]
		AppKit.NSButton BtnAddGroup { get; set; }

		[Outlet]
		AppKit.NSButton BtnApply { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveGroup { get; set; }

		[Outlet]
		AppKit.NSButton ChActAsUser { get; set; }

		[Outlet]
		AppKit.NSButton ChIdpAdmin { get; set; }

		[Outlet]
		AppKit.NSButton chkActive { get; set; }

		[Outlet]
		AppKit.NSButton LoginAsUser { get; set; }

		[Outlet]
		AppKit.NSTableView MemberTableView { get; set; }

		[Outlet]
		AppKit.NSMatrix RdoRoleGroup { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDescription { get; set; }

		[Outlet]
		AppKit.NSTextField TxtEmail { get; set; }

		[Outlet]
		AppKit.NSTextField TxtFirstName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtLastName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUsername { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnAddGroup != null) {
				BtnAddGroup.Dispose ();
				BtnAddGroup = null;
			}

			if (BtnApply != null) {
				BtnApply.Dispose ();
				BtnApply = null;
			}

			if (BtnRemoveGroup != null) {
				BtnRemoveGroup.Dispose ();
				BtnRemoveGroup = null;
			}

			if (ChActAsUser != null) {
				ChActAsUser.Dispose ();
				ChActAsUser = null;
			}

			if (ChIdpAdmin != null) {
				ChIdpAdmin.Dispose ();
				ChIdpAdmin = null;
			}

			if (chkActive != null) {
				chkActive.Dispose ();
				chkActive = null;
			}

			if (MemberTableView != null) {
				MemberTableView.Dispose ();
				MemberTableView = null;
			}

			if (RdoRoleGroup != null) {
				RdoRoleGroup.Dispose ();
				RdoRoleGroup = null;
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

			if (TxtUsername != null) {
				TxtUsername.Dispose ();
				TxtUsername = null;
			}

			if (LoginAsUser != null) {
				LoginAsUser.Dispose ();
				LoginAsUser = null;
			}
		}
	}
}

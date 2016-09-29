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
 
using Foundation;
using System.CodeDom.Compiler;

namespace RestSsoAdminSnapIn
{
	[Register ("ViewUserDetailsController")]
	partial class ViewUserDetailsController
	{
		[Outlet]
		AppKit.NSButton BtnAddGroup { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveGroup { get; set; }

		[Outlet]
		AppKit.NSButton BtnSave { get; set; }

		[Outlet]
		AppKit.NSButton ChActAsUser { get; set; }

		[Outlet]
		AppKit.NSButton ChDisabled { get; set; }

		[Outlet]
		AppKit.NSButton ChIdpAdmin { get; set; }

		[Outlet]
		AppKit.NSScrollView LstGroupMembershipView { get; set; }

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
		AppKit.NSTextField TxtPasswordDaysUntilExpiry { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordLastChanged { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUsername { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnAddGroup != null) {
				BtnAddGroup.Dispose ();
				BtnAddGroup = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnRemoveGroup != null) {
				BtnRemoveGroup.Dispose ();
				BtnRemoveGroup = null;
			}

			if (BtnSave != null) {
				BtnSave.Dispose ();
				BtnSave = null;
			}

			if (ChActAsUser != null) {
				ChActAsUser.Dispose ();
				ChActAsUser = null;
			}

			if (ChDisabled != null) {
				ChDisabled.Dispose ();
				ChDisabled = null;
			}

			if (ChIdpAdmin != null) {
				ChIdpAdmin.Dispose ();
				ChIdpAdmin = null;
			}

			if (LstGroupMembershipView != null) {
				LstGroupMembershipView.Dispose ();
				LstGroupMembershipView = null;
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

			if (TxtPasswordDaysUntilExpiry != null) {
				TxtPasswordDaysUntilExpiry.Dispose ();
				TxtPasswordDaysUntilExpiry = null;
			}

			if (TxtPasswordLastChanged != null) {
				TxtPasswordLastChanged.Dispose ();
				TxtPasswordLastChanged = null;
			}

			if (TxtUsername != null) {
				TxtUsername.Dispose ();
				TxtUsername = null;
			}
		}
	}

	[Register ("ViewUserDetails")]
	partial class ViewUserDetails
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

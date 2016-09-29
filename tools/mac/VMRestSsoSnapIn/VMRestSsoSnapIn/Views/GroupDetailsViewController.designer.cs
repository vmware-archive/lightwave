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
	[Register ("GroupDetailsViewController")]
	partial class GroupDetailsViewController
	{
		[Outlet]
		AppKit.NSButton BtnAddMember { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveMember { get; set; }

		[Outlet]
		AppKit.NSButton BtnSave { get; set; }

		[Outlet]
		AppKit.NSTableView GroupMembersTableView { get; set; }

		[Outlet]
		AppKit.NSTextField TxtGroupDescription { get; set; }

		[Outlet]
		AppKit.NSTextField TxtGroupName { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnAddMember != null) {
				BtnAddMember.Dispose ();
				BtnAddMember = null;
			}

			if (BtnRemoveMember != null) {
				BtnRemoveMember.Dispose ();
				BtnRemoveMember = null;
			}

			if (BtnSave != null) {
				BtnSave.Dispose ();
				BtnSave = null;
			}

			if (GroupMembersTableView != null) {
				GroupMembersTableView.Dispose ();
				GroupMembersTableView = null;
			}

			if (TxtGroupDescription != null) {
				TxtGroupDescription.Dispose ();
				TxtGroupDescription = null;
			}

			if (TxtGroupName != null) {
				TxtGroupName.Dispose ();
				TxtGroupName = null;
			}
		}
	}
}

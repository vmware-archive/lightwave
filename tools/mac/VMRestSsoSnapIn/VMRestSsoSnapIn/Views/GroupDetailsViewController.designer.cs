// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
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

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
	[Register ("ShowAllGroupsController")]
	partial class ShowAllGroupsController
	{
		[Outlet]
		AppKit.NSButton BtnAdd { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSComboBox DomainComboBox { get; set; }

		[Outlet]
		AppKit.NSTextField DomainLabel { get; set; }

		[Outlet]
		AppKit.NSTableView GroupsTableView { get; set; }

		[Outlet]
		AppKit.NSScrollView MainTableView { get; set; }

		[Outlet]
		AppKit.NSComboBox MemberTypeComboBox { get; set; }

		[Outlet]
		AppKit.NSTextField MemberTypeLabel { get; set; }

		[Outlet]
		AppKit.NSTextField NameTextString { get; set; }

		[Outlet]
		AppKit.NSButton SearchButton { get; set; }

		[Outlet]
		AppKit.NSTextField WarningLabel { get; set; }
		
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

			if (DomainComboBox != null) {
				DomainComboBox.Dispose ();
				DomainComboBox = null;
			}

			if (GroupsTableView != null) {
				GroupsTableView.Dispose ();
				GroupsTableView = null;
			}

			if (MainTableView != null) {
				MainTableView.Dispose ();
				MainTableView = null;
			}

			if (MemberTypeComboBox != null) {
				MemberTypeComboBox.Dispose ();
				MemberTypeComboBox = null;
			}

			if (NameTextString != null) {
				NameTextString.Dispose ();
				NameTextString = null;
			}

			if (SearchButton != null) {
				SearchButton.Dispose ();
				SearchButton = null;
			}

			if (WarningLabel != null) {
				WarningLabel.Dispose ();
				WarningLabel = null;
			}

			if (MemberTypeLabel != null) {
				MemberTypeLabel.Dispose ();
				MemberTypeLabel = null;
			}

			if (DomainLabel != null) {
				DomainLabel.Dispose ();
				DomainLabel = null;
			}
		}
	}

	[Register ("ShowAllGroups")]
	partial class ShowAllGroups
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

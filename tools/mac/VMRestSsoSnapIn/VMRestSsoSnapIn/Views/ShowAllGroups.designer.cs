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
	[Register ("ShowAllGroupsController")]
	partial class ShowAllGroupsController
	{
		[Outlet]
		AppKit.NSButton BtnAdd { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSTableView GroupsTableView { get; set; }

		[Outlet]
		AppKit.NSScrollView MainTableView { get; set; }
		
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

			if (MainTableView != null) {
				MainTableView.Dispose ();
				MainTableView = null;
			}

			if (GroupsTableView != null) {
				GroupsTableView.Dispose ();
				GroupsTableView = null;
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

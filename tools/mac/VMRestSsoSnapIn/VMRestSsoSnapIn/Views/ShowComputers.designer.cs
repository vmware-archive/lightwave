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
	[Register ("ShowComputersController")]
	partial class ShowComputersController
	{
		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSScrollView MainTableView { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (MainTableView != null) {
				MainTableView.Dispose ();
				MainTableView = null;
			}
		}
	}

	[Register ("ShowComputers")]
	partial class ShowComputers
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

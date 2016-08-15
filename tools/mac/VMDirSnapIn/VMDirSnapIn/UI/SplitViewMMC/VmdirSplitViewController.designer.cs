// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMDirSnapIn.UI
{
	[Register ("VmdirSplitViewController")]
	partial class VmdirSplitViewController
	{
		[Outlet]
		public AppKit.NSOutlineView VmdirOutlineView { get; set; }

		[Outlet]
		public AppKit.NSView VmdirPropView { get; private set; }

		[Action ("OnClickAction:")]
		partial void OnClickAction (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (VmdirOutlineView != null) {
				VmdirOutlineView.Dispose ();
				VmdirOutlineView = null;
			}

			if (VmdirPropView != null) {
				VmdirPropView.Dispose ();
				VmdirPropView = null;
			}
		}
	}
}

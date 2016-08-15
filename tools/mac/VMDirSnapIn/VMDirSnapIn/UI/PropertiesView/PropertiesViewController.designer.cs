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
	[Register ("PropertiesViewController")]
	partial class PropertiesViewController
	{
		[Outlet]
		AppKit.NSButton AttrAdd { get; set; }

		[Outlet]
		public AppKit.NSButton PropApply { get; set; }

		[Outlet]
		public AppKit.NSButton PropReset { get; set; }

		[Outlet]
		public AppKit.NSTableView PropTableView { get; set; }

		[Action ("AttrAddClick:")]
		partial void AttrAddClick (Foundation.NSObject sender);

		[Action ("PropApplyClick:")]
		partial void PropApplyClick (Foundation.NSObject sender);

		[Action ("PropResetClick:")]
		partial void PropResetClick (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (AttrAdd != null) {
				AttrAdd.Dispose ();
				AttrAdd = null;
			}

			if (PropApply != null) {
				PropApply.Dispose ();
				PropApply = null;
			}

			if (PropReset != null) {
				PropReset.Dispose ();
				PropReset = null;
			}

			if (PropTableView != null) {
				PropTableView.Dispose ();
				PropTableView = null;
			}
		}
	}
}

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
	[Register ("PageSizeController")]
	partial class PageSizeController
	{
		[Outlet]
		AppKit.NSTextField PageSizeTextField { get; set; }

		[Action ("OnCancelButton:")]
		partial void OnCancelButton (Foundation.NSObject sender);

		[Action ("OnSubmitButton:")]
		partial void OnSubmitButton (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (PageSizeTextField != null) {
				PageSizeTextField.Dispose ();
				PageSizeTextField = null;
			}
		}
	}
}

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
	[Register ("ModSubmitConfirmController")]
	partial class ModSubmitConfirmController
	{
		[Outlet]
		AppKit.NSTableView ModifiedAttributeTableView { get; set; }

		[Action ("OnNo:")]
		partial void OnNo (Foundation.NSObject sender);

		[Action ("OnYes:")]
		partial void OnYes (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (ModifiedAttributeTableView != null) {
				ModifiedAttributeTableView.Dispose ();
				ModifiedAttributeTableView = null;
			}
		}
	}
}

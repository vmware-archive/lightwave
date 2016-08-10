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
	[Register ("ModSubmitStatusController")]
	partial class ModSubmitStatusController
	{
		[Outlet]
		AppKit.NSTableView ModificationStatusTableView { get; set; }

		[Action ("OnOk:")]
		partial void OnOk (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (ModificationStatusTableView != null) {
				ModificationStatusTableView.Dispose ();
				ModificationStatusTableView = null;
			}
		}
	}
}

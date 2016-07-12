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
	[Register ("SyntaxHelpWindowController")]
	partial class SyntaxHelpWindowController
	{
		[Outlet]
		AppKit.NSTextView ExampleTextView { get; set; }

		[Outlet]
		AppKit.NSTextField MoreInfoTextField { get; set; }

		[Outlet]
		AppKit.NSTextField NameTextField { get; set; }

		[Outlet]
		AppKit.NSTextField SyntaxTextField { get; set; }

		[Action ("OnCancel:")]
		partial void OnCancel (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (ExampleTextView != null) {
				ExampleTextView.Dispose ();
				ExampleTextView = null;
			}

			if (NameTextField != null) {
				NameTextField.Dispose ();
				NameTextField = null;
			}

			if (SyntaxTextField != null) {
				SyntaxTextField.Dispose ();
				SyntaxTextField = null;
			}

			if (MoreInfoTextField != null) {
				MoreInfoTextField.Dispose ();
				MoreInfoTextField = null;
			}
		}
	}
}

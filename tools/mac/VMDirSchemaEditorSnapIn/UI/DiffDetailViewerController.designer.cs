// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMDirSchemaEditorSnapIn
{
	[Register ("DiffDetailViewerController")]
	partial class DiffDetailViewerController
	{
		[Outlet]
		AppKit.NSTextField BaseLabel { get; set; }

		[Outlet]
		AppKit.NSTextView BaseTextView { get; set; }

		[Outlet]
		AppKit.NSTextField CurrentLabel { get; set; }

		[Outlet]
		AppKit.NSTextView CurrentTextView { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BaseTextView != null) {
				BaseTextView.Dispose ();
				BaseTextView = null;
			}

			if (CurrentTextView != null) {
				CurrentTextView.Dispose ();
				CurrentTextView = null;
			}

			if (BaseLabel != null) {
				BaseLabel.Dispose ();
				BaseLabel = null;
			}

			if (CurrentLabel != null) {
				CurrentLabel.Dispose ();
				CurrentLabel = null;
			}
		}
	}
}

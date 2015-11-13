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
	[Register ("AddNewSignatureAlgorithmController")]
	partial class AddNewSignatureAlgorithmController
	{
		[Outlet]
		AppKit.NSButton BtnAdd { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSTextField TxtMaxKeySize { get; set; }

		[Outlet]
		AppKit.NSTextField TxtMinKeySize { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPriority { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (TxtMaxKeySize != null) {
				TxtMaxKeySize.Dispose ();
				TxtMaxKeySize = null;
			}

			if (TxtMinKeySize != null) {
				TxtMinKeySize.Dispose ();
				TxtMinKeySize = null;
			}

			if (TxtPriority != null) {
				TxtPriority.Dispose ();
				TxtPriority = null;
			}

			if (BtnAdd != null) {
				BtnAdd.Dispose ();
				BtnAdd = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}
		}
	}
}

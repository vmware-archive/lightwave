// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMCASnapIn.UI
{
	[Register ("CreateKeyPairWindowController")]
	partial class CreateKeyPairWindowController
	{
		[Outlet]
		AppKit.NSButton CancelButton { get; set; }

		[Outlet]
		AppKit.NSButton CreateKeyButton { get; set; }

		[Outlet]
		AppKit.NSComboBox KeyLengthOptions { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (CreateKeyButton != null) {
				CreateKeyButton.Dispose ();
				CreateKeyButton = null;
			}

			if (KeyLengthOptions != null) {
				KeyLengthOptions.Dispose ();
				KeyLengthOptions = null;
			}

			if (CancelButton != null) {
				CancelButton.Dispose ();
				CancelButton = null;
			}
		}
	}

	[Register ("CreateKeyPairWindow")]
	partial class CreateKeyPairWindow
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

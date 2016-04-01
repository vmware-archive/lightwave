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
	[Register ("SuperLoggingFilterController")]
	partial class SuperLoggingFilterController
	{
		[Outlet]
		AppKit.NSButton AddButton { get; set; }

		[Outlet]
		AppKit.NSButton CancelButton { get; set; }

		[Outlet]
		AppKit.NSButton CloseButton { get; set; }

		[Outlet]
		AppKit.NSComboBox ColumnComboBox { get; set; }

		[Outlet]
		AppKit.NSTableView FilterTableView { get; set; }

		[Outlet]
		AppKit.NSComboBox OperatorComboBox { get; set; }

		[Outlet]
		AppKit.NSButton RemoveButton { get; set; }

		[Outlet]
		AppKit.NSButton UpdateButton { get; set; }

		[Outlet]
		AppKit.NSTextField ValueTextField { get; set; }

		void ReleaseDesignerOutlets ()
		{
			if (AddButton != null) {
				AddButton.Dispose ();
				AddButton = null;
			}

			if (CancelButton != null) {
				CancelButton.Dispose ();
				CancelButton = null;
			}

			if (CloseButton != null) {
				CloseButton.Dispose ();
				CloseButton = null;
			}

			if (ColumnComboBox != null) {
				ColumnComboBox.Dispose ();
				ColumnComboBox = null;
			}

			if (FilterTableView != null) {
				FilterTableView.Dispose ();
				FilterTableView = null;
			}

			if (OperatorComboBox != null) {
				OperatorComboBox.Dispose ();
				OperatorComboBox = null;
			}

			if (RemoveButton != null) {
				RemoveButton.Dispose ();
				RemoveButton = null;
			}

			if (ValueTextField != null) {
				ValueTextField.Dispose ();
				ValueTextField = null;
			}

			if (UpdateButton != null) {
				UpdateButton.Dispose ();
				UpdateButton = null;
			}
		}
	}
}

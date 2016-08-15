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
	[Register ("SelectListItemsWindowController")]
	partial class SelectListItemsWindowController
	{
		[Outlet]
		AppKit.NSSearchField SearchBox { get; set; }

		[Outlet]
		AppKit.NSSearchFieldCell SearchBoxCell { get; set; }

		[Outlet]
		AppKit.NSTableView SelectedItemsListBox { get; set; }

		[Outlet]
		AppKit.NSTableView SelectItemsListBox { get; set; }

		[Action ("AddItems:")]
		partial void AddItems (Foundation.NSObject sender);

		[Action ("ApplyAction:")]
		partial void ApplyAction (Foundation.NSObject sender);

		[Action ("RemoveItems:")]
		partial void RemoveItems (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (SearchBox != null) {
				SearchBox.Dispose ();
				SearchBox = null;
			}

			if (SelectedItemsListBox != null) {
				SelectedItemsListBox.Dispose ();
				SelectedItemsListBox = null;
			}

			if (SelectItemsListBox != null) {
				SelectItemsListBox.Dispose ();
				SelectItemsListBox = null;
			}

			if (SearchBoxCell != null) {
				SearchBoxCell.Dispose ();
				SearchBoxCell = null;
			}
		}
	}
}

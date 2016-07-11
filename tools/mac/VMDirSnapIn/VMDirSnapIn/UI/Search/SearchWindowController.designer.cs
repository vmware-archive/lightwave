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
	[Register ("SearchWindowController")]
	partial class SearchWindowController
	{
		[Outlet]
		AppKit.NSComboBox BFAttrComboBox { get; set; }

		[Outlet]
		AppKit.NSComboBox BFCondComboBox { get; set; }

		[Outlet]
		AppKit.NSTableView BFCondTableView { get; set; }

		[Outlet]
		AppKit.NSComboBox BFOperatorComboBox { get; set; }

		[Outlet]
		AppKit.NSComboBox BFSeachScopeComboBox { get; set; }

		[Outlet]
		AppKit.NSTextField BFSearchBaseTextField { get; set; }

		[Outlet]
		AppKit.NSTextField BFValTextField { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem LoadQueryToolBarItem { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem OperationalAttrToolBarItem { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem PageSizeToolBarItem { get; set; }

		[Outlet]
		AppKit.NSTextField ResultPageNoTextField { get; set; }

		[Outlet]
		AppKit.NSView ResultPropView { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem SearchBoxVisibilityToolBarItem { get; set; }

		[Outlet]
		AppKit.NSSplitView SearchHorizontalSplitView { get; set; }

		[Outlet]
		AppKit.NSView SearchQueryContainerView { get; set; }

		[Outlet]
		AppKit.NSTabView SearchQueryTabView { get; set; }

		[Outlet]
		AppKit.NSView SearchResultContainerView { get; set; }

		[Outlet]
		public AppKit.NSOutlineView SearchResultOutlineView { get; private set; }

		[Outlet]
		AppKit.NSTabView SearQueryTabView { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem StoreQueryToolBarItem { get; set; }

		[Outlet]
		AppKit.NSTextField TFSearchBaseTextField { get; set; }

		[Outlet]
		AppKit.NSTextView TFSearchFilterTextView { get; set; }

		[Outlet]
		AppKit.NSComboBox TFSearchScopeComboBox { get; set; }

		[Action ("BFOnAddAction:")]
		partial void BFOnAddAction (Foundation.NSObject sender);

		[Action ("BFOnSearchAction:")]
		partial void BFOnSearchAction (Foundation.NSObject sender);

		[Action ("BFOnViewAction:")]
		partial void BFOnViewAction (Foundation.NSObject sender);

		[Action ("OnLoadQueryToolBarItem:")]
		partial void OnLoadQueryToolBarItem (Foundation.NSObject sender);

		[Action ("OnNextResultButton:")]
		partial void OnNextResultButton (Foundation.NSObject sender);

		[Action ("OnOperationalToolBarItem:")]
		partial void OnOperationalToolBarItem (Foundation.NSObject sender);

		[Action ("OnPageSizeToolBarItem:")]
		partial void OnPageSizeToolBarItem (Foundation.NSObject sender);

		[Action ("OnPrevResultButton:")]
		partial void OnPrevResultButton (Foundation.NSObject sender);

		[Action ("OnRemoveTableEntry:")]
		partial void OnRemoveTableEntry (Foundation.NSObject sender);

		[Action ("OnSearchBoxVisibilityToolBarItem:")]
		partial void OnSearchBoxVisibilityToolBarItem (Foundation.NSObject sender);

		[Action ("OnStoreQueryToolBarItem:")]
		partial void OnStoreQueryToolBarItem (Foundation.NSObject sender);

		[Action ("TFOnSearchAction:")]
		partial void TFOnSearchAction (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (BFAttrComboBox != null) {
				BFAttrComboBox.Dispose ();
				BFAttrComboBox = null;
			}

			if (BFCondComboBox != null) {
				BFCondComboBox.Dispose ();
				BFCondComboBox = null;
			}

			if (BFCondTableView != null) {
				BFCondTableView.Dispose ();
				BFCondTableView = null;
			}

			if (BFOperatorComboBox != null) {
				BFOperatorComboBox.Dispose ();
				BFOperatorComboBox = null;
			}

			if (BFSeachScopeComboBox != null) {
				BFSeachScopeComboBox.Dispose ();
				BFSeachScopeComboBox = null;
			}

			if (BFSearchBaseTextField != null) {
				BFSearchBaseTextField.Dispose ();
				BFSearchBaseTextField = null;
			}

			if (BFValTextField != null) {
				BFValTextField.Dispose ();
				BFValTextField = null;
			}

			if (LoadQueryToolBarItem != null) {
				LoadQueryToolBarItem.Dispose ();
				LoadQueryToolBarItem = null;
			}

			if (OperationalAttrToolBarItem != null) {
				OperationalAttrToolBarItem.Dispose ();
				OperationalAttrToolBarItem = null;
			}

			if (PageSizeToolBarItem != null) {
				PageSizeToolBarItem.Dispose ();
				PageSizeToolBarItem = null;
			}

			if (ResultPageNoTextField != null) {
				ResultPageNoTextField.Dispose ();
				ResultPageNoTextField = null;
			}

			if (ResultPropView != null) {
				ResultPropView.Dispose ();
				ResultPropView = null;
			}

			if (SearchBoxVisibilityToolBarItem != null) {
				SearchBoxVisibilityToolBarItem.Dispose ();
				SearchBoxVisibilityToolBarItem = null;
			}

			if (SearchHorizontalSplitView != null) {
				SearchHorizontalSplitView.Dispose ();
				SearchHorizontalSplitView = null;
			}

			if (SearchQueryContainerView != null) {
				SearchQueryContainerView.Dispose ();
				SearchQueryContainerView = null;
			}

			if (SearchQueryTabView != null) {
				SearchQueryTabView.Dispose ();
				SearchQueryTabView = null;
			}

			if (SearchResultContainerView != null) {
				SearchResultContainerView.Dispose ();
				SearchResultContainerView = null;
			}

			if (SearchResultOutlineView != null) {
				SearchResultOutlineView.Dispose ();
				SearchResultOutlineView = null;
			}

			if (SearQueryTabView != null) {
				SearQueryTabView.Dispose ();
				SearQueryTabView = null;
			}

			if (StoreQueryToolBarItem != null) {
				StoreQueryToolBarItem.Dispose ();
				StoreQueryToolBarItem = null;
			}

			if (TFSearchBaseTextField != null) {
				TFSearchBaseTextField.Dispose ();
				TFSearchBaseTextField = null;
			}

			if (TFSearchFilterTextView != null) {
				TFSearchFilterTextView.Dispose ();
				TFSearchFilterTextView = null;
			}

			if (TFSearchScopeComboBox != null) {
				TFSearchScopeComboBox.Dispose ();
				TFSearchScopeComboBox = null;
			}
		}
	}
}

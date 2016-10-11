/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

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
		AppKit.NSComboBox AttrToReturnComboBox { get; set; }

		[Outlet]
		AppKit.NSTableView AttrToReturnTableView { get; set; }

		[Outlet]
		AppKit.NSComboBox BfAttributeComboBox { get; set; }

		[Outlet]
		AppKit.NSComboBox BfConditionComboBox { get; set; }

		[Outlet]
		AppKit.NSTableView BfConditionsTableView { get; set; }

		[Outlet]
		AppKit.NSComboBox BfOperatorComboBox { get; set; }

		[Outlet]
		AppKit.NSTextField BfValueTextField { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem DeleteToolBarItem { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem ExportToolBarItem { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem LoadQueryToolBarItem { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem OperationalAttrToolBarItem { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem OptionalToolBarItem { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem PageSizeToolBarItem { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem RefreshToolBarItem { get; set; }

		[Outlet]
		AppKit.NSTextField ResultPageNoTextField { get; set; }

		[Outlet]
		AppKit.NSView ResultPropView { get; set; }

		[Outlet]
		AppKit.NSTextField SearchBaseTextField { get; set; }

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
		AppKit.NSComboBox SearchScopeComboBox { get; set; }

		[Outlet]
		VmIdentity.UI.Common.ActivatableToolBarItem StoreQueryToolBarItem { get; set; }

		[Outlet]
		AppKit.NSTextView TfSearchFilterTextView { get; set; }

		[Action ("BFOnViewAction:")]
		partial void BFOnViewAction (Foundation.NSObject sender);

		[Action ("OnAttrToReturnAdd:")]
		partial void OnAttrToReturnAdd (Foundation.NSObject sender);

		[Action ("OnAttrToReturnRemove:")]
		partial void OnAttrToReturnRemove (Foundation.NSObject sender);

		[Action ("OnAttrToReturnRemoveAll:")]
		partial void OnAttrToReturnRemoveAll (Foundation.NSObject sender);

		[Action ("OnBfAddAction:")]
		partial void OnBfAddAction (Foundation.NSObject sender);

		[Action ("OnBfCopyToTf:")]
		partial void OnBfCopyToTf (Foundation.NSObject sender);

		[Action ("OnBfMultipleValFromFile:")]
		partial void OnBfMultipleValFromFile (Foundation.NSObject sender);

		[Action ("OnBfRemoveAllTableEntries:")]
		partial void OnBfRemoveAllTableEntries (Foundation.NSObject sender);

		[Action ("OnBfRemoveTableEntry:")]
		partial void OnBfRemoveTableEntry (Foundation.NSObject sender);

		[Action ("OnDelete:")]
		partial void OnDelete (Foundation.NSObject sender);

		[Action ("OnExportToolBarItem:")]
		partial void OnExportToolBarItem (Foundation.NSObject sender);

		[Action ("OnLoadQueryToolBarItem:")]
		partial void OnLoadQueryToolBarItem (Foundation.NSObject sender);

		[Action ("OnNextResultButton:")]
		partial void OnNextResultButton (Foundation.NSObject sender);

		[Action ("OnOperationalToolBarItem:")]
		partial void OnOperationalToolBarItem (Foundation.NSObject sender);

		[Action ("OnOptionalToolBarItem:")]
		partial void OnOptionalToolBarItem (Foundation.NSObject sender);

		[Action ("OnPageSizeToolBarItem:")]
		partial void OnPageSizeToolBarItem (Foundation.NSObject sender);

		[Action ("OnPrevResultButton:")]
		partial void OnPrevResultButton (Foundation.NSObject sender);

		[Action ("OnRefresh:")]
		partial void OnRefresh (Foundation.NSObject sender);

		[Action ("OnSearchAction:")]
		partial void OnSearchAction (Foundation.NSObject sender);

		[Action ("OnSearchBoxVisibilityToolBarItem:")]
		partial void OnSearchBoxVisibilityToolBarItem (Foundation.NSObject sender);

		[Action ("OnStoreQueryToolBarItem:")]
		partial void OnStoreQueryToolBarItem (Foundation.NSObject sender);

		[Action ("TFOnSearchAction:")]
		partial void TFOnSearchAction (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (AttrToReturnComboBox != null) {
				AttrToReturnComboBox.Dispose ();
				AttrToReturnComboBox = null;
			}

			if (AttrToReturnTableView != null) {
				AttrToReturnTableView.Dispose ();
				AttrToReturnTableView = null;
			}

			if (BfAttributeComboBox != null) {
				BfAttributeComboBox.Dispose ();
				BfAttributeComboBox = null;
			}

			if (BfConditionComboBox != null) {
				BfConditionComboBox.Dispose ();
				BfConditionComboBox = null;
			}

			if (BfConditionsTableView != null) {
				BfConditionsTableView.Dispose ();
				BfConditionsTableView = null;
			}

			if (BfOperatorComboBox != null) {
				BfOperatorComboBox.Dispose ();
				BfOperatorComboBox = null;
			}

			if (BfValueTextField != null) {
				BfValueTextField.Dispose ();
				BfValueTextField = null;
			}

			if (DeleteToolBarItem != null) {
				DeleteToolBarItem.Dispose ();
				DeleteToolBarItem = null;
			}

			if (ExportToolBarItem != null) {
				ExportToolBarItem.Dispose ();
				ExportToolBarItem = null;
			}

			if (LoadQueryToolBarItem != null) {
				LoadQueryToolBarItem.Dispose ();
				LoadQueryToolBarItem = null;
			}

			if (OperationalAttrToolBarItem != null) {
				OperationalAttrToolBarItem.Dispose ();
				OperationalAttrToolBarItem = null;
			}

			if (OptionalToolBarItem != null) {
				OptionalToolBarItem.Dispose ();
				OptionalToolBarItem = null;
			}

			if (PageSizeToolBarItem != null) {
				PageSizeToolBarItem.Dispose ();
				PageSizeToolBarItem = null;
			}

			if (RefreshToolBarItem != null) {
				RefreshToolBarItem.Dispose ();
				RefreshToolBarItem = null;
			}

			if (ResultPageNoTextField != null) {
				ResultPageNoTextField.Dispose ();
				ResultPageNoTextField = null;
			}

			if (ResultPropView != null) {
				ResultPropView.Dispose ();
				ResultPropView = null;
			}

			if (SearchBaseTextField != null) {
				SearchBaseTextField.Dispose ();
				SearchBaseTextField = null;
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

			if (SearchScopeComboBox != null) {
				SearchScopeComboBox.Dispose ();
				SearchScopeComboBox = null;
			}

			if (StoreQueryToolBarItem != null) {
				StoreQueryToolBarItem.Dispose ();
				StoreQueryToolBarItem = null;
			}

			if (TfSearchFilterTextView != null) {
				TfSearchFilterTextView.Dispose ();
				TfSearchFilterTextView = null;
			}
		}
	}
}

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

namespace VMDirSchemaEditorSnapIn
{
	[Register ("ObjectClassWindowController")]
	partial class ObjectClassWindowController
	{
		[Outlet]
		AppKit.NSButton ActionButton { get; set; }

		[Outlet]
		AppKit.NSButton AddAuxiliaryClassesButton { get; set; }

		[Outlet]
		AppKit.NSButton AddMandatoryAttributesButton { get; set; }

		[Outlet]
		AppKit.NSButton AddObjectClassButton { get; set; }

		[Outlet]
		AppKit.NSButton AddOptionalAttributesButton { get; set; }

		[Outlet]
		AppKit.NSTableView AuxiliaryClassesListView { get; set; }

		[Outlet]
		public AppKit.NSPopUpButton ClassTypePopup { get; private set; }

		[Outlet]
		AppKit.NSTextField GovernsIDField { get; set; }

		[Outlet]
		AppKit.NSButton MandatoryAttributesButton { get; set; }

		[Outlet]
		AppKit.NSTableView MandatoryAttributesListView { get; set; }

		[Outlet]
		public AppKit.NSTextField ObjectClassDescription { get; private set; }

		[Outlet]
		public AppKit.NSTextField ObjectClassID { get; private set; }

		[Outlet]
		public AppKit.NSTextField ObjectClassName { get; private set; }

		[Outlet]
		AppKit.NSButton OptionalAttributesButton { get; set; }

		[Outlet]
		AppKit.NSTableView OptionalAttributesListView { get; set; }

		[Outlet]
		public AppKit.NSTextField ParentClass { get; private set; }

		[Outlet]
		AppKit.NSButton RemoveAuxiliaryClassesButton { get; set; }

		[Outlet]
		AppKit.NSButton RemoveMandatoryAttributesButton { get; set; }

		[Outlet]
		AppKit.NSButton RemoveOptionalAttributesButton { get; set; }

		[Action ("AddAuxiliaryClasses:")]
		partial void AddAuxiliaryClasses (Foundation.NSObject sender);

		[Action ("AddMandatoryAttributes:")]
		partial void AddMandatoryAttributes (Foundation.NSObject sender);

		[Action ("AddObjectClass:")]
		partial void AddObjectClass (Foundation.NSObject sender);

		[Action ("AddOptionalAttributes:")]
		partial void AddOptionalAttributes (Foundation.NSObject sender);

		[Action ("OnActionButton:")]
		partial void OnActionButton (Foundation.NSObject sender);

		[Action ("OnClassTypePopupChanged:")]
		partial void OnClassTypePopupChanged (Foundation.NSObject sender);

		[Action ("RemoveAuxiliaryClasses:")]
		partial void RemoveAuxiliaryClasses (Foundation.NSObject sender);

		[Action ("RemoveMandatoryAttributes:")]
		partial void RemoveMandatoryAttributes (Foundation.NSObject sender);

		[Action ("RemoveOptionalAttributes:")]
		partial void RemoveOptionalAttributes (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (ActionButton != null) {
				ActionButton.Dispose ();
				ActionButton = null;
			}

			if (AddAuxiliaryClassesButton != null) {
				AddAuxiliaryClassesButton.Dispose ();
				AddAuxiliaryClassesButton = null;
			}

			if (AddMandatoryAttributesButton != null) {
				AddMandatoryAttributesButton.Dispose ();
				AddMandatoryAttributesButton = null;
			}

			if (AddObjectClassButton != null) {
				AddObjectClassButton.Dispose ();
				AddObjectClassButton = null;
			}

			if (AddOptionalAttributesButton != null) {
				AddOptionalAttributesButton.Dispose ();
				AddOptionalAttributesButton = null;
			}

			if (AuxiliaryClassesListView != null) {
				AuxiliaryClassesListView.Dispose ();
				AuxiliaryClassesListView = null;
			}

			if (ClassTypePopup != null) {
				ClassTypePopup.Dispose ();
				ClassTypePopup = null;
			}

			if (MandatoryAttributesButton != null) {
				MandatoryAttributesButton.Dispose ();
				MandatoryAttributesButton = null;
			}

			if (MandatoryAttributesListView != null) {
				MandatoryAttributesListView.Dispose ();
				MandatoryAttributesListView = null;
			}

			if (ObjectClassDescription != null) {
				ObjectClassDescription.Dispose ();
				ObjectClassDescription = null;
			}

			if (GovernsIDField != null) {
				GovernsIDField.Dispose ();
				GovernsIDField = null;
			}

			if (ObjectClassID != null) {
				ObjectClassID.Dispose ();
				ObjectClassID = null;
			}

			if (ObjectClassName != null) {
				ObjectClassName.Dispose ();
				ObjectClassName = null;
			}

			if (OptionalAttributesButton != null) {
				OptionalAttributesButton.Dispose ();
				OptionalAttributesButton = null;
			}

			if (OptionalAttributesListView != null) {
				OptionalAttributesListView.Dispose ();
				OptionalAttributesListView = null;
			}

			if (ParentClass != null) {
				ParentClass.Dispose ();
				ParentClass = null;
			}

			if (RemoveAuxiliaryClassesButton != null) {
				RemoveAuxiliaryClassesButton.Dispose ();
				RemoveAuxiliaryClassesButton = null;
			}

			if (RemoveMandatoryAttributesButton != null) {
				RemoveMandatoryAttributesButton.Dispose ();
				RemoveMandatoryAttributesButton = null;
			}

			if (RemoveOptionalAttributesButton != null) {
				RemoveOptionalAttributesButton.Dispose ();
				RemoveOptionalAttributesButton = null;
			}
		}
	}
}

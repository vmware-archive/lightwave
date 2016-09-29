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
 
using Foundation;
using System.CodeDom.Compiler;

namespace RestSsoAdminSnapIn
{
	[Register ("AddNewAttributeConsumerServiceController")]
	partial class AddNewAttributeConsumerServiceController
	{
		[Outlet]
		AppKit.NSTableView AttributeTableView { get; set; }

		[Outlet]
		AppKit.NSButton BtnAdd { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddAttribute { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveAttribute { get; set; }

		[Outlet]
		AppKit.NSButton ChDefault { get; set; }

		[Outlet]
		AppKit.NSTextField TxtAttributeName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtFriendlyName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtIndex { get; set; }

		[Outlet]
		AppKit.NSTextField TxtName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtNameFormat { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (AttributeTableView != null) {
				AttributeTableView.Dispose ();
				AttributeTableView = null;
			}

			if (BtnAdd != null) {
				BtnAdd.Dispose ();
				BtnAdd = null;
			}

			if (BtnAddAttribute != null) {
				BtnAddAttribute.Dispose ();
				BtnAddAttribute = null;
			}

			if (TxtName != null) {
				TxtName.Dispose ();
				TxtName = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnRemoveAttribute != null) {
				BtnRemoveAttribute.Dispose ();
				BtnRemoveAttribute = null;
			}

			if (ChDefault != null) {
				ChDefault.Dispose ();
				ChDefault = null;
			}

			if (TxtAttributeName != null) {
				TxtAttributeName.Dispose ();
				TxtAttributeName = null;
			}

			if (TxtFriendlyName != null) {
				TxtFriendlyName.Dispose ();
				TxtFriendlyName = null;
			}

			if (TxtIndex != null) {
				TxtIndex.Dispose ();
				TxtIndex = null;
			}

			if (TxtNameFormat != null) {
				TxtNameFormat.Dispose ();
				TxtNameFormat = null;
			}
		}
	}
}

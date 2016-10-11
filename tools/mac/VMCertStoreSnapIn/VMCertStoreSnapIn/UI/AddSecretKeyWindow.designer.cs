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

namespace VMCertStoreSnapIn
{
	[Register ("AddSecretKeyWindowController")]
	partial class AddSecretKeyWindowController
	{
		[Outlet]
		public AppKit.NSButton AddButton { get; private set; }

		[Outlet]
		public AppKit.NSTextField AliasTextField { get; private set; }

		[Outlet]
		public AppKit.NSButton CancelButton { get; private set; }

		[Outlet]
		public AppKit.NSSecureTextField PasswordField { get; private set; }

		[Outlet]
		AppKit.NSTextField SecretKeyTextField { get; set; }

		[Action ("OnAdd:")]
		partial void OnAdd (Foundation.NSObject sender);

		[Action ("OnBrowse:")]
		partial void OnBrowse (Foundation.NSObject sender);

		[Action ("OnCancel:")]
		partial void OnCancel (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (AddButton != null) {
				AddButton.Dispose ();
				AddButton = null;
			}

			if (AliasTextField != null) {
				AliasTextField.Dispose ();
				AliasTextField = null;
			}

			if (CancelButton != null) {
				CancelButton.Dispose ();
				CancelButton = null;
			}

			if (PasswordField != null) {
				PasswordField.Dispose ();
				PasswordField = null;
			}

			if (SecretKeyTextField != null) {
				SecretKeyTextField.Dispose ();
				SecretKeyTextField = null;
			}
		}
	}

	[Register ("AddSecretKeyWindow")]
	partial class AddSecretKeyWindow
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

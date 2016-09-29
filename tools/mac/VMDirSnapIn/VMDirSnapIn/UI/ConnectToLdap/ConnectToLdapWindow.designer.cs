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
	[Register ("ConnectToLdapWindowController")]
	partial class ConnectToLdapWindowController
	{
		[Outlet]
		AppKit.NSTextField BaseDN { get; set; }

		[Outlet]
		public AppKit.NSTextField BindDN { get; private set; }

		[Outlet]
		public AppKit.NSButton CancelButton { get; private set; }

		[Outlet]
		public AppKit.NSButton OKButton { get; private set; }

		[Outlet]
		public AppKit.NSSecureTextField Password { get; private set; }

		[Outlet]
		AppKit.NSComboBox ServerComboBox { get; set; }

		[Action ("OnServerComboBox:")]
		partial void OnServerComboBox (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (BaseDN != null) {
				BaseDN.Dispose ();
				BaseDN = null;
			}

			if (BindDN != null) {
				BindDN.Dispose ();
				BindDN = null;
			}

			if (CancelButton != null) {
				CancelButton.Dispose ();
				CancelButton = null;
			}

			if (OKButton != null) {
				OKButton.Dispose ();
				OKButton = null;
			}

			if (Password != null) {
				Password.Dispose ();
				Password = null;
			}

			if (ServerComboBox != null) {
				ServerComboBox.Dispose ();
				ServerComboBox = null;
			}
		}
	}

	[Register ("ConnectToLdapWindow")]
	partial class ConnectToLdapWindow
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

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

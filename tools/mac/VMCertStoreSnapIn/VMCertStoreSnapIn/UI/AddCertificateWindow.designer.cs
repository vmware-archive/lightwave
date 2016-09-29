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

namespace VMCertStoreSnapIn
{
	[Register ("AddCertificateWindowController")]
	partial class AddCertificateWindowController
	{
		[Outlet]
		AppKit.NSButton AddButton { get; set; }

		[Outlet]
		AppKit.NSTextField AliasField { get; set; }

		[Outlet]
		AppKit.NSButton CancelButton { get; set; }

		[Outlet]
		AppKit.NSTextField CertificateField { get; set; }

		[Outlet]
		AppKit.NSButton OpenFileButton { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (AliasField != null) {
				AliasField.Dispose ();
				AliasField = null;
			}

			if (CertificateField != null) {
				CertificateField.Dispose ();
				CertificateField = null;
			}

			if (OpenFileButton != null) {
				OpenFileButton.Dispose ();
				OpenFileButton = null;
			}

			if (AddButton != null) {
				AddButton.Dispose ();
				AddButton = null;
			}

			if (CancelButton != null) {
				CancelButton.Dispose ();
				CancelButton = null;
			}
		}
	}

	[Register ("AddCertificateWindow")]
	partial class AddCertificateWindow
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

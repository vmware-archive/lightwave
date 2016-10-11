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
	[Register ("ServerInfoPopOverController")]
	partial class ServerInfoPopOverController
	{
		[Outlet]
		AppKit.NSTextField NoStoresLabel { get; set; }

		[Outlet]
		AppKit.NSTextField PrivateKeysLabel { get; set; }

		[Outlet]
		AppKit.NSTextField SecretKeysLabel { get; set; }

		[Outlet]
		AppKit.NSTextField TrustedCertsLabel { get; set; }

		[Action ("AddRootCertificate:")]
		partial void AddRootCertificate (Foundation.NSObject sender);

		[Action ("CreateStore:")]
		partial void CreateStore (Foundation.NSObject sender);

		[Action ("ShowRootCertificate:")]
		partial void ShowRootCertificate (Foundation.NSObject sender);

		[Action ("ValidateCA:")]
		partial void ValidateCA (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (NoStoresLabel != null) {
				NoStoresLabel.Dispose ();
				NoStoresLabel = null;
			}

			if (PrivateKeysLabel != null) {
				PrivateKeysLabel.Dispose ();
				PrivateKeysLabel = null;
			}

			if (TrustedCertsLabel != null) {
				TrustedCertsLabel.Dispose ();
				TrustedCertsLabel = null;
			}

			if (SecretKeysLabel != null) {
				SecretKeysLabel.Dispose ();
				SecretKeysLabel = null;
			}
		}
	}
}

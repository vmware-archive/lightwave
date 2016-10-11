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
	[Register ("JoinActiveDirectoryController")]
	partial class JoinActiveDirectoryController
	{
		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnJoin { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDomain { get; set; }

		[Outlet]
		AppKit.NSTextField TxtOU { get; set; }

		[Outlet]
		AppKit.NSSecureTextField TxtPassword { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUsername { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (TxtUsername != null) {
				TxtUsername.Dispose ();
				TxtUsername = null;
			}

			if (TxtPassword != null) {
				TxtPassword.Dispose ();
				TxtPassword = null;
			}

			if (TxtDomain != null) {
				TxtDomain.Dispose ();
				TxtDomain = null;
			}

			if (TxtOU != null) {
				TxtOU.Dispose ();
				TxtOU = null;
			}

			if (BtnJoin != null) {
				BtnJoin.Dispose ();
				BtnJoin = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}
		}
	}

	[Register ("JoinActiveDirectory")]
	partial class JoinActiveDirectory
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

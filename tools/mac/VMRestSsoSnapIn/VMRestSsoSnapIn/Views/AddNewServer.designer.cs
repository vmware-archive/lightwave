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
	[Register ("AddNewServerController")]
	partial class AddNewServerController
	{
		[Outlet]
		AppKit.NSButton BtnAdd { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton CbSaml { get; set; }

		[Outlet]
		AppKit.NSButton CbSsl { get; set; }

		[Outlet]
		AppKit.NSTextField LblStsEndpoint { get; set; }

		[Outlet]
		AppKit.NSTextField LblUrl { get; set; }

		[Outlet]
		AppKit.NSSecureTextField TxtPassword { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPort { get; set; }

		[Outlet]
		AppKit.NSTextField TxtServer { get; set; }

		[Outlet]
		AppKit.NSTextField TxtStsEndpoint { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTenant { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUsername { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnAdd != null) {
				BtnAdd.Dispose ();
				BtnAdd = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (CbSaml != null) {
				CbSaml.Dispose ();
				CbSaml = null;
			}

			if (CbSsl != null) {
				CbSsl.Dispose ();
				CbSsl = null;
			}

			if (LblStsEndpoint != null) {
				LblStsEndpoint.Dispose ();
				LblStsEndpoint = null;
			}

			if (LblUrl != null) {
				LblUrl.Dispose ();
				LblUrl = null;
			}

			if (TxtPort != null) {
				TxtPort.Dispose ();
				TxtPort = null;
			}

			if (TxtServer != null) {
				TxtServer.Dispose ();
				TxtServer = null;
			}

			if (TxtStsEndpoint != null) {
				TxtStsEndpoint.Dispose ();
				TxtStsEndpoint = null;
			}

			if (TxtTenant != null) {
				TxtTenant.Dispose ();
				TxtTenant = null;
			}

			if (TxtUsername != null) {
				TxtUsername.Dispose ();
				TxtUsername = null;
			}

			if (TxtPassword != null) {
				TxtPassword.Dispose ();
				TxtPassword = null;
			}
		}
	}

	[Register ("AddNewServer")]
	partial class AddNewServer
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

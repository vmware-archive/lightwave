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
	[Register ("AddSolutionUserController")]
	partial class AddSolutionUserController
	{
		[Outlet]
		AppKit.NSButton BtnAddNew { get; set; }

		[Outlet]
		AppKit.NSButton BtnBrowseCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnCloseNew { get; set; }

		[Outlet]
		AppKit.NSTextField TxtCertificatePath { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDescription { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUsername { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnAddNew != null) {
				BtnAddNew.Dispose ();
				BtnAddNew = null;
			}

			if (BtnCloseNew != null) {
				BtnCloseNew.Dispose ();
				BtnCloseNew = null;
			}

			if (BtnBrowseCertificate != null) {
				BtnBrowseCertificate.Dispose ();
				BtnBrowseCertificate = null;
			}

			if (TxtCertificatePath != null) {
				TxtCertificatePath.Dispose ();
				TxtCertificatePath = null;
			}

			if (TxtDescription != null) {
				TxtDescription.Dispose ();
				TxtDescription = null;
			}

			if (TxtUsername != null) {
				TxtUsername.Dispose ();
				TxtUsername = null;
			}
		}
	}

	[Register ("AddSolutionUser")]
	partial class AddSolutionUser
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

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
	[Register ("ViewSolutionUserDetailsController")]
	partial class ViewSolutionUserDetailsController
	{
		[Outlet]
		AppKit.NSButton BtnChangeCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnSave { get; set; }

		[Outlet]
		AppKit.NSButton CbDisabled { get; set; }

		[Outlet]
		AppKit.NSView ImageView { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDC { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDescription { get; set; }

		[Outlet]
		AppKit.NSTextField TxtIssuer { get; set; }

		[Outlet]
		AppKit.NSTextField TxtName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtValidFrom { get; set; }

		[Outlet]
		AppKit.NSTextField TxtValidTo { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnChangeCertificate != null) {
				BtnChangeCertificate.Dispose ();
				BtnChangeCertificate = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnSave != null) {
				BtnSave.Dispose ();
				BtnSave = null;
			}

			if (CbDisabled != null) {
				CbDisabled.Dispose ();
				CbDisabled = null;
			}

			if (ImageView != null) {
				ImageView.Dispose ();
				ImageView = null;
			}

			if (TxtDescription != null) {
				TxtDescription.Dispose ();
				TxtDescription = null;
			}

			if (TxtName != null) {
				TxtName.Dispose ();
				TxtName = null;
			}

			if (TxtIssuer != null) {
				TxtIssuer.Dispose ();
				TxtIssuer = null;
			}

			if (TxtValidFrom != null) {
				TxtValidFrom.Dispose ();
				TxtValidFrom = null;
			}

			if (TxtValidTo != null) {
				TxtValidTo.Dispose ();
				TxtValidTo = null;
			}

			if (TxtDC != null) {
				TxtDC.Dispose ();
				TxtDC = null;
			}
		}
	}

	[Register ("ViewSolutionUserDetails")]
	partial class ViewSolutionUserDetails
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

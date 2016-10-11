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
	[Register ("AddExternalIdentityProviderController")]
	partial class AddExternalIdentityProviderController
	{
		[Outlet]
		AppKit.NSButton BtnAddCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddNameIdFormat { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddSloService { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddSsoService { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddSubjectFormat { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveNameIdFormat { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveSloService { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveSsoService { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveSubjectFormat { get; set; }

		[Outlet]
		AppKit.NSButton BtnSave { get; set; }

		[Outlet]
		AppKit.NSButton ChkJit { get; set; }

		[Outlet]
		AppKit.NSScrollView LstCertificates { get; set; }

		[Outlet]
		AppKit.NSScrollView LstNameIdFormats { get; set; }

		[Outlet]
		AppKit.NSScrollView LstSloServices { get; set; }

		[Outlet]
		AppKit.NSScrollView LstSsoServices { get; set; }

		[Outlet]
		AppKit.NSScrollView LstSubjectFormats { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUniqueId { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (TxtUniqueId != null) {
				TxtUniqueId.Dispose ();
				TxtUniqueId = null;
			}

			if (ChkJit != null) {
				ChkJit.Dispose ();
				ChkJit = null;
			}

			if (BtnAddCertificate != null) {
				BtnAddCertificate.Dispose ();
				BtnAddCertificate = null;
			}

			if (BtnRemoveCertificate != null) {
				BtnRemoveCertificate.Dispose ();
				BtnRemoveCertificate = null;
			}

			if (LstCertificates != null) {
				LstCertificates.Dispose ();
				LstCertificates = null;
			}

			if (LstSsoServices != null) {
				LstSsoServices.Dispose ();
				LstSsoServices = null;
			}

			if (BtnAddSsoService != null) {
				BtnAddSsoService.Dispose ();
				BtnAddSsoService = null;
			}

			if (BtnRemoveSsoService != null) {
				BtnRemoveSsoService.Dispose ();
				BtnRemoveSsoService = null;
			}

			if (BtnAddSloService != null) {
				BtnAddSloService.Dispose ();
				BtnAddSloService = null;
			}

			if (BtnRemoveSloService != null) {
				BtnRemoveSloService.Dispose ();
				BtnRemoveSloService = null;
			}

			if (LstSloServices != null) {
				LstSloServices.Dispose ();
				LstSloServices = null;
			}

			if (BtnAddSubjectFormat != null) {
				BtnAddSubjectFormat.Dispose ();
				BtnAddSubjectFormat = null;
			}

			if (BtnRemoveSubjectFormat != null) {
				BtnRemoveSubjectFormat.Dispose ();
				BtnRemoveSubjectFormat = null;
			}

			if (LstSubjectFormats != null) {
				LstSubjectFormats.Dispose ();
				LstSubjectFormats = null;
			}

			if (BtnAddNameIdFormat != null) {
				BtnAddNameIdFormat.Dispose ();
				BtnAddNameIdFormat = null;
			}

			if (BtnRemoveNameIdFormat != null) {
				BtnRemoveNameIdFormat.Dispose ();
				BtnRemoveNameIdFormat = null;
			}

			if (LstNameIdFormats != null) {
				LstNameIdFormats.Dispose ();
				LstNameIdFormats = null;
			}

			if (BtnSave != null) {
				BtnSave.Dispose ();
				BtnSave = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}
		}
	}
}

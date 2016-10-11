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
	[Register ("AddNewExternalIdentityProviderController")]
	partial class AddNewExternalIdentityProviderController
	{
		[Outlet]
		AppKit.NSButton BtnAddCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddNameIdFormat { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddSlo { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddSso { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddSubjectFormat { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveNameIdFormat { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveSlo { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveSso { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveSubjectFormat { get; set; }

		[Outlet]
		AppKit.NSButton BtnSave { get; set; }

		[Outlet]
		AppKit.NSButton BtnViewCertificate { get; set; }

		[Outlet]
		AppKit.NSButton ChkJit { get; set; }

		[Outlet]
		AppKit.NSTableView LstCertificates { get; set; }

		[Outlet]
		AppKit.NSTableView LstNameIdFormat { get; set; }

		[Outlet]
		AppKit.NSTableView LstSlo { get; set; }

		[Outlet]
		AppKit.NSTableView LstSso { get; set; }

		[Outlet]
		AppKit.NSTableView LstSubjectFormat { get; set; }

		[Outlet]
		AppKit.NSTextField TxtAlias { get; set; }

		[Outlet]
		AppKit.NSTextField TxtNameIdFormat { get; set; }

		[Outlet]
		AppKit.NSTextField TxtSloBinding { get; set; }

		[Outlet]
		AppKit.NSTextField TxtSloEndpoint { get; set; }

		[Outlet]
		AppKit.NSTextField TxtSloName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtSsoBinding { get; set; }

		[Outlet]
		AppKit.NSTextField TxtSsoEndpoint { get; set; }

		[Outlet]
		AppKit.NSTextField TxtSsoName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtSubjectFormatName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtSubjectFormatValue { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUniqueId { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (TxtAlias != null) {
				TxtAlias.Dispose ();
				TxtAlias = null;
			}

			if (BtnAddCertificate != null) {
				BtnAddCertificate.Dispose ();
				BtnAddCertificate = null;
			}

			if (BtnAddNameIdFormat != null) {
				BtnAddNameIdFormat.Dispose ();
				BtnAddNameIdFormat = null;
			}

			if (BtnAddSlo != null) {
				BtnAddSlo.Dispose ();
				BtnAddSlo = null;
			}

			if (BtnAddSso != null) {
				BtnAddSso.Dispose ();
				BtnAddSso = null;
			}

			if (BtnAddSubjectFormat != null) {
				BtnAddSubjectFormat.Dispose ();
				BtnAddSubjectFormat = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnRemoveCertificate != null) {
				BtnRemoveCertificate.Dispose ();
				BtnRemoveCertificate = null;
			}

			if (BtnRemoveNameIdFormat != null) {
				BtnRemoveNameIdFormat.Dispose ();
				BtnRemoveNameIdFormat = null;
			}

			if (BtnRemoveSlo != null) {
				BtnRemoveSlo.Dispose ();
				BtnRemoveSlo = null;
			}

			if (BtnRemoveSso != null) {
				BtnRemoveSso.Dispose ();
				BtnRemoveSso = null;
			}

			if (BtnRemoveSubjectFormat != null) {
				BtnRemoveSubjectFormat.Dispose ();
				BtnRemoveSubjectFormat = null;
			}

			if (BtnSave != null) {
				BtnSave.Dispose ();
				BtnSave = null;
			}

			if (BtnViewCertificate != null) {
				BtnViewCertificate.Dispose ();
				BtnViewCertificate = null;
			}

			if (ChkJit != null) {
				ChkJit.Dispose ();
				ChkJit = null;
			}

			if (LstCertificates != null) {
				LstCertificates.Dispose ();
				LstCertificates = null;
			}

			if (LstNameIdFormat != null) {
				LstNameIdFormat.Dispose ();
				LstNameIdFormat = null;
			}

			if (LstSlo != null) {
				LstSlo.Dispose ();
				LstSlo = null;
			}

			if (LstSso != null) {
				LstSso.Dispose ();
				LstSso = null;
			}

			if (LstSubjectFormat != null) {
				LstSubjectFormat.Dispose ();
				LstSubjectFormat = null;
			}

			if (TxtNameIdFormat != null) {
				TxtNameIdFormat.Dispose ();
				TxtNameIdFormat = null;
			}

			if (TxtSloBinding != null) {
				TxtSloBinding.Dispose ();
				TxtSloBinding = null;
			}

			if (TxtSloEndpoint != null) {
				TxtSloEndpoint.Dispose ();
				TxtSloEndpoint = null;
			}

			if (TxtSloName != null) {
				TxtSloName.Dispose ();
				TxtSloName = null;
			}

			if (TxtSsoBinding != null) {
				TxtSsoBinding.Dispose ();
				TxtSsoBinding = null;
			}

			if (TxtSsoEndpoint != null) {
				TxtSsoEndpoint.Dispose ();
				TxtSsoEndpoint = null;
			}

			if (TxtSsoName != null) {
				TxtSsoName.Dispose ();
				TxtSsoName = null;
			}

			if (TxtSubjectFormatName != null) {
				TxtSubjectFormatName.Dispose ();
				TxtSubjectFormatName = null;
			}

			if (TxtSubjectFormatValue != null) {
				TxtSubjectFormatValue.Dispose ();
				TxtSubjectFormatValue = null;
			}

			if (TxtUniqueId != null) {
				TxtUniqueId.Dispose ();
				TxtUniqueId = null;
			}
		}
	}
}

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
	[Register ("AddRelyingPartyController")]
	partial class AddRelyingPartyController
	{
		[Outlet]
		AppKit.NSTableView AssertionTableView { get; set; }

		[Outlet]
		AppKit.NSTableView AttributeTableView { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddAssertServices { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddAttributeServices { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddSignatureAlgorithm { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddSlo { get; set; }

		[Outlet]
		AppKit.NSButton BtnBrowseCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveAssertServices { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveAttributeServices { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveSignatureAlgorithm { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveSlo { get; set; }

		[Outlet]
		AppKit.NSButton BtnSave { get; set; }

		[Outlet]
		AppKit.NSButton CbSignAuthRequest { get; set; }

		[Outlet]
		AppKit.NSTableView SignAlgoTableView { get; set; }

		[Outlet]
		AppKit.NSTableView SloTableView { get; set; }

		[Outlet]
		AppKit.NSTextField TxtCertificate { get; set; }

		[Outlet]
		AppKit.NSTextField TxtName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUrl { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BtnAddAssertServices != null) {
				BtnAddAssertServices.Dispose ();
				BtnAddAssertServices = null;
			}

			if (BtnAddAttributeServices != null) {
				BtnAddAttributeServices.Dispose ();
				BtnAddAttributeServices = null;
			}

			if (BtnAddSignatureAlgorithm != null) {
				BtnAddSignatureAlgorithm.Dispose ();
				BtnAddSignatureAlgorithm = null;
			}

			if (BtnAddSlo != null) {
				BtnAddSlo.Dispose ();
				BtnAddSlo = null;
			}

			if (AssertionTableView != null) {
				AssertionTableView.Dispose ();
				AssertionTableView = null;
			}

			if (AttributeTableView != null) {
				AttributeTableView.Dispose ();
				AttributeTableView = null;
			}

			if (BtnBrowseCertificate != null) {
				BtnBrowseCertificate.Dispose ();
				BtnBrowseCertificate = null;
			}

			if (SloTableView != null) {
				SloTableView.Dispose ();
				SloTableView = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnRemoveAssertServices != null) {
				BtnRemoveAssertServices.Dispose ();
				BtnRemoveAssertServices = null;
			}

			if (BtnRemoveAttributeServices != null) {
				BtnRemoveAttributeServices.Dispose ();
				BtnRemoveAttributeServices = null;
			}

			if (BtnRemoveSignatureAlgorithm != null) {
				BtnRemoveSignatureAlgorithm.Dispose ();
				BtnRemoveSignatureAlgorithm = null;
			}

			if (BtnRemoveSlo != null) {
				BtnRemoveSlo.Dispose ();
				BtnRemoveSlo = null;
			}

			if (BtnSave != null) {
				BtnSave.Dispose ();
				BtnSave = null;
			}

			if (CbSignAuthRequest != null) {
				CbSignAuthRequest.Dispose ();
				CbSignAuthRequest = null;
			}

			if (SignAlgoTableView != null) {
				SignAlgoTableView.Dispose ();
				SignAlgoTableView = null;
			}

			if (TxtCertificate != null) {
				TxtCertificate.Dispose ();
				TxtCertificate = null;
			}

			if (TxtName != null) {
				TxtName.Dispose ();
				TxtName = null;
			}

			if (TxtUrl != null) {
				TxtUrl.Dispose ();
				TxtUrl = null;
			}
		}
	}

	[Register ("AddRelyingParty")]
	partial class AddRelyingParty
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

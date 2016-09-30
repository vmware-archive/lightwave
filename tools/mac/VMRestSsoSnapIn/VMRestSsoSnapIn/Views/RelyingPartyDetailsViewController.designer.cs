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
	[Register ("RelyingPartyDetailsViewController")]
	partial class RelyingPartyDetailsViewController
	{
		[Outlet]
		AppKit.NSTableView AssertTableView { get; set; }

		[Outlet]
		AppKit.NSTableView AttributeTableView { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddAssertServices { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddAttributeService { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddSignAlgo { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddSloService { get; set; }

		[Outlet]
		AppKit.NSButton BtnApply { get; set; }

		[Outlet]
		AppKit.NSButton BtnBrowseCertificate { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveAssertService { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveAttributeService { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveSignAlgo { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveSloService { get; set; }

		[Outlet]
		AppKit.NSButton BtnViewCertificate { get; set; }

		[Outlet]
		AppKit.NSButton ChkSign { get; set; }

		[Outlet]
		AppKit.NSTableView SignAlgorithmTableView { get; set; }

		[Outlet]
		AppKit.NSTableView SloServicesTableView { get; set; }

		[Outlet]
		AppKit.NSTextField TxtCertificate { get; set; }

		[Outlet]
		AppKit.NSTextField TxtRpName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUrl { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (AssertTableView != null) {
				AssertTableView.Dispose ();
				AssertTableView = null;
			}

			if (AttributeTableView != null) {
				AttributeTableView.Dispose ();
				AttributeTableView = null;
			}

			if (BtnAddAssertServices != null) {
				BtnAddAssertServices.Dispose ();
				BtnAddAssertServices = null;
			}

			if (BtnAddAttributeService != null) {
				BtnAddAttributeService.Dispose ();
				BtnAddAttributeService = null;
			}

			if (BtnAddSignAlgo != null) {
				BtnAddSignAlgo.Dispose ();
				BtnAddSignAlgo = null;
			}

			if (BtnApply != null) {
				BtnApply.Dispose ();
				BtnApply = null;
			}

			if (BtnAddSloService != null) {
				BtnAddSloService.Dispose ();
				BtnAddSloService = null;
			}

			if (BtnRemoveSloService != null) {
				BtnRemoveSloService.Dispose ();
				BtnRemoveSloService = null;
			}

			if (BtnBrowseCertificate != null) {
				BtnBrowseCertificate.Dispose ();
				BtnBrowseCertificate = null;
			}

			if (BtnRemoveAssertService != null) {
				BtnRemoveAssertService.Dispose ();
				BtnRemoveAssertService = null;
			}

			if (BtnRemoveAttributeService != null) {
				BtnRemoveAttributeService.Dispose ();
				BtnRemoveAttributeService = null;
			}

			if (BtnRemoveSignAlgo != null) {
				BtnRemoveSignAlgo.Dispose ();
				BtnRemoveSignAlgo = null;
			}

			if (BtnViewCertificate != null) {
				BtnViewCertificate.Dispose ();
				BtnViewCertificate = null;
			}

			if (ChkSign != null) {
				ChkSign.Dispose ();
				ChkSign = null;
			}

			if (SignAlgorithmTableView != null) {
				SignAlgorithmTableView.Dispose ();
				SignAlgorithmTableView = null;
			}

			if (SloServicesTableView != null) {
				SloServicesTableView.Dispose ();
				SloServicesTableView = null;
			}

			if (TxtCertificate != null) {
				TxtCertificate.Dispose ();
				TxtCertificate = null;
			}

			if (TxtRpName != null) {
				TxtRpName.Dispose ();
				TxtRpName = null;
			}

			if (TxtUrl != null) {
				TxtUrl.Dispose ();
				TxtUrl = null;
			}
		}
	}
}

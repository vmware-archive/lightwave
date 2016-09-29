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

namespace VMCASnapIn.UI
{
	[Register ("CreateCertificateWindowController")]
	partial class CreateCertificateWindowController
	{
		[Outlet]
		AppKit.NSButton CancelButton { get; set; }

		[Outlet]
		AppKit.NSTextField Country { get; set; }

		[Outlet]
		AppKit.NSPopUpButton CountryPopUpButton { get; set; }

		[Outlet]
		AppKit.NSButton CreateButton { get; set; }

		[Outlet]
		AppKit.NSTextField DNSName { get; set; }

		[Outlet]
		AppKit.NSTextField Email { get; set; }

		[Outlet]
		AppKit.NSTextField IPAddress { get; set; }

		[Outlet]
		AppKit.NSTextField KeyUSageContraints { get; set; }

		[Outlet]
		AppKit.NSTextField Locality { get; set; }

		[Outlet]
		AppKit.NSTextField Name { get; set; }

		[Outlet]
		AppKit.NSDatePicker NotAfter { get; set; }

		[Outlet]
		AppKit.NSDatePicker NotBefore { get; set; }

		[Outlet]
		AppKit.NSTextField Organization { get; set; }

		[Outlet]
		AppKit.NSTextField OU { get; set; }

		[Outlet]
		AppKit.NSTextField PrivateKey { get; set; }

		[Outlet]
		AppKit.NSButton SelectPriKey { get; set; }

		[Outlet]
		AppKit.NSTextField State { get; set; }

		[Outlet]
		AppKit.NSTextField URIName { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (CountryPopUpButton != null) {
				CountryPopUpButton.Dispose ();
				CountryPopUpButton = null;
			}

			if (CancelButton != null) {
				CancelButton.Dispose ();
				CancelButton = null;
			}

			if (Country != null) {
				Country.Dispose ();
				Country = null;
			}

			if (CreateButton != null) {
				CreateButton.Dispose ();
				CreateButton = null;
			}

			if (DNSName != null) {
				DNSName.Dispose ();
				DNSName = null;
			}

			if (Email != null) {
				Email.Dispose ();
				Email = null;
			}

			if (IPAddress != null) {
				IPAddress.Dispose ();
				IPAddress = null;
			}

			if (KeyUSageContraints != null) {
				KeyUSageContraints.Dispose ();
				KeyUSageContraints = null;
			}

			if (Locality != null) {
				Locality.Dispose ();
				Locality = null;
			}

			if (Name != null) {
				Name.Dispose ();
				Name = null;
			}

			if (NotAfter != null) {
				NotAfter.Dispose ();
				NotAfter = null;
			}

			if (NotBefore != null) {
				NotBefore.Dispose ();
				NotBefore = null;
			}

			if (Organization != null) {
				Organization.Dispose ();
				Organization = null;
			}

			if (OU != null) {
				OU.Dispose ();
				OU = null;
			}

			if (PrivateKey != null) {
				PrivateKey.Dispose ();
				PrivateKey = null;
			}

			if (SelectPriKey != null) {
				SelectPriKey.Dispose ();
				SelectPriKey = null;
			}

			if (State != null) {
				State.Dispose ();
				State = null;
			}

			if (URIName != null) {
				URIName.Dispose ();
				URIName = null;
			}
		}
	}

	[Register ("CreateCertificateWindow")]
	partial class CreateCertificateWindow
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

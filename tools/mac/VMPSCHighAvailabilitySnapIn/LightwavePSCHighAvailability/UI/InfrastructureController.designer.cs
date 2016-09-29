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

namespace VMPSCHighAvailability.UI
{
	[Register ("InfrastructureController")]
	partial class InfrastructureController
	{
		[Outlet]
		AppKit.NSTextField HealthTextField { get; set; }

		[Outlet]
		AppKit.NSTextField HotnametextField { get; set; }

		[Outlet]
		AppKit.NSTextField IpAddressTextField { get; set; }

		[Outlet]
		AppKit.NSTableView ServiceTableView { get; set; }

		[Outlet]
		AppKit.NSTextField SitenameTextField { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (HealthTextField != null) {
				HealthTextField.Dispose ();
				HealthTextField = null;
			}

			if (HotnametextField != null) {
				HotnametextField.Dispose ();
				HotnametextField = null;
			}

			if (ServiceTableView != null) {
				ServiceTableView.Dispose ();
				ServiceTableView = null;
			}

			if (SitenameTextField != null) {
				SitenameTextField.Dispose ();
				SitenameTextField = null;
			}

			if (IpAddressTextField != null) {
				IpAddressTextField.Dispose ();
				IpAddressTextField = null;
			}
		}
	}
}

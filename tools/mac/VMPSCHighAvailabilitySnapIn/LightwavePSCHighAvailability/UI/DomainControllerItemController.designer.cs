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

// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMPSCHighAvailability.UI
{
	[Register ("DomainControllerItemController")]
	partial class DomainControllerItemController
	{
		[Outlet]
		AppKit.NSTextField AffinitizedTextField { get; set; }

		[Outlet]
		AppKit.NSButton ConnectButton { get; set; }

		[Outlet]
		AppKit.NSTextField DcTextField { get; set; }

		[Outlet]
		AppKit.NSTextField LastHeartBeatTextField { get; set; }

		[Outlet]
		AppKit.NSButton ServicesButton { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (DcTextField != null) {
				DcTextField.Dispose ();
				DcTextField = null;
			}

			if (AffinitizedTextField != null) {
				AffinitizedTextField.Dispose ();
				AffinitizedTextField = null;
			}

			if (ConnectButton != null) {
				ConnectButton.Dispose ();
				ConnectButton = null;
			}

			if (ServicesButton != null) {
				ServicesButton.Dispose ();
				ServicesButton = null;
			}

			if (LastHeartBeatTextField != null) {
				LastHeartBeatTextField.Dispose ();
				LastHeartBeatTextField = null;
			}
		}
	}
}

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

namespace VMDNS
{
	[Register ("AddNewZoneController")]
	partial class AddNewZoneController
	{
		[Outlet]
		AppKit.NSTextField AdminEmailForwardZone { get; set; }

		[Outlet]
		AppKit.NSTextField AdminEmailReverseZone { get; set; }

		[Outlet]
		AppKit.NSBox ForwardZoneBox { get; set; }

		[Outlet]
		AppKit.NSTextField HostIPForwardZone { get; set; }

		[Outlet]
		AppKit.NSTextField HostNameForwardZone { get; set; }

		[Outlet]
		AppKit.NSTextField HostNameReverseZone { get; set; }

		[Outlet]
		AppKit.NSTextField NetworkIDReverseZone { get; set; }

		[Outlet]
		AppKit.NSBox NewZoneContainerBox { get; set; }

		[Outlet]
		AppKit.NSTextField NoBitsReverseZone { get; set; }

		[Outlet]
		AppKit.NSBox ReverseZoneBox { get; set; }

		[Outlet]
		AppKit.NSTextField ZoneNameForwardZone { get; set; }

		[Action ("OnAddZone:")]
		partial void OnAddZone (Foundation.NSObject sender);

		[Action ("OnCancel:")]
		partial void OnCancel (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (HostNameReverseZone != null) {
				HostNameReverseZone.Dispose ();
				HostNameReverseZone = null;
			}

			if (AdminEmailForwardZone != null) {
				AdminEmailForwardZone.Dispose ();
				AdminEmailForwardZone = null;
			}

			if (AdminEmailReverseZone != null) {
				AdminEmailReverseZone.Dispose ();
				AdminEmailReverseZone = null;
			}

			if (ForwardZoneBox != null) {
				ForwardZoneBox.Dispose ();
				ForwardZoneBox = null;
			}

			if (HostIPForwardZone != null) {
				HostIPForwardZone.Dispose ();
				HostIPForwardZone = null;
			}

			if (HostNameForwardZone != null) {
				HostNameForwardZone.Dispose ();
				HostNameForwardZone = null;
			}

			if (NetworkIDReverseZone != null) {
				NetworkIDReverseZone.Dispose ();
				NetworkIDReverseZone = null;
			}

			if (NewZoneContainerBox != null) {
				NewZoneContainerBox.Dispose ();
				NewZoneContainerBox = null;
			}

			if (NoBitsReverseZone != null) {
				NoBitsReverseZone.Dispose ();
				NoBitsReverseZone = null;
			}

			if (ReverseZoneBox != null) {
				ReverseZoneBox.Dispose ();
				ReverseZoneBox = null;
			}

			if (ZoneNameForwardZone != null) {
				ZoneNameForwardZone.Dispose ();
				ZoneNameForwardZone = null;
			}
		}
	}
}

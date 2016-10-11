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
	[Register ("SRVRecordController")]
	partial class SRVRecordController
	{
		[Outlet]
		AppKit.NSTextField PortField { get; set; }

		[Outlet]
		AppKit.NSTextField PriorityField { get; set; }

		[Outlet]
		AppKit.NSTextField ProtocolLabel { get; set; }

		[Outlet]
		AppKit.NSPopUpButton ProtocolOptionsField { get; set; }

		[Outlet]
		AppKit.NSTextField RecordNameField { get; set; }

		[Outlet]
		AppKit.NSTextField RecordNameLabel { get; set; }

		[Outlet]
		AppKit.NSPopUpButton ServiceOptionsField { get; set; }

		[Outlet]
		AppKit.NSTextField ServiceOptionsLabel { get; set; }

		[Outlet]
		AppKit.NSTextField TargetHostField { get; set; }

		[Outlet]
		AppKit.NSTextField WeightField { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (ServiceOptionsLabel != null) {
				ServiceOptionsLabel.Dispose ();
				ServiceOptionsLabel = null;
			}

			if (ProtocolLabel != null) {
				ProtocolLabel.Dispose ();
				ProtocolLabel = null;
			}

			if (RecordNameLabel != null) {
				RecordNameLabel.Dispose ();
				RecordNameLabel = null;
			}

			if (PortField != null) {
				PortField.Dispose ();
				PortField = null;
			}

			if (RecordNameField != null) {
				RecordNameField.Dispose ();
				RecordNameField = null;
			}

			if (PriorityField != null) {
				PriorityField.Dispose ();
				PriorityField = null;
			}

			if (ProtocolOptionsField != null) {
				ProtocolOptionsField.Dispose ();
				ProtocolOptionsField = null;
			}

			if (ServiceOptionsField != null) {
				ServiceOptionsField.Dispose ();
				ServiceOptionsField = null;
			}

			if (TargetHostField != null) {
				TargetHostField.Dispose ();
				TargetHostField = null;
			}

			if (WeightField != null) {
				WeightField.Dispose ();
				WeightField = null;
			}
		}
	}
}

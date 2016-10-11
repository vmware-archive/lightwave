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
	[Register ("AddNewAssertionConsumerServiceController")]
	partial class AddNewAssertionConsumerServiceController
	{
		[Outlet]
		AppKit.NSButton BtnAdd { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton ChkDefault { get; set; }

		[Outlet]
		AppKit.NSTextField TxtBinding { get; set; }

		[Outlet]
		AppKit.NSTextField TxtEndpoint { get; set; }

		[Outlet]
		AppKit.NSTextField TxtIndex { get; set; }

		[Outlet]
		AppKit.NSTextField TxtName { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (TxtName != null) {
				TxtName.Dispose ();
				TxtName = null;
			}

			if (TxtEndpoint != null) {
				TxtEndpoint.Dispose ();
				TxtEndpoint = null;
			}

			if (TxtBinding != null) {
				TxtBinding.Dispose ();
				TxtBinding = null;
			}

			if (TxtIndex != null) {
				TxtIndex.Dispose ();
				TxtIndex = null;
			}

			if (ChkDefault != null) {
				ChkDefault.Dispose ();
				ChkDefault = null;
			}

			if (BtnAdd != null) {
				BtnAdd.Dispose ();
				BtnAdd = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}
		}
	}
}

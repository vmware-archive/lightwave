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

namespace VMDirSnapIn.UI
{
	[Register ("PropertiesViewController")]
	partial class PropertiesViewController
	{
		[Outlet]
		AppKit.NSButton AttrAdd { get; set; }

		[Outlet]
		public AppKit.NSButton PropApply { get; set; }

		[Outlet]
		public AppKit.NSButton PropReset { get; set; }

		[Outlet]
		public AppKit.NSTableView PropTableView { get; set; }

		[Action ("AttrAddClick:")]
		partial void AttrAddClick (Foundation.NSObject sender);

		[Action ("PropApplyClick:")]
		partial void PropApplyClick (Foundation.NSObject sender);

		[Action ("PropResetClick:")]
		partial void PropResetClick (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (AttrAdd != null) {
				AttrAdd.Dispose ();
				AttrAdd = null;
			}

			if (PropApply != null) {
				PropApply.Dispose ();
				PropApply = null;
			}

			if (PropReset != null) {
				PropReset.Dispose ();
				PropReset = null;
			}

			if (PropTableView != null) {
				PropTableView.Dispose ();
				PropTableView = null;
			}
		}
	}
}

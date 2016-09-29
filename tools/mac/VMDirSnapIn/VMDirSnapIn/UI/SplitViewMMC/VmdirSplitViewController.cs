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
using System;
using Foundation;
using VmIdentity.UI.Common.Utilities;

namespace VMDirSnapIn.UI
{
	public partial class VmdirSplitViewController : AppKit.NSViewController
	{
		public PropertiesViewController propViewController { get; set; }
		#region Constructors

		// Called when created from unmanaged code
		public VmdirSplitViewController(IntPtr handle) : base(handle)
		{
			Initialize();
		}

		// Called when created directly from a XIB file
		[Export("initWithCoder:")]
		public VmdirSplitViewController(NSCoder coder) : base(coder)
		{
			Initialize();
		}

		// Call to load from the XIB/NIB file
		public VmdirSplitViewController() : base("VmdirSplitView", NSBundle.MainBundle)
		{
			Initialize();
		}

		// Shared initialization code
		void Initialize()
		{
		}

		#endregion

		//strongly typed view accessor
		public new VmdirSplitView View
		{
			get
			{
				return (VmdirSplitView)base.View;
			}
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			propViewController = new PropertiesViewController();
			propViewController.PropTableView = new VMDirTableView();
			VmdirPropView.AddSubview(propViewController.View);
		}

		partial void OnClickAction(NSObject sender)
		{
			UIErrorHelper.ShowWarning(VmdirOutlineView.SelectedTag.ToString());
		}
	}
}

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
using System.Collections.Generic;
using System.Linq;
using Foundation;
using AppKit;

using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace RestSsoAdminSnapIn
{
	public partial class AddExistingTenantController : AppKit.NSWindowController
	{
		public TenantDto TenantDto { get; private set; }

		#region Constructors

		// Called when created from unmanaged code
		public AddExistingTenantController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public AddExistingTenantController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public AddExistingTenantController () : base ("AddExistingTenant")
		{
			Initialize ();
		}
		
		// Shared initialization code
		void Initialize ()
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();

			//Events
			this.BtnOk.Activated += OnClickOkButton;
			this.BtnCancel.Activated += OnClickCancelButton;
		}
		public void OnClickOkButton (object sender, EventArgs e)
		{
			if (string.IsNullOrEmpty (TXTTenantName.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid tenant name", "Alert");
			} else {
				TenantDto = new TenantDto{ Name = TXTTenantName.StringValue };
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (1);
			}
		}

		public void OnClickCancelButton (object sender, EventArgs e)
		{
			this.Close ();
			NSApplication.SharedApplication.StopModalWithCode(0);
		}
		#endregion

		//strongly typed window accessor
		public new AddExistingTenant Window {
			get {
				return (AddExistingTenant)base.Window;
			}
		}
	}
}


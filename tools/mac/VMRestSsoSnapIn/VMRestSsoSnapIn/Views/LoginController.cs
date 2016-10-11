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

using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace RestSsoAdminSnapIn
{
	public partial class LoginController : AppKit.NSWindowController
	{
		public LoginDto LoginDto{ get; private set; }
		public ServerDto ServerDto{ get; set; }
		public string Username { get; set; }
		#region Constructors

		// Called when created from unmanaged code
		public LoginController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public LoginController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public LoginController () : base ("Login")
		{
			Initialize ();
		}
		
		// Shared initialization code
		void Initialize ()
		{
		}

		#endregion

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();

			//Events
			this.BtnLogin.Activated += OnClickLoginButton;
			this.BtnClose.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};
			TxtUsername.StringValue = string.IsNullOrEmpty(Username)? "Administrator" : Username;
		}

		public void OnClickLoginButton (object sender, EventArgs e)
		{
			if (string.IsNullOrEmpty (TxtUsername.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid username", "Alert");
			} else if (string.IsNullOrEmpty (TxtPassword.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid password", "Alert");
			} else {
				 LoginDto = new LoginDto () {
					User = TxtUsername.StringValue,
					Pass = TxtPassword.StringValue,
					TenantName = ServerDto.Tenant,
					DomainName = ServerDto.Tenant
				};
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode(1);
			}
		}
		//strongly typed window accessor
		public new Login Window {
			get {
				return (Login)base.Window;
			}
		}
	}
}


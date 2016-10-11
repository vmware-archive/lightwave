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
	public partial class AddUserController : AppKit.NSWindowController
	{
		public UserDto UserDto;

		#region Constructors

		// Called when created from unmanaged code
		public AddUserController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public AddUserController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public AddUserController () : base ("AddUser")
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
			this.BtnAdd.Activated += OnClickAddButton;
			this.BtnClose.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};
		}

		public void OnClickAddButton (object sender, EventArgs e)
		{
			if (string.IsNullOrEmpty (TxtUserName.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid username", "Alert");
			} else if (string.IsNullOrEmpty (TxtPassword.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid Password", "Alert");
			} else if (string.IsNullOrEmpty (TxtConfirmPassword.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid Confirm Password", "Alert");
			} else if (TxtPassword.StringValue != TxtConfirmPassword.StringValue) {
				UIErrorHelper.ShowAlert ("Password and confirm password values do not match", "Alert");
			} else {
				UserDto = new UserDto () {
					Name = TxtUserName.StringValue,
					PersonDetails = new PersonUserDto { 
						FirstName = TxtFirstName.StringValue,
						LastName = TxtLastName.StringValue,
						EmailAddress = TxtEmail.StringValue,
						Description = TxtDescription.StringValue
					},
					PasswordDetails = new PasswordDetailsDto {
						Password = TxtPassword.StringValue
					},
					Disabled = chkDisabled.StringValue == "1"
				};
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (1);
			}
		}

		#endregion

		//strongly typed window accessor
		public new AddUser Window {
			get {
				return (AddUser)base.Window;
			}
		}
	}
}


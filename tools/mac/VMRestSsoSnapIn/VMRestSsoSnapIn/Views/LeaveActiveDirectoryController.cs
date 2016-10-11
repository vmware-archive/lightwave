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
using System.Security.Cryptography.X509Certificates;
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace RestSsoAdminSnapIn
{
	public partial class LeaveActiveDirectoryController : AppKit.NSWindowController
	{
		public ActiveDirectoryJoinInfoDto ActiveDirectoryJoinInfoDto;
		public string Server;

		#region Constructors

		// Called when created from unmanaged code
		public LeaveActiveDirectoryController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public LeaveActiveDirectoryController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public LeaveActiveDirectoryController () : base ("LeaveActiveDirectory")
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
			this.BtnLeave.Activated += OnClickAddButton;
			this.BtnClose.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};

			TxtDomain.StringValue = ActiveDirectoryJoinInfoDto.Name;
		}

		public void OnClickAddButton (object sender, EventArgs e)
		{
			var activeDirectoryJoinRequestDto = new ActiveDirectoryJoinRequestDto () {
				Username = TxtUsername.StringValue,
				Password = TxtPassword.StringValue,
				Domain = TxtDomain.StringValue,
				OrganizationalUnit = TxtOU.StringValue
			};
			var success = LeaveActiveDirectory (activeDirectoryJoinRequestDto);
			if (success) {
				UIErrorHelper.ShowAlert ("AD leave operation was succesful. Please reboot the node.", "Information");
			} else {
				UIErrorHelper.ShowAlert ("AD leave operation failed.", "Information");
				return;
			}
			this.Close ();
			NSApplication.SharedApplication.StopModalWithCode (1);
			
		}

		#endregion

		//strongly typed window accessor
		public new LeaveActiveDirectory Window {
			get {
				return (LeaveActiveDirectory)base.Window;
			}
		}

		private bool LeaveActiveDirectory (ActiveDirectoryJoinRequestDto dto)
		{
			bool success = false;
			ActionHelper.Execute (delegate() {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (Server);
				var credentialsDto = new CredentialsDto { Username = dto.Username, Password = dto.Password };
				success = SnapInContext.Instance.ServiceGateway.Adf.LeaveActiveDirectory (auth.ServerDto, credentialsDto, auth.Token);
			});
			return success;
		}
	}
}


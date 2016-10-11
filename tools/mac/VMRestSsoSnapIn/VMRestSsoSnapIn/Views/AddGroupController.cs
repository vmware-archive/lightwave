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
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace RestSsoAdminSnapIn
{
	public partial class AddGroupController : AppKit.NSWindowController
	{
		public GroupDto GroupDto;

		#region Constructors

		// Called when created from unmanaged code
		public AddGroupController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public AddGroupController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public AddGroupController () : base ("AddGroup")
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
			if (string.IsNullOrEmpty (TxtGroupName.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid group name", "Alert");
			} else if (string.IsNullOrEmpty (TxtDescription.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid Description", "Alert");
			} else {
				
				GroupDto = new GroupDto () {
					GroupName = TxtGroupName.StringValue,
					GroupDetails =  new GroupDetailsDto { Description = TxtDescription.StringValue }
				};
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (1);
			}
		}

		#endregion

		//strongly typed window accessor
		public new AddGroup Window {
			get {
				return (AddGroup)base.Window;
			}
		}
	}
}


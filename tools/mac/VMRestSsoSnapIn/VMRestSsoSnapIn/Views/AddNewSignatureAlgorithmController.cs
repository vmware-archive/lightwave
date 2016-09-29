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
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;


namespace RestSsoAdminSnapIn
{
	public partial class AddNewSignatureAlgorithmController : NSWindowController
	{
		public SignatureAlgorithmDto SignatureAlgorithmDto { get; set;}
		public bool IsUpdated { get; private set;}

		public AddNewSignatureAlgorithmController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public AddNewSignatureAlgorithmController (NSCoder coder) : base (coder)
		{
		}

		public AddNewSignatureAlgorithmController () : base ("AddNewSignatureAlgorithm")
		{
		}

		public new AddNewSignatureAlgorithm Window {
			get { return (AddNewSignatureAlgorithm)base.Window; }
		}
		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			IsUpdated = false;

			//Events
			this.BtnAdd.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtMaxKeySize.StringValue))

				{
					UIErrorHelper.ShowAlert ("Max Key size has invalid value", "Alert");
				} else if(string.IsNullOrEmpty(TxtMinKeySize.StringValue))
				{
					UIErrorHelper.ShowAlert ("Min Key size has invalid value", "Alert");
				} else if(string.IsNullOrEmpty(TxtPriority.StringValue))
				{
					UIErrorHelper.ShowAlert ("Priority has invalid value", "Alert");
				}
				else
				{
					SignatureAlgorithmDto = new SignatureAlgorithmDto
					{
						MaxKeySize = TxtMaxKeySize.IntValue,
						MinKeySize = TxtMinKeySize.IntValue,
						Priority = TxtPriority.IntValue,
					};
					IsUpdated = true;
					this.Close ();
					NSApplication.SharedApplication.StopModalWithCode (0);
				}
			};
			this.BtnClose.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};


			if (SignatureAlgorithmDto != null) {
				TxtMaxKeySize.IntValue = SignatureAlgorithmDto.MaxKeySize;
				TxtMinKeySize.IntValue = SignatureAlgorithmDto.MinKeySize;
				TxtPriority.IntValue = SignatureAlgorithmDto.Priority;
			}
		}
	}
}

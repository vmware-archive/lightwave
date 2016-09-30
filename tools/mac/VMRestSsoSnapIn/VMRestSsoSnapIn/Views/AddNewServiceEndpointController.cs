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
	public partial class AddNewServiceEndpointController : NSWindowController
	{
		public ServiceEndpointDto ServiceEndpointDto { get; set;}
		public bool IsUpdated { get; private set;}

		public AddNewServiceEndpointController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public AddNewServiceEndpointController (NSCoder coder) : base (coder)
		{
		}

		public AddNewServiceEndpointController () : base ("AddNewServiceEndpoint")
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			IsUpdated = false;

			//Events
			this.BtnAdd.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtName.StringValue))
				{
					UIErrorHelper.ShowAlert ("Name has invalid value", "Alert");
				} else if(string.IsNullOrEmpty(TxtBinding.StringValue))
				{
					UIErrorHelper.ShowAlert ("Binding has invalid value", "Alert");
				} else if(string.IsNullOrEmpty(TxtEndpoint.StringValue))
				{
					UIErrorHelper.ShowAlert ("Endpoint has invalid value", "Alert");
				} 
				else
				{
					ServiceEndpointDto = new ServiceEndpointDto
					{
						Name = TxtName.StringValue,
						Binding = TxtBinding.StringValue,
						Endpoint = TxtEndpoint.StringValue
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

			if (ServiceEndpointDto != null) {
				TxtName.StringValue = ServiceEndpointDto.Name;
				TxtEndpoint.StringValue = ServiceEndpointDto.Endpoint;
				TxtBinding.StringValue = ServiceEndpointDto.Binding;
			}
		}

		public new AddNewServiceEndpoint Window {
			get { return (AddNewServiceEndpoint)base.Window; }
		}
	}
}

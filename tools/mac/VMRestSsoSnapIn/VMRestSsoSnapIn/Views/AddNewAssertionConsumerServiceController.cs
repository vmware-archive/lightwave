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
	public partial class AddNewAssertionConsumerServiceController : NSWindowController
	{
		public AssertionConsumerServiceDto AssertionConsumerServiceDto { get; set;}
		public bool DefaultSet { get; set; }
		public bool IsUpdated { get; private set;}
		public AddNewAssertionConsumerServiceController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public AddNewAssertionConsumerServiceController (NSCoder coder) : base (coder)
		{
		}

		public AddNewAssertionConsumerServiceController () : base ("AddNewAssertionConsumerService")
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
				} else if(string.IsNullOrEmpty(TxtEndpoint.StringValue))
				{
					UIErrorHelper.ShowAlert ("Endpoint has invalid value", "Alert");
				} else if(string.IsNullOrEmpty(TxtBinding.StringValue))
				{
					UIErrorHelper.ShowAlert ("Binding has invalid value", "Alert");
				} else if(string.IsNullOrEmpty(TxtIndex.StringValue))
				{
					UIErrorHelper.ShowAlert ("Index has invalid value", "Alert");
				} else if(ChkDefault.StringValue == "1" && DefaultSet && (AssertionConsumerServiceDto != null && !AssertionConsumerServiceDto.IsDefault))
				{
					UIErrorHelper.ShowAlert ("Multiple assertion consumer services chosen as default", "Alert");
				}
				else
				{
					AssertionConsumerServiceDto = new AssertionConsumerServiceDto
					{
						Name = TxtName.StringValue,
						Endpoint = TxtEndpoint.StringValue,
						Binding = TxtBinding.StringValue,
						Index = TxtIndex.IntValue,
						IsDefault = ChkDefault.StringValue == "1"
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

			if (AssertionConsumerServiceDto != null) {
				TxtIndex.IntValue = AssertionConsumerServiceDto.Index;
				TxtName.StringValue = AssertionConsumerServiceDto.Name;
				TxtEndpoint.StringValue = AssertionConsumerServiceDto.Endpoint;
				TxtBinding.StringValue = AssertionConsumerServiceDto.Binding;
			}
		}

		public new AddNewAssertionConsumerService Window {
			get { return (AddNewAssertionConsumerService)base.Window; }
		}
	}
}

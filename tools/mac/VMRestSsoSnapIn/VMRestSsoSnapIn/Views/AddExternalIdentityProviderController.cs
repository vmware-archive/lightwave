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
 
using AppKit;
using Foundation;
using System;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace RestSsoAdminSnapIn
{
	public partial class AddExternalIdentityProviderController : NSWindowController
	{
		public ExternalIdentityProviderDto ExternalIdentityProviderDto { get; set; }

		public AddExternalIdentityProviderController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public AddExternalIdentityProviderController (NSCoder coder) : base (coder)
		{
		}

		public AddExternalIdentityProviderController () : base ("AddExternalIdentityProvider")
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
		}

		public new AddExternalIdentityProvider Window {
			get { return (AddExternalIdentityProvider)base.Window; }
		}
	}
}

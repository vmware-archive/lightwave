/*
Copyright 2015 VMware, Inc. All rights reserved.
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

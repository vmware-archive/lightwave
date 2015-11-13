/*
Copyright 2015 VMware, Inc. All rights reserved.
*/
using System;
using Foundation;
using AppKit;

namespace RestSsoAdminSnapIn
{
	public partial class AddExternalIdentityProvider : NSWindow
	{
		public AddExternalIdentityProvider (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public AddExternalIdentityProvider (NSCoder coder) : base (coder)
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
		}
		[Export ("windowWillClose:")]
		public void WindowWillClose (NSNotification notification)
		{
			NSApplication.SharedApplication.StopModalWithCode (0);
		}
	}
}

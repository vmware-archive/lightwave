// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMDirSnapIn.UI
{
	[Register ("ConnectToLdapWindowController")]
	partial class ConnectToLdapWindowController
	{
		[Outlet]
		AppKit.NSTextField BaseDN { get; set; }

		[Outlet]
		public AppKit.NSTextField BindDN { get; private set; }

		[Outlet]
		public AppKit.NSButton CancelButton { get; private set; }

		[Outlet]
		public AppKit.NSButton OKButton { get; private set; }

		[Outlet]
		public AppKit.NSSecureTextField Password { get; private set; }

		[Outlet]
		public AppKit.NSTextField ServerName { get; private set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BaseDN != null) {
				BaseDN.Dispose ();
				BaseDN = null;
			}

			if (BindDN != null) {
				BindDN.Dispose ();
				BindDN = null;
			}

			if (CancelButton != null) {
				CancelButton.Dispose ();
				CancelButton = null;
			}

			if (OKButton != null) {
				OKButton.Dispose ();
				OKButton = null;
			}

			if (Password != null) {
				Password.Dispose ();
				Password = null;
			}

			if (ServerName != null) {
				ServerName.Dispose ();
				ServerName = null;
			}
		}
	}

	[Register ("ConnectToLdapWindow")]
	partial class ConnectToLdapWindow
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

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
	[Register ("WelcomeScreenController")]
	partial class WelcomeScreenController
	{
		[Outlet]
		public AppKit.NSPopUpButton ConnectToServer { get; private set; }

		[Action ("OnConnect:")]
		partial void OnConnect (Foundation.NSObject sender);
		
		void ReleaseDesignerOutlets ()
		{
			if (ConnectToServer != null) {
				ConnectToServer.Dispose ();
				ConnectToServer = null;
			}
		}
	}
}

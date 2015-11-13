// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMCASnapIn.UI
{
    [Register ("AppDelegate")]
    partial class AppDelegate
    {
        [Outlet]
        public AppKit.NSMenuItem OpenConnectionMenuItem { get; set; }

        [Action ("OpenConnection:")]
        partial void OpenConnection (Foundation.NSObject sender);

        void ReleaseDesignerOutlets ()
        {
            if (OpenConnectionMenuItem != null) {
                OpenConnectionMenuItem.Dispose ();
                OpenConnectionMenuItem = null;
            }
        }
    }
}

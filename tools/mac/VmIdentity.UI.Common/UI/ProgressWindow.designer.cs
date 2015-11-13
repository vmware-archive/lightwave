// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VmIdentity.UI.Common
{
    [Register ("ProgressWindowController")]
    partial class ProgressWindowController
    {
        [Outlet]
        public AppKit.NSProgressIndicator ProgressBar { get; set; }

        [Outlet]
        public AppKit.NSTextField ProgressLabel { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (ProgressBar != null) {
                ProgressBar.Dispose ();
                ProgressBar = null;
            }

            if (ProgressLabel != null) {
                ProgressLabel.Dispose ();
                ProgressLabel = null;
            }
        }
    }

    [Register ("ProgressWindow")]
    partial class ProgressWindow
    {
		
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

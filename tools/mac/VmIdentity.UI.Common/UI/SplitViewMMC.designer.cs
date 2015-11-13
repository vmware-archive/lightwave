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
    [Register ("SplitViewMMCController")]
    partial class SplitViewMMCController
    {
        [Outlet]
        public AppKit.NSView DetailView { get; set; }

        [Outlet]
        public AppKit.NSOutlineView MainOutlineView { get; set; }

        [Outlet]
        public AppKit.NSTableView MainTableView { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (DetailView != null) {
                DetailView.Dispose ();
                DetailView = null;
            }

            if (MainOutlineView != null) {
                MainOutlineView.Dispose ();
                MainOutlineView = null;
            }

            if (MainTableView != null) {
                MainTableView.Dispose ();
                MainTableView = null;
            }
        }
    }

    [Register ("SplitViewMMC")]
    partial class SplitViewMMC
    {
		
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

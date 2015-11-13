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
    [Register ("ServerViewController")]
    partial class ServerViewController
    {
        [Outlet]
        public AppKit.NSTextField DomainControllerLabelValue { get; set; }

        [Outlet]
        public AppKit.NSImageView ImageView { get; set; }

        [Outlet]
        public AppKit.NSTextField ReplLabelValue { get; set; }

        [Outlet]
        public AppKit.NSTextField ServerName { get; set; }

        [Outlet]
        public VmIdentity.UI.Common.ServerView serverView { get; set; }

        [Outlet]
        public AppKit.NSTextField SitesLabelValue { get; set; }

        void ReleaseDesignerOutlets ()
        {
            if (DomainControllerLabelValue != null) {
                DomainControllerLabelValue.Dispose ();
                DomainControllerLabelValue = null;
            }

            if (ImageView != null) {
                ImageView.Dispose ();
                ImageView = null;
            }

            if (ReplLabelValue != null) {
                ReplLabelValue.Dispose ();
                ReplLabelValue = null;
            }

            if (ServerName != null) {
                ServerName.Dispose ();
                ServerName = null;
            }

            if (serverView != null) {
                serverView.Dispose ();
                serverView = null;
            }

            if (SitesLabelValue != null) {
                SitesLabelValue.Dispose ();
                SitesLabelValue = null;
            }
        }
    }

    [Register ("ServerView")]
    partial class ServerView
    {
		
        void ReleaseDesignerOutlets ()
        {
        }
    }
}

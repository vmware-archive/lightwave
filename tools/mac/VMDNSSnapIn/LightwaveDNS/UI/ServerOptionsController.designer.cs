// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMDNS
{
    [Register("ServerOptionsController")]
    partial class ServerOptionsController
    {
        [Outlet]
        public AppKit.NSButton AddButton { get; private set; }

        [Outlet]
        AppKit.NSButton DeleteButton { get; set; }

        [Outlet]
        AppKit.NSButton DownButton { get; set; }

        [Outlet]
        public AppKit.NSTextField ForwarderIPField { get; private set; }

        [Outlet]
        public AppKit.NSTableView ForwardersTableView { get; private set; }

        [Outlet]
        AppKit.NSTabViewItem ForwardersTabView { get; set; }

        [Outlet]
        AppKit.NSButton UpButton { get; set; }

        [Action("AddForwarder:")]
        partial void AddForwarder(Foundation.NSObject sender);

        [Action("ApplyChanges:")]
        partial void ApplyChanges(Foundation.NSObject sender);

        [Action("Cancel:")]
        partial void Cancel(Foundation.NSObject sender);

        [Action("DeleteForwarder:")]
        partial void DeleteForwarder(Foundation.NSObject sender);

        [Action("MoveForwarderDown:")]
        partial void MoveForwarderDown(Foundation.NSObject sender);

        [Action("MoveForwarderUp:")]
        partial void MoveForwarderUp(Foundation.NSObject sender);

        [Action("OnClose:")]
        partial void OnClose(Foundation.NSObject sender);

        void ReleaseDesignerOutlets()
        {
            if (DeleteButton != null)
            {
                DeleteButton.Dispose();
                DeleteButton = null;
            }

            if (AddButton != null)
            {
                AddButton.Dispose();
                AddButton = null;
            }

            if (DownButton != null)
            {
                DownButton.Dispose();
                DownButton = null;
            }

            if (ForwarderIPField != null)
            {
                ForwarderIPField.Dispose();
                ForwarderIPField = null;
            }

            if (ForwardersTableView != null)
            {
                ForwardersTableView.Dispose();
                ForwardersTableView = null;
            }

            if (ForwardersTabView != null)
            {
                ForwardersTabView.Dispose();
                ForwardersTabView = null;
            }

            if (UpButton != null)
            {
                UpButton.Dispose();
                UpButton = null;
            }
        }
    }
}

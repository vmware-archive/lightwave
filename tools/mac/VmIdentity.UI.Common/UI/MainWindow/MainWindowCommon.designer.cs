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
	[Register ("MainWindowCommonController")]
	partial class MainWindowCommonController
	{
		[Outlet]
		public VmIdentity.UI.Common.ActivatableToolBarItem BackForwardToolBarItem { get; private set; }

		[Outlet]
		public AppKit.NSView ContainerView { get; private set; }

		[Outlet]
		public AppKit.NSTextField LoggedInLabel { get; private set; }

		[Outlet]
		AppKit.NSTextField NetworkStatus { get; set; }

		[Outlet]
		public VmIdentity.UI.Common.ActivatableToolBarItem RefreshToolBarItem { get; private set; }

		[Outlet]
		public AppKit.NSSearchFieldCell SearchFieldCell { get; private set; }

		[Outlet]
		public AppKit.NSSearchField SearchRecordsField { get; private set; }

		[Outlet]
		public VmIdentity.UI.Common.ActivatableToolBarItem SearchToolBarItem { get; private set; }

		[Outlet]
		public VmIdentity.UI.Common.ActivatableToolBarItem ServerToolBarItem { get; private set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (BackForwardToolBarItem != null) {
				BackForwardToolBarItem.Dispose ();
				BackForwardToolBarItem = null;
			}

			if (ContainerView != null) {
				ContainerView.Dispose ();
				ContainerView = null;
			}

			if (LoggedInLabel != null) {
				LoggedInLabel.Dispose ();
				LoggedInLabel = null;
			}

			if (RefreshToolBarItem != null) {
				RefreshToolBarItem.Dispose ();
				RefreshToolBarItem = null;
			}

			if (SearchFieldCell != null) {
				SearchFieldCell.Dispose ();
				SearchFieldCell = null;
			}

			if (SearchRecordsField != null) {
				SearchRecordsField.Dispose ();
				SearchRecordsField = null;
			}

			if (SearchToolBarItem != null) {
				SearchToolBarItem.Dispose ();
				SearchToolBarItem = null;
			}

			if (ServerToolBarItem != null) {
				ServerToolBarItem.Dispose ();
				ServerToolBarItem = null;
			}

			if (NetworkStatus != null) {
				NetworkStatus.Dispose ();
				NetworkStatus = null;
			}
		}
	}

	[Register ("MainWindowCommon")]
	partial class MainWindowCommon
	{
		
		void ReleaseDesignerOutlets ()
		{
		}
	}
}

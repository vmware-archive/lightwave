// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMPSCHighAvailability.UI
{
	[Register ("MonitorController")]
	partial class MonitorController
	{
		[Outlet]
		AppKit.NSButton AutoRefreshButton { get; set; }

		[Outlet]
		AppKit.NSView ContentPanel { get; set; }

		[Outlet]
		AppKit.NSTextField CurrentStatusTextField { get; set; }

		[Outlet]
		AppKit.NSTextField DomainControllerTextField { get; set; }

		[Outlet]
		AppKit.NSTextField HealthtextField { get; set; }

		[Outlet]
		AppKit.NSTextField HostnameHeader { get; set; }

		[Outlet]
		AppKit.NSTextField HostnameServiceHeaderTextField { get; set; }

		[Outlet]
		AppKit.NSComboBox IntervalComboBox { get; set; }

		[Outlet]
		AppKit.NSTextField LastRefreshTextField { get; set; }

		[Outlet]
		public AppKit.NSTableView PscTableView { get; private set; }

		[Outlet]
		AppKit.NSButton RefreshButton { get; set; }

		[Outlet]
		AppKit.NSTextField ServicesheaderTextField { get; set; }

		[Outlet]
		AppKit.NSTableView ServicesTableView { get; set; }

		[Outlet]
		AppKit.NSButton SiteAffinityButton { get; set; }

		[Outlet]
		AppKit.NSTextField SitenameTextField { get; set; }

		[Outlet]
		AppKit.NSTextField StatusTextField { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (SitenameTextField != null) {
				SitenameTextField.Dispose ();
				SitenameTextField = null;
			}

			if (AutoRefreshButton != null) {
				AutoRefreshButton.Dispose ();
				AutoRefreshButton = null;
			}

			if (ContentPanel != null) {
				ContentPanel.Dispose ();
				ContentPanel = null;
			}

			if (CurrentStatusTextField != null) {
				CurrentStatusTextField.Dispose ();
				CurrentStatusTextField = null;
			}

			if (DomainControllerTextField != null) {
				DomainControllerTextField.Dispose ();
				DomainControllerTextField = null;
			}

			if (HealthtextField != null) {
				HealthtextField.Dispose ();
				HealthtextField = null;
			}

			if (HostnameHeader != null) {
				HostnameHeader.Dispose ();
				HostnameHeader = null;
			}

			if (HostnameServiceHeaderTextField != null) {
				HostnameServiceHeaderTextField.Dispose ();
				HostnameServiceHeaderTextField = null;
			}

			if (IntervalComboBox != null) {
				IntervalComboBox.Dispose ();
				IntervalComboBox = null;
			}

			if (LastRefreshTextField != null) {
				LastRefreshTextField.Dispose ();
				LastRefreshTextField = null;
			}

			if (PscTableView != null) {
				PscTableView.Dispose ();
				PscTableView = null;
			}

			if (RefreshButton != null) {
				RefreshButton.Dispose ();
				RefreshButton = null;
			}

			if (ServicesheaderTextField != null) {
				ServicesheaderTextField.Dispose ();
				ServicesheaderTextField = null;
			}

			if (ServicesTableView != null) {
				ServicesTableView.Dispose ();
				ServicesTableView = null;
			}

			if (SiteAffinityButton != null) {
				SiteAffinityButton.Dispose ();
				SiteAffinityButton = null;
			}

			if (StatusTextField != null) {
				StatusTextField.Dispose ();
				StatusTextField = null;
			}
		}
	}
}

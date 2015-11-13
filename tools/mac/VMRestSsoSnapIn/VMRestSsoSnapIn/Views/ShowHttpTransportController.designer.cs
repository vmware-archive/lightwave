// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace RestSsoAdminSnapIn
{
	[Register ("ShowHttpTransportController")]
	partial class ShowHttpTransportController
	{
		[Outlet]
		AppKit.NSTableView HttpTransportTableView { get; set; }

		[Outlet]
		AppKit.NSTextField TxtContentLength { get; set; }

		[Outlet]
		AppKit.NSTextField TxtContentType { get; set; }

		[Outlet]
		AppKit.NSTextView TxtError { get; set; }

		[Outlet]
		AppKit.NSTextField TxtHttpMethod { get; set; }

		[Outlet]
		AppKit.NSTextField TxtReponseContentType { get; set; }

		[Outlet]
		AppKit.NSTextView TxtRequestData { get; set; }

		[Outlet]
		AppKit.NSTextField TxtRequestUri { get; set; }

		[Outlet]
		AppKit.NSTextField TxtResponseContentLength { get; set; }

		[Outlet]
		AppKit.NSTextView TxtResponseData { get; set; }

		[Outlet]
		AppKit.NSTextField TxtServerName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTimestamp { get; set; }

		[Outlet]
		AppKit.NSTextField TxtTimeTaken { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUserAgent { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (HttpTransportTableView != null) {
				HttpTransportTableView.Dispose ();
				HttpTransportTableView = null;
			}

			if (TxtContentLength != null) {
				TxtContentLength.Dispose ();
				TxtContentLength = null;
			}

			if (TxtContentType != null) {
				TxtContentType.Dispose ();
				TxtContentType = null;
			}

			if (TxtError != null) {
				TxtError.Dispose ();
				TxtError = null;
			}

			if (TxtHttpMethod != null) {
				TxtHttpMethod.Dispose ();
				TxtHttpMethod = null;
			}

			if (TxtReponseContentType != null) {
				TxtReponseContentType.Dispose ();
				TxtReponseContentType = null;
			}

			if (TxtRequestData != null) {
				TxtRequestData.Dispose ();
				TxtRequestData = null;
			}

			if (TxtRequestUri != null) {
				TxtRequestUri.Dispose ();
				TxtRequestUri = null;
			}

			if (TxtResponseContentLength != null) {
				TxtResponseContentLength.Dispose ();
				TxtResponseContentLength = null;
			}

			if (TxtResponseData != null) {
				TxtResponseData.Dispose ();
				TxtResponseData = null;
			}

			if (TxtServerName != null) {
				TxtServerName.Dispose ();
				TxtServerName = null;
			}

			if (TxtTimeTaken != null) {
				TxtTimeTaken.Dispose ();
				TxtTimeTaken = null;
			}

			if (TxtUserAgent != null) {
				TxtUserAgent.Dispose ();
				TxtUserAgent = null;
			}

			if (TxtTimestamp != null) {
				TxtTimestamp.Dispose ();
				TxtTimestamp = null;
			}
		}
	}
}

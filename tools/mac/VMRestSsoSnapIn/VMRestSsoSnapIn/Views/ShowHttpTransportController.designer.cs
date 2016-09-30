/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
 
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

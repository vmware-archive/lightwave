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

using System;
using System.Collections.Generic;
using System.Linq;
using AppKit;
using Foundation;

namespace VMPSCHighAvailability.UI
{
	/// <summary>
	/// Monitor.
	/// </summary>
	public partial class Monitor : AppKit.NSView
	{
		#region Constructors

		// Called when created from unmanaged code
		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.Monitor"/> class.
		/// </summary>
		/// <param name="handle">Handle.</param>
		public Monitor (IntPtr handle) : base (handle)
		{
		}

		// Called when created directly from a XIB file
		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.Monitor"/> class.
		/// </summary>
		/// <param name="coder">Coder.</param>
		[Export ("initWithCoder:")]
		public Monitor (NSCoder coder) : base (coder)
		{
		}

		#endregion
	}
}

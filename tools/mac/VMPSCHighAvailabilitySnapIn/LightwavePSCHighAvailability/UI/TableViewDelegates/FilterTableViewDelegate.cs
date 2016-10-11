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
using Foundation;
using AppKit;
using VmIdentity.UI.Common;
using VMAFD.Client;
using VMPSCHighAvailability.Common.DTO;
using VMPSCHighAvailability.Common.Helpers;
using VMPSCHighAvailability.DataSources;
using VmIdentity.UI.Common.Utilities;
using System.Threading.Tasks;

namespace VMPSCHighAvailability.UI
{
	/// <summary>
	/// Filter table view delegate.
	/// </summary>
	public class FilterTableViewDelegate : NSTableViewDelegate
	{
		/// <summary>
		/// The controller.
		/// </summary>
		private FilterCriteriaController _controller;

		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.ServicesTableViewDelegate"/> class.
		/// </summary>
		/// <param name="controller">Controller.</param>
		public FilterTableViewDelegate (FilterCriteriaController controller)
		{
			_controller = controller;
		}
	}
}


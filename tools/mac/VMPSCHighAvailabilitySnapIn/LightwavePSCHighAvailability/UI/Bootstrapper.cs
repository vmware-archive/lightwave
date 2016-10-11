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
using Foundation;
using AppKit;
using VmIdentity.UI.Common;

namespace VMPSCHighAvailability.UI
{
	/// <summary>
	/// Encapsulates the operations to be performed when the Application starts, stops or disposes.
	/// </summary>
	public class Bootstrapper
	{
		/// <summary>
		/// Initial sequence of steps to be called during application start
		/// </summary>
		/// <param name="appDelegate">App delegate.</param>
		public static void Init(AppDelegate appDelegate)
		{
			var welcomeScreenController = new WelcomeScreenCommonController ();
			WelcomeScreenHelper.SetDisplayElements (welcomeScreenController);
			nint ret = NSApplication.SharedApplication.RunModalForWindow (welcomeScreenController.Window);

			if((nint)VMIdentityConstants.DIALOGOK == ret)
			{
				var _factory = new VMPSCHighAvailabilityMainWindowControllerFactory ();
				var mainWindowController = _factory.GetMainWindowController () as VMPSCHighAvailabilityMainWindowController;
				mainWindowController.Window.MakeKeyAndOrderFront (appDelegate);
			}
		}

		/// <summary>
		/// Cleanup steps to be performed before the application stops/disposes.
		/// </summary>
		public void Cleanup()
		{
			//todo: Cleanup activities before the tool exits
		}
	}
}


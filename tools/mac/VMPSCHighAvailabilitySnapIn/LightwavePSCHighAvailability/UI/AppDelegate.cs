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
using AppKit;
using Foundation;
using VmIdentity.UI.Common;
using VMPSCHighAvailability.Common;

namespace VMPSCHighAvailability.UI
{
	/// <summary>
	/// Application delegate
	/// </summary>
	public partial class AppDelegate : NSApplicationDelegate
	{
		/// <summary>
		/// Tracks the number of active windows for triggering the application termination.
		/// </summary>
		private uint _activeMainWindows;

		/// <summary>
		/// Initializes a new instance of the <see cref="VMPSCHighAvailability.UI.AppDelegate"/> class.
		/// </summary>
		public AppDelegate()
		{
			_activeMainWindows = 0;
			NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"CloseMainWindow", OnCloseMainWindow);
		}

		/// <summary>
		/// Sets the welcome screen text.
		/// </summary>
		/// <param name="welcomeScreenController">Welcome screen controller.</param>
		void SetWelcomeScreenText(WelcomeScreenCommonController welcomeScreenController)
		{
			welcomeScreenController.TitleDescription = Constants.WelcomeText + Constants.SuiteName + " "+Constants.ToolName;
			welcomeScreenController.Description1 =  Constants.FeatureLine1;
			welcomeScreenController.Description2 = Constants.FeatureLine2;
			welcomeScreenController.Description3 = Constants.FeatureLine3;
		}

		/// <summary>
		/// Raises the open connection event.
		/// </summary>
		/// <param name="sender">Sender.</param>
		partial void OnOpenConnection (Foundation.NSObject sender)
		{
			var window = NSApplication.SharedApplication.KeyWindow;
			LoadMainWindow();
		}

		/// <summary>
		/// Does launching on finish.
		/// </summary>
		/// <param name="notification">Notification.</param>
		public override void DidFinishLaunching(NSNotification notification)
		{
			OnFinishLaunching ();
		}

		/// <summary>
		/// Raises the finish launching event.
		/// </summary>
		public void OnFinishLaunching()
		{
			var welcomeScreenController = new WelcomeScreenCommonController();
			SetWelcomeScreenText(welcomeScreenController);
			NSApplication.SharedApplication.RunModalForWindow(welcomeScreenController.Window);
			LoadMainWindow ();
		}

		/// <summary>
		/// Loads the main window.
		/// </summary>
		public void LoadMainWindow()
		{
			var factory = new VMPSCHighAvailabilityMainWindowControllerFactory();
			var mainWindowController = factory.GetMainWindowController();
			mainWindowController.Window.MakeKeyAndOrderFront(this);
			_activeMainWindows++;
		}

		/// <summary>
		/// Raises the close main window event.
		/// </summary>
		/// <param name="notification">Notification.</param>
		public void OnCloseMainWindow(NSNotification notification)
		{
			_activeMainWindows--;
			if (_activeMainWindows <= 0)
				TerminateApplication ();
		}

		/// <summary>
		/// Terminates the application.
		/// </summary>
		public void TerminateApplication()
		{
			PscHighAvailabilityAppEnvironment.Instance.SaveLocalData ();
			NSApplication.SharedApplication.Terminate (this);
		}
	}
}


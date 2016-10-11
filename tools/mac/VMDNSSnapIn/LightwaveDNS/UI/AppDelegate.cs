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
using AppKit;
using VmIdentity.UI.Common;
using System;
using VMDNS.Common;

namespace VMDNS.UI
{
    public partial class AppDelegate : NSApplicationDelegate
    {
        VMDNSMainWindowControllerFactory _factory;
        VMDNSMainWindowController mainWindowController;
        WelcomeScreenCommonController welcomeScreenController;


        public AppDelegate()
        {
            NSNotificationCenter.DefaultCenter.AddObserver((NSString)"CloseMainWindow", OnCloseMainWindow);
        }

        void InitialiseWindowControllers()
        {
            welcomeScreenController = new WelcomeScreenCommonController();
            _factory = new VMDNSMainWindowControllerFactory();
            mainWindowController = _factory.GetMainWindowController() as VMDNSMainWindowController;
        }

        void SetWelcomeScreenText()
        {
            welcomeScreenController.TitleDescription = VMDNSConstants.WELCOME_TITLE_DESCRIPTION;
            welcomeScreenController.Description1 = VMDNSConstants.WELCOME_DESCRIPTION1;
            welcomeScreenController.Description2 = VMDNSConstants.WELCOME_DESCRIPTION2;
            welcomeScreenController.Description3 = VMDNSConstants.WELCOME_DESCRIPTION3;
        }

        public override void DidFinishLaunching(NSNotification notification)
        {
            InitialiseWindowControllers();
            SetWelcomeScreenText();

            nint ret = NSApplication.SharedApplication.RunModalForWindow(welcomeScreenController.Window);
            mainWindowController.Window.MakeKeyAndOrderFront(this);

        }

        public void OnCloseMainWindow(NSNotification notification)
        {
            TerminateApplication();
        }

        /// <summary>
        /// Terminates the application.
        /// </summary>
        public void TerminateApplication()
        {
            VMDNSSnapInEnvironment.Instance.SaveLocalData();
            NSApplication.SharedApplication.Terminate(this);
        }
    }
}


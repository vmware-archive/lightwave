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

namespace VMDirSchema.UI
{
    public partial class AppDelegate : NSApplicationDelegate
    {
        VMDirSchemaMainWindowContollerFactory _factory;
        VMDirSchemaMainWindowController mainWindowController;
        WelcomeScreenCommonController welcomeScreenController;

        public AppDelegate()
        {
            NSNotificationCenter.DefaultCenter.AddObserver((NSString)VMIdentityConstants.CLOSE_MAIN_WINDOW, OnCloseMainWindow);
        }

        void InitialiseWindowControllers()
        {
            welcomeScreenController = new WelcomeScreenCommonController();
            _factory = new VMDirSchemaMainWindowContollerFactory();
            mainWindowController = _factory.GetMainWindowController() as VMDirSchemaMainWindowController;
        }

        void SetWelcomeScreenText()
        {
            welcomeScreenController.TitleDescription = VMDirSchemaConstants.WELCOME_TITLE_DESCRIPTION;
            welcomeScreenController.Description1 = VMDirSchemaConstants.WELCOME_DESCRIPTION1;
            welcomeScreenController.Description2 = VMDirSchemaConstants.WELCOME_DESCRIPTION2;
            welcomeScreenController.Description3 = VMDirSchemaConstants.WELCOME_DESCRIPTION3;
        }

        public override void DidFinishLaunching(NSNotification notification)
        {
            InitialiseWindowControllers();
            SetWelcomeScreenText();

            NSApplication.SharedApplication.RunModalForWindow(welcomeScreenController.Window);
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
            VMDirSchemaSnapInEnvironment.Instance.SaveLocalData();
            NSApplication.SharedApplication.Terminate(this);
        }
    }
}


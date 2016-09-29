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

using AppKit;
using Foundation;
using VmIdentity.UI.Common.Utilities;

namespace VMDirSnapIn.UI
{
    public partial class AppDelegate : NSApplicationDelegate
    {
        WelcomeScreenController welcomeScreenController;

        public AppDelegate ()
        {
        }

        partial void OnOpenConnection (Foundation.NSObject sender)
        {
            var window = NSApplication.SharedApplication.KeyWindow;
            if (window == null)
                welcomeScreenController.Window.MakeKeyAndOrderFront (this);
        }

        /*public override bool ApplicationShouldHandleReopen (NSApplication sender, bool flag)
        {
            var window = NSApplication.SharedApplication.KeyWindow;
            if (window == null)
                welcomeScreenController.Window.MakeKeyAndOrderFront (this);
            return true;
        }*/

        public override void DidFinishLaunching (NSNotification notification)
        {
            welcomeScreenController = new WelcomeScreenController ();
            welcomeScreenController.Window.MakeKeyAndOrderFront (this);
        }
    }
}


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
using System.Linq;
using VMDir.Common.DTO;
using System.Collections.Generic;

namespace VMDirSnapIn.UI
{
    public partial class WelcomeScreenController : NSWindowController
    {
        public WelcomeScreenController (IntPtr handle) : base (handle)
        {
        }

        [Export ("initWithCoder:")]
        public WelcomeScreenController (NSCoder coder) : base (coder)
        {
        }

        public WelcomeScreenController () : base ("WelcomeScreen")
        {
        }

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();
            //set window background color
            this.Window.BackgroundColor = NSColor.FromSrgb (1, 1, (float)1, (float)1);
        }

		partial void OnConnect(Foundation.NSObject sender)
		{
			this.Close();
			var servers = VMDirSnapInEnvironment.Instance.LocalData.ServerList;
			if (servers == null)
				servers = new List<VMDirServerDTO>();
			MainWindowController mainWindowController = new MainWindowController(servers);
			mainWindowController.Window.MakeKeyAndOrderFront(this);
		}

        public new WelcomeScreen Window {
            get { return (WelcomeScreen)base.Window; }
        }
    }
}

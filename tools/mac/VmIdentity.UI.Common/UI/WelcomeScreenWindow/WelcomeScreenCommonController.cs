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

namespace VmIdentity.UI.Common
{
    public partial class WelcomeScreenCommonController : NSWindowController
    {
        public string[] Servers { get; set; }

        public string SelectedServer { get; set; }

        public string TitleDescription { get; set; }

        public string Description1 { get; set; }

        public string Description2 { get; set; }

        public string Description3 { get; set; }

        public WelcomeScreenCommonController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public WelcomeScreenCommonController(NSCoder coder)
            : base(coder)
        {
        }

        public WelcomeScreenCommonController()
            : base("WelcomeScreenCommon")
        {
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            //set window background color
            this.Window.BackgroundColor = NSColor.FromSrgb(1, 1, (float)1, (float)1);

            this.TitleDescriptionField.StringValue = TitleDescription;
            this.DescriptionField1.StringValue = Description1;
            this.DescriptionField2.StringValue = Description2;
            this.DescriptionField3.StringValue = Description3;


            ConnectToServer.StringValue = VMIdentityConstants.SERVER_CONNECT;

            ConnectToServer.Activated += LoginAction;
        }

        public void LoginAction(object sender, EventArgs e)
        {
            NSButton senderButton = sender as NSButton;
            if (!string.IsNullOrEmpty(senderButton.Title))
            {
                SelectedServer = senderButton.Title;
            }
            this.Close();
            NSApplication.SharedApplication.StopModalWithCode(1);

        }

        public new WelcomeScreenCommon Window
        {
            get { return (WelcomeScreenCommon)base.Window; }
        }
    }
}

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
    public partial class GenericTextViewWindowController : NSWindowController
    {
        public string TextToShow { get; set; }

        public GenericTextViewWindowController (IntPtr handle) : base (handle)
        {
        }

        [Export ("initWithCoder:")]
        public GenericTextViewWindowController (NSCoder coder) : base (coder)
        {
        }

        public GenericTextViewWindowController () : base ("GenericTextViewWindow")
        {
        }

        public GenericTextViewWindowController (string cert) : base ("GenericTextViewWindow")
        {
            TextToShow = cert;
        }

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();
            this.TextView.Value = TextToShow;
            this.OkButton.Activated += (object sender, EventArgs e) => {
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (1);
            };
        }

        public new GenericTextViewWindow Window {
            get { return (GenericTextViewWindow)base.Window; }
        }
    }
}

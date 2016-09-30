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
using VmIdentity.CommonUtils;

namespace VmIdentity.UI.Common
{
    public partial class MainWindowCommon : NSWindow
    {
        public MainWindowCommon(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]	
        public MainWindowCommon(NSCoder coder)
            : base(coder)
        {
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
        }

        [Export("windowWillClose:")]
        public void WindowWillClose(NSNotification notification)
        {
            NSNotificationCenter.DefaultCenter.PostNotificationName("CloseMainWindow", this);
        }
    }
}

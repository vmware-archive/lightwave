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
using AppKit;
using System.IO;

namespace VmIdentity.UI.Common.Utilities
{
    public static class UIHelper
    {
        public static void ShowGenericWindowAsSheet (string content, string title, NSWindow parentWindow)
        {
            UIErrorHelper.CheckedExec (delegate () {
                GenericTextViewWindowController gwc = new GenericTextViewWindowController (content);
                gwc.Window.Title = title;
                NSApplication.SharedApplication.BeginSheet (gwc.Window, parentWindow, () => {
                });

                NSApplication.SharedApplication.RunModalForWindow (gwc.Window);
                parentWindow.EndSheet (gwc.Window);
                gwc.Dispose ();
            });
        }
    }
}


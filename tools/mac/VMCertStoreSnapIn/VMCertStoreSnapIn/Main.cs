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
using VmIdentity.UI.Common;

namespace VMCertStoreSnapIn
{
    class MainClass
    {
        static void Main (string[] args)
        {
            NSApplication.Init ();
            VMCertStoreSnapInEnvironment.Instance.LoadLocalData ();
            /* Bug in Xamarin. Cant load Views referenced in external project without referencing the assembly before NSApplication.Main
            ref -https://forums.xamarin.com/discussion/1771/creating-a-custom-control-view-xamarin-mac#latest
            */
            Console.WriteLine (typeof(SplitViewMMC).Assembly);
            NSApplication.Main (args);
        }
    }
}


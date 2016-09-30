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

namespace Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions
{
    public static class DateTimeHelper
    {
        public static DateTime UnixToWindows(long unixTimestamp)
        {
            DateTime start = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
            return start.AddSeconds(unixTimestamp).ToLocalTime();
        }

        public static DateTime UnixToWindowsMilliSecs(long unixTimestamp)
        {
            DateTime start = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
            return start.AddMilliseconds(unixTimestamp);
        }

        public static long WindowsToUnix(DateTime windowsDate)
        {
            return (long)(windowsDate.Subtract(new DateTime(1970, 1, 1))).TotalSeconds;
        }
    }
}

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

namespace VMIdentity.CommonUtils
{
    public static class DateTimeConverter
    {
        public static DateTime FromUnixToLocalDateTime(double timestamp)
        {
            var dateTime = DateTime.Parse("01-Jan-1970");
            return dateTime.AddSeconds(timestamp).ToLocalTime();
        }

        public static DateTime FromUnixToDateTime(double timestamp)
        {
            var dateTime = DateTime.Parse("01-Jan-1970");
            return dateTime.AddSeconds(timestamp);
        }

        public static long ToUnixDate(DateTime date)
        {
            var startDate = DateTime.Parse("01-Jan-1970");
            return (long)date.Subtract(startDate).TotalSeconds;
        }

        public static string ToDurationAgo(DateTime dateTime)
        {
			var duration = DateTime.UtcNow.Subtract(dateTime);
			string value = string.Empty;

            if (duration <= TimeSpan.FromSeconds(60))
            {
                value = string.Format("{0} secs", duration.Seconds);
            }
            else if (duration <= TimeSpan.FromMinutes(60))
            {
                value = String.Format("{0} mins {1} secs", duration.Minutes, duration.Seconds);
            }
            else if (duration <= TimeSpan.FromHours(24))
            {
                value = String.Format("{0} hrs {1} mins {2} secs", duration.Hours, duration.Minutes, duration.Seconds);
            }
            else if (duration <= TimeSpan.FromDays(30))
            {
                String.Format("{0} day(s)", duration.Days);         
            }
            else value = "over a month";

            if (!string.IsNullOrWhiteSpace(value))
			    value += " ago";

            if (value.Contains("-"))
                value = "Change system time to align with the Local Time Zone";
            return value;
        }
    }
}
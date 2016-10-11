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

using System.Linq;
using System.Net;

namespace VMIdentity.CommonUtils.Utilities
{
    public class Network
    {
        public static string GetIpAddress(string hostname)
        {
            var ips = Dns.GetHostAddresses(hostname);
            return ips != null ? ips.First().ToString() : string.Empty;
        }

        public static bool IsValidEmail(string email)
        {
            try
            {
                var address = new System.Net.Mail.MailAddress(email);
                return address.Address == email;
            }
            catch
            {
                return false;
            }
        }

        public static bool IsValidIP(string ip)
        {
            IPAddress address;
            if (IPAddress.TryParse(ip, out address))
                return true;
            return false;
        }
    }
}

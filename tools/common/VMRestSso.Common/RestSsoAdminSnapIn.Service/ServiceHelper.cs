/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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


using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service
{
    public class ServiceHelper
    {
        public static Dictionary<HttpRequestHeader, string> AddHeaders(string mediaType=null)
        {
            return new Dictionary<HttpRequestHeader, string>
                       {
                           {HttpRequestHeader.ContentType, mediaType ?? ServiceConstants.ContentType},
                           {HttpRequestHeader.UserAgent, ServiceConstants.UserAgent}
                       };
        }

        public static string GetHostName(string ipAddress)
        {
            IPAddress address;
            if (IPAddress.TryParse(ipAddress, out address))
            {
                try
                {
                    IPHostEntry entry = Dns.GetHostEntry(ipAddress);
                    if (entry != null)
                    {
                        ipAddress = entry.HostName;
                    }
                }
                catch
                {
                    return ipAddress;
                }
            }
            return ipAddress;
        }
    }
}

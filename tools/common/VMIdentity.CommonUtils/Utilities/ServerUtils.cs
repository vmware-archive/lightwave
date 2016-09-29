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
using System.Net;
using System.Net.Sockets;

namespace VMIdentity.CommonUtils
{
    public static class ServerUtils
    {

        public static bool CheckServerReachability(string serverNameOrAddress, int port, int timeoutMs)
        {
            try
            {
                var addresses = Dns.GetHostAddresses(serverNameOrAddress);

                using (var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
                {
                    IAsyncResult result = socket.BeginConnect(addresses[0], port, null, null);
                    bool success = result.AsyncWaitHandle.WaitOne(timeoutMs, true);
                    if (success)
                    {
                        success = socket.Connected;
                    }
                    return success;
                }
            }
            catch (Exception e)
            {
                throw e;
            }
        }
    }
}


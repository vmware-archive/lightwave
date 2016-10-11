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

namespace VMDNS.Client
{
    public class VmDnsSession : IDisposable
    {
        public
        VmDnsSession(
            string pszNetworkAddress,
            string pszUserName,
            string pszDomain,
            string pszPassword
        )
        {
            try
            {
                EnsureServerContext(pszNetworkAddress, pszUserName, pszDomain, pszPassword);
            }
            catch (Exception e)
            {
                throw e;
            }

        }

        public void EnsureServerContext(string pszNetworkAddress,
                                        string pszUserName,
                                        string pszDomain,
                                        string pszPassword)
        {
            try
            {
                UInt32 dwError = 0;
                IntPtr pServerContext = IntPtr.Zero;

                if (this.ServerContext != IntPtr.Zero)
                    return;
                dwError = Adaptor.VmDnsOpenServerA(
                    pszNetworkAddress, pszUserName, pszDomain, pszPassword,
                    0,
                    IntPtr.Zero,
                    out pServerContext
                );
                if (dwError != 0)
                {
                    throw new VmDnsException(dwError);
                }
                this.ServerContext = pServerContext;

            }
            catch (Exception e)
            {
                throw e;
            }
        }


        ~VmDnsSession ()
        {
            this.Dispose(false);
        }

        public void
        Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        public void
        Dispose(bool dispose)
        {
            DisposeConnection();
        }

        public void DisposeConnection()
        {
            if (this.ServerContext != IntPtr.Zero)
            {
                Adaptor.VmDnsCloseServer(this.ServerContext);
                this.ServerContext = IntPtr.Zero;
            }
        }

        public IntPtr ServerContext
        {
            get;
            set;
        }

    }
}

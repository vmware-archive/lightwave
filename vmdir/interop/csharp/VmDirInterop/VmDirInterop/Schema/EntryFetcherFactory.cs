/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using VMDirInterop;
using VMDirInterop.LDAP;
using VMDirInterop.Interfaces;
using VmDirInterop.Schema.Interfaces;
using VMDirInterop.LDAPExceptions;

namespace VmDirInterop.Schema
{
    public class EntryFetcherFactory : IEntryFetcherFactory
    {
        private const long defaultLdapTimeoutSec = 3;
        private const long defaultLdapTimeoutUsec = 0;

        public EntryFetcherFactory()
        {
            SetLdapTimeout(defaultLdapTimeoutSec, defaultLdapTimeoutUsec);
        }

        public IEntryFetcher CreateEntryFetcher(String hostName, String upn, String passwd)
        {
            EntryFetcher entryFetcher = null;
            try
            {
                ILdapConnection ldapConn = new LdapConnection();
                ldapConn.VmDirSafeLDAPBind(hostName, upn, passwd);
                entryFetcher = new EntryFetcher(ldapConn);
            }
            catch (LdapException)
            {
                // noop
            }
            return entryFetcher;
        }

        private void SetLdapTimeout(long sec, long usec)
        {
            timeval timeout = new timeval(sec, usec);
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(timeval)));
            Marshal.StructureToPtr(timeout, ptr, false);
            new LdapConnection().SetOption(LDAPOption.LDAP_OPT_NETWORK_TIMEOUT, ptr);
            Marshal.FreeHGlobal(ptr);
        }
    }
}

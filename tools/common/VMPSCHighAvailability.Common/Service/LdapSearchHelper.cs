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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;

namespace VMPSCHighAvailability.Common.Service
{
    public class LdapSearchHelper
    {
        private string _server;
        private string _bindDN;
        private string _password;
        public LdapSearchHelper(string server, string bindDN, string password)
        {
            _server = server;
            _bindDN = bindDN;
            _password = password;
        }
        public void Search(String searchDN, LdapScope scope, string filter, string[] attribsToReturn, bool includeValues, Action<ILdapMessage, List<ILdapEntry>> action)
        {
            List<ILdapEntry> entries = new List<ILdapEntry>();
            var ldConn = new LdapConnection();

            try
            {
                ldConn.VmDirSafeLDAPBind(_server, _bindDN, _password);
                var attrsOnly = includeValues ? 0 : 1;
                var searchRequest = ldConn.LdapSearchExtS(searchDN, (int)scope, filter, attribsToReturn, attrsOnly, IntPtr.Zero, 0);

                if (searchRequest != null)
                {
                    entries = searchRequest.GetEntries();
                    if (action != null)
                        action(searchRequest, entries);
                }
            }
            finally
            {
                if (ldConn != null)
                    ldConn.LdapUnbindS();
            }
        }
    }
}

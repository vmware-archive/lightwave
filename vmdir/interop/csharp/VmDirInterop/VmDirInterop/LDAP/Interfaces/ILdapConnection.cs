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
using VMDirInterop.LDAP;

namespace VMDirInterop.Interfaces
{
    public interface ILdapConnection
    {
        void LdapSimpleBindS(string dn, string passwd);
        void SetVersion(int version);
        void SetOption(int option, IntPtr ptrVersion);
        void VmDirSafeLDAPBind(string host, string upn, string passwd);
        ILdapMessage LdapSearchExtExS(string querybase, int scope, string filter, string[] attrs, int attrsonly, IntPtr timeout, int sizelimit, IntPtr[] ServerControls);
        ILdapMessage LdapSearchExtS(string querybase, int scope, string filter, string[] attrs, int attrsonly, IntPtr timeout, int sizelimit);
        void AddObject(string dn, LdapMod[] attrs);
        void ModifyObject(string basedn, LdapMod[] attrs);
        void DeleteObject(string dn);
        void CleanSearch();
        void LdapUnbindS();
        IntPtr LdapCreatePageControl(int pagesize, IntPtr cookie, bool isCritical);
        void LdapParseResult(ILdapMessage msg, ref IntPtr serverControls, bool fFreeIt);
        void LdapParsePageControl(IntPtr serverControls, ref IntPtr cookie);
        bool HasMorePages(IntPtr cookie);
    }
}

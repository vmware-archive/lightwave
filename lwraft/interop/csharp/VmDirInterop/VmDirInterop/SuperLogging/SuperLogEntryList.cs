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
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Linq;
using System.Text;
using VmDirInterop.SuperLogging.Constants;
using VmDirInterop.SuperLogging.Interfaces;

namespace VmDirInterop.SuperLogging
{
    public class SuperLogEntryList : ISuperLogEntryList
    {
        private IntPtr _pEntries;
        private List<ISuperLogEntry> _entryList = new List<ISuperLogEntry>();
        private static int _entrySize =
            Marshal.SizeOf(typeof(VMDIR_SUPERLOG_ENTRY_LDAPOPERATION));

        public SuperLogEntryList(IntPtr pEntries)
        {
            _pEntries = pEntries;
            if (_pEntries != IntPtr.Zero)
            {
                VMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY array =
                    (VMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY)Marshal.PtrToStructure(
                        _pEntries,
                        typeof(VMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY));

                long ptr = array.entries.ToInt64();
                for (int i = 0; i < array.dwCount; i++, ptr += _entrySize)
                {
                    VMDIR_SUPERLOG_ENTRY_LDAPOPERATION e =
                        (VMDIR_SUPERLOG_ENTRY_LDAPOPERATION)Marshal.PtrToStructure(
                            new IntPtr(ptr),
                            typeof(VMDIR_SUPERLOG_ENTRY_LDAPOPERATION));
                    if (e.opType == LDAPOperations.LDAP_REQ_SEARCH)
                    {
                        _entryList.Add(new SuperLogSearchEntry(e));
                    }
                    else
                    {
                        _entryList.Add(new SuperLogEntry(e));
                    }
                }
            }
        }

        ~SuperLogEntryList()
        {
            SuperLoggingClientLibrary.VmDirFreeSuperLogEntryLdapOperationArray(_pEntries);
        }

        public IEnumerator GetEnumerator()
        {
            foreach (ISuperLogEntry e in _entryList)
            {
                yield return e;
            }
        }

        public List<ISuperLogEntry> getEntries()
        {
            return _entryList;
        }

        public int getCount()
        {
            return _entryList.Count;
        }

        public IntPtr getIntPtr()
        {
            return _pEntries;
        }
    }
}

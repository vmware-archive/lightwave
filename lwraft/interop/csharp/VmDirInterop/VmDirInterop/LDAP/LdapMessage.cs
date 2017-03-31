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
using VMDirInterop.Interfaces;

namespace VMDirInterop.LDAP
{
    public class LdapMessage : ILdapMessage
    {
        private LdapConnection _connection = null;
        private IntPtr _message = IntPtr.Zero;

        public LdapMessage(LdapConnection connection)
        {
            this._connection = connection;
        }

        public LdapMessage(LdapConnection connection, IntPtr message)
        {
            this._connection = connection;
            this._message = message;
        }

        public List<ILdapEntry> GetEntries()
        {
            int numberEntries = LdapClientLibrary.ldap_count_entries(this._connection.GetIntPtr(), this._message);
            List<ILdapEntry> entryList = new List<ILdapEntry>(numberEntries);
            if (numberEntries > 0)
            {
                try
                {
                    IntPtr pEntry = LdapClientLibrary.ldap_first_entry(this._connection.GetIntPtr(), this._message);
                    while ((pEntry != null) && (pEntry != IntPtr.Zero))
                    {
                        entryList.Add(new LdapEntry(this, pEntry));
                        pEntry = LdapClientLibrary.ldap_next_entry(this._connection.GetIntPtr(), pEntry);
                    }
                }
                catch (Exception exception)
                {
                    throw exception;
                }
            }
            return entryList;
        }

        //Override
        public int GetEntriesCount()
        {
            int returnError;
            try
            {
                returnError = LdapClientLibrary.ldap_count_entries(this._connection.GetIntPtr(), this._message);
                return (returnError);
            }
            catch (Exception exception)
            {
                throw exception;
            }
        }

        public LdapConnection GetConnection()
        {
            return this._connection;
        }

        public void FreeMessage()
        {
            if ((this._message != IntPtr.Zero))
            {
                LdapClientLibrary.ldap_msgfree(this._message);
            }
        }

    }
}

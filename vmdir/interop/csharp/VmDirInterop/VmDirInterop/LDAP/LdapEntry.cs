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
using System.Runtime.InteropServices;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAPExceptions;

namespace VMDirInterop.LDAP
{
    public class LdapEntry : ILdapEntry
    {
        private LdapMessage _message = null;
        private IntPtr _entry = IntPtr.Zero;

        public LdapEntry(LdapMessage msg, IntPtr ent)
        {
            this._message = msg;
            this._entry = ent;
        }

        //Override
        public string getDN()
        {
            IntPtr dnPointer = IntPtr.Zero;
            string dn;

            try
            {
                dnPointer = LdapClientLibrary.ldap_get_dn(this._message.GetConnection().GetIntPtr(), this._entry);
                dn = Marshal.PtrToStringAnsi(dnPointer);
                return (dn);
            }
            finally
            {
                LdapClientLibrary.ldap_memfree(dnPointer);
            }
        }

        //Override
        public List<string> getAttributeNames()
        {
            IntPtr attributePointer = IntPtr.Zero, berPointer = IntPtr.Zero;
            List<string> attributeList = new List<string>();

            try
            {
                attributePointer = LdapClientLibrary.ldap_first_attribute(this._message.GetConnection().GetIntPtr(), this._entry, out berPointer);
                while (attributePointer != IntPtr.Zero)
                {
                    var attributeName = Marshal.PtrToStringAnsi(attributePointer);
                    attributeList.Add(attributeName);
                    LdapClientLibrary.ldap_memfree(attributePointer);
                    attributePointer = LdapClientLibrary.ldap_next_attribute(this._message.GetConnection().GetIntPtr(), this._entry, berPointer);
                }
                return attributeList;
            }
            finally
            {
                LdapClientLibrary.ldap_value_free(attributePointer);
                LdapClientLibrary.ber_free(berPointer, 0);
            }
        }

        //Override
        public List<LdapValue> getAttributeValues(string attributeName)
        {
            List<LdapValue> attributeValues = new List<LdapValue>();
            IntPtr attributePointer = IntPtr.Zero;
            int i = 0;
            try
            {
                attributePointer = LdapClientLibrary.ldap_get_values(this._message.GetConnection().GetIntPtr(), this._entry, attributeName);
                var count = LdapClientLibrary.ldap_count_values(attributePointer);
                while (i < count)
                {
                    var attributeValuePointer = Marshal.ReadIntPtr(attributePointer, System.Runtime.InteropServices.Marshal.SizeOf(attributePointer) * i);
                    var attributeValue = new LdapValue(Marshal.PtrToStringAnsi(attributeValuePointer));
                    attributeValues.Add(attributeValue);
                    i++;
                }
                return attributeValues;
            }
            finally
            {
                LdapClientLibrary.ldap_value_free(attributePointer);
            }
        }
    }
}

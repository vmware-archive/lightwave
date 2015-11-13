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
using VMDir.Common.DTO;
using VMDirInterop.LDAP;
using VMDirInterop.Interfaces;

namespace VMDir.Common.VMDirUtilities
{
    public class VMDirBagItem
    {
        public object Value { get; set; }

        public bool IsReadOnly { get; set; }

        public bool IsRequired { get; set; }

        public string Description { get; set; }
    }

    public static class Utilities
    {
        //LDAP Attribute Constants
        public const string NTSecurityDescriptor = "nTSecurityDescriptor";
        public const string ObjectSID = "objectSid";

        //dont show attributes list
        public static HashSet<string> DontShowAttributes = new HashSet<string> {
            Utilities.NTSecurityDescriptor,
            Utilities.ObjectSID
        };

        //returns string from LdapValue. if LDAPValue is array, return first object.
        public static string LdapValueToString (object ldapValue)
        {
            if (ldapValue != null) {
                Type valueType = ldapValue.GetType ();
                if (valueType.IsArray) {
                    LdapValue[] arr = ldapValue as LdapValue[];
                    //return last element of the array
                    var entry = (LdapValue)arr [(arr.Length - 1)];
                    return entry.StringValue;
                } else {
                    var entry = (LdapValue)ldapValue;
                    return entry.StringValue;
                }
            }
            return "";
        }
            

        //returns LDAPValue  from string
        public static LdapValue StringToLdapValue (string stringVal)
        {
            LdapValue val = new LdapValue (stringVal);
            return val;
        }

        //call checked exec on the caller of this method
        public static void  GetItemProperties (string itemName, VMDirServerDTO serverDTO, Dictionary<string,VMDirBagItem> _properties)
        {
            ILdapMessage ldMsg = null;
            try {
                List<ILdapEntry> dnEntries = serverDTO.Connection.SearchAndGetEntries (itemName, LdapScope.SCOPE_BASE, "(objectClass=*)", null, 0, ref ldMsg);
                foreach (LdapEntry entry in dnEntries) {
                    string[] names = entry.getAttributeNames ().ToArray ();
                    foreach (string name in names) {
                        LdapValue[] vals = entry.getAttributeValues (name).ToArray ();
                        var val = vals;
                        var key = name;
                        var attribType = serverDTO.Connection.SchemaManager.GetAttributeType (key);
                        bool readOnly = true;
                        string desc = "";
                        if (attribType != null) {
                            readOnly = attribType.ReadOnly;
                            desc = attribType.Description;
                            if (attribType != null) {
                                _properties [key] = new VMDirBagItem {
                                    Value = val,
                                    IsReadOnly = readOnly,
                                    Description = desc,
                                    IsRequired = true
                                };
                            }
                        }
                    }
                }
                if (ldMsg != null)
                    (ldMsg as LdapMessage).FreeMessage ();
            } catch (Exception e) {
                throw e;
            }
        }

        public static string[]  SearchItemCN (string itemName, string searchObjectClass, string searchName, string[] attribs, VMDirServerDTO serverDTO)
        {
            ILdapMessage ldMsg = null;
            try {
                string filter = "(&(objectClass=" + searchObjectClass + ")(cn=" + searchName + "))";
                List<string> dn = new List<String> ();
                List<ILdapEntry> dnEntries = serverDTO.Connection.SearchAndGetEntries (itemName, LdapScope.SCOPE_SUBTREE, filter, attribs, 0, ref ldMsg);
                foreach (LdapEntry entry in dnEntries) {
                    dn.Add (entry.getDN ());
                }
                return dn.ToArray ();
            } catch (Exception e) {
                throw e;
            }
        }

        //Remove specific System  Attributes from UI which are not editable
        public static void RemoveDontShowAttributes (Dictionary<string,VMDirBagItem> properties)
        {
            foreach (string val in Utilities.DontShowAttributes) {
                if (properties.ContainsKey (val))
                    properties.Remove (val);
            }
        }
    }
}


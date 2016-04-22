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
using System.Globalization;
using VMDir.Common.Schema;
using VMDirSnapIn.Utilities;using VMDirInterop.LDAP;
using VMDir.Common.VMDirUtilities;

namespace VMDirSnapIn.Services{    public static class LdapTypesService    {        public static string DATE_FORMAT = "yyyyMMddHHmmss.fZ";
        public static int AttributeTypeSort(AttributeTypeDTO lhs, AttributeTypeDTO rhs)        {            return lhs.Name.CompareTo(rhs.Name);        }
        public static int ObjectClassDTOSort(ObjectClassDTO lhs, ObjectClassDTO rhs)        {            return lhs.Name.CompareTo(rhs.Name);        }
        public static LdapMod MakeAttribute(KeyValuePair<string, VMDirBagItem> entry)        {
            object val = null;
            object entryVal = entry.Value.Value;
            if (entryVal != null)
            {
                var type = entryVal.GetType();
                switch (type.FullName)
                {
                    case "System.DateTime":
                        val = DateTime.Now.ToString(DATE_FORMAT);
                        break;
                    default:
                        val = entryVal;
                        break;
                }
                //entry.Value.Value
            }
            return new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, entry.Key, new string[] { val.ToString(), null });        }

        public static string ConvertVal(object valueIn)
        {
            string result = ""; 
            if (valueIn == null)
                return result;

            var type = valueIn.GetType();

            switch (type.FullName)
            {
                //case "System.Byte[]":
                //    result = BitConverter.ToString(valueIn as byte[]);
                //    break;
                default:
                    result = valueIn.ToString();
                    break;
            }
            return result;
        }
        public static object GetInstanceWithVal(string type, List<LdapValue> ldapValue)        {            object returnVal = "";            bool hasMultiple = ldapValue.Count > 1;            if (!hasMultiple)            {                returnVal = MiscUtilsService.GetInstanceFromType(type);                switch(type)                {                    case "System.DateTime":
                        var str = ldapValue[0].StringValue;                        returnVal = DateTime.ParseExact(str, DATE_FORMAT, CultureInfo.InvariantCulture);                        break;                    default:
                        returnVal = ConvertVal(ldapValue[0].StringValue);
                        break;                }            }            else            {                var list = new List<string>();

                foreach (LdapValue value in ldapValue)
                    list.Add(value.StringValue);                returnVal = list;            }            return returnVal;        }    }}

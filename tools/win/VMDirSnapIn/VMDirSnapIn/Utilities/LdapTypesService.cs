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
using System.Globalization;
using VMDir.Common.Schema;
using VMDirSnapIn.Utilities;
using VMDirInterop.LDAP;
using VMDir.Common.VMDirUtilities;
using VMDir.Common.DTO;

namespace VMDirSnapIn.Utilities
{
    public static class LdapTypesService
    {
        public static int AttributeTypeSort(AttributeTypeDTO lhs, AttributeTypeDTO rhs)
        {
            return lhs.Name.CompareTo(rhs.Name);
        }
        //move to common
        public static LdapMod MakeAttribute(KeyValuePair<string, VMDirAttributeDTO> entry)
        {
            List<LdapValue> entryVal = entry.Value.Values;
            List<string> vals = new List<string>() { null };
            if (entryVal != null)
            {
                vals = entryVal.ConvertAll(x => x.StringValue);
                vals.Add(null);
            }
            return new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, entry.Key, vals.ToArray());
        }
    }
}

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
using VMDirInterop.LDAP;
using VMDir.Common.VMDirUtilities;
using System.Linq;

namespace VMDir.Common.Schema
{
    public class ObjectClassManager
    {
        private Dictionary<string, ObjectClassDTO> _data = new Dictionary<string, ObjectClassDTO>();

        public Dictionary<string, ObjectClassDTO> Data { get { return _data; } }

        /*Declaring local constants here ,as these are case sensitive and to be used only in the function below
         * Not to be confused with similar contants being used in the Application in other places.
         * */
        public const string ObjectClassDescription = "description";
        public const string ObjectClassSubClass = "subClassOf";
        public const string ObjectClassCategory = "objectClassCategory";
        public const string SystemMayContain = "systemMayContain";
        public const string SystemMustContain = "systemMustContain";
        public const string ObjectClass = "objectClass";
        public const string AuxiliaryClass = "auxiliaryClass";

        public ObjectClassManager(Dictionary<string, Dictionary<string,object>> dict)
        {
            AddObjectClassDTO(dict);
        }

        private void AddObjectClassDTO(Dictionary<string, Dictionary<string,object>> dict)
        {
            foreach (var key in dict.Keys)
            {
                var dto = new ObjectClassDTO { Name = key };
                Dictionary<string,object> objectclasses = dict[key];

                LdapValue[] val;
                val = Utilities.FetchLdapValueFromAttributesDictionary(ObjectClassDescription, objectclasses);
                dto.Description = val != null ? val[0].StringValue : string.Empty;

                val = Utilities.FetchLdapValueFromAttributesDictionary(ObjectClassSubClass, objectclasses);
                dto.SuperClass = val != null ? val[0].StringValue : string.Empty;

                val = Utilities.FetchLdapValueFromAttributesDictionary(ObjectClassCategory, objectclasses);
                if (val != null)
                    dto.ClassType = (ObjectClassDTO.ObjectClassType)Convert.ToInt32(val[0].StringValue);

                val = Utilities.FetchLdapValueFromAttributesDictionary(SystemMayContain, objectclasses);
                dto.May = val != null ? val.Select(data => data.StringValue).ToList() : null;

                val = Utilities.FetchLdapValueFromAttributesDictionary(SystemMustContain, objectclasses);
                dto.Must = val != null ? val.Select(data => data.StringValue).ToList() : null;

                val = Utilities.FetchLdapValueFromAttributesDictionary(ObjectClass, objectclasses);
                dto.ObjectClass = val != null ? val.Select(data => data.StringValue).ToList() : null;

                val = Utilities.FetchLdapValueFromAttributesDictionary(AuxiliaryClass, objectclasses);
                dto.Aux = val != null ? val.Select(data => data.StringValue).ToList() : null;


                _data[key] = dto;
            }
        }
    }
}

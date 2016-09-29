/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

    public class ObjectClassManager : ManagerBase
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
        public const string GovernsID = "governsID";

        public const string MayContain = "mayContain";

        public const string MustContain = "mustContain";
        public const string ObjectClass = "objectClass";
        public const string AuxiliaryClass = "auxiliaryClass";

        public const string SystemAuxiliaryClass = "systemAuxiliaryClass";
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



                val = Utilities.FetchLdapValueFromAttributesDictionary(GovernsID, objectclasses);

                dto.GovernsID = val != null ? val[0].StringValue : string.Empty;



                val = Utilities.FetchLdapValueFromAttributesDictionary(ObjectClassCategory, objectclasses);

                if (val != null)

                    dto.ClassType = (ObjectClassDTO.ObjectClassType)Convert.ToInt32(val[0].StringValue);



                val = Utilities.FetchLdapValueFromAttributesDictionary(SystemMayContain, objectclasses);
                dto.May = val != null ? val.Select(data => data.StringValue).ToList() : null;

                val = Utilities.FetchLdapValueFromAttributesDictionary(MayContain, objectclasses);
                if (val != null)

                {
                    if (dto.May != null)

                        dto.May.AddRange(val.Select(data => data.StringValue));

                    else

                        dto.May = val.Select(data => data.StringValue).ToList();
                }

                val = Utilities.FetchLdapValueFromAttributesDictionary(SystemMustContain, objectclasses);
                dto.Must = val != null ? val.Select(data => data.StringValue).ToList() : null;

                val = Utilities.FetchLdapValueFromAttributesDictionary(MustContain, objectclasses);
                if (val != null)

                {
                    if (dto.Must != null)

                        dto.Must.AddRange(val.Select(data => data.StringValue));

                    else

                        dto.Must = val.Select(data => data.StringValue).ToList();
                }


                val = Utilities.FetchLdapValueFromAttributesDictionary(ObjectClass, objectclasses);

                dto.ObjectClass = val != null ? val.Select(data => data.StringValue).ToList() : null;



                val = Utilities.FetchLdapValueFromAttributesDictionary(AuxiliaryClass, objectclasses);

                dto.Aux = val != null ? val.Select(data => data.StringValue).ToList() : null;

                val = Utilities.FetchLdapValueFromAttributesDictionary(SystemAuxiliaryClass, objectclasses);
                if (val != null)
                {
                    if (dto.Aux != null)

                        dto.Aux.AddRange(val.Select(data => data.StringValue));

                    else

                        dto.Aux = val.Select(data => data.StringValue).ToList();
                }



                _data[key] = dto;

            }

        }
        public ObjectClassManager(List<string> items)
        {
            InitSchemaDefinition();
            ParseAndAdd(items);
        }
        private void InitSchemaDefinition()
        {
            _defs = new List<SchemaComponentDef> {
                new SchemaComponentDef{ Name = "NAME", Parser = SchemaValueParsers.SingleValueQuoted },
                new SchemaComponentDef{ Name = "DESC", Parser = SchemaValueParsers.SingleQuotedString },
                new SchemaComponentDef{ Name = "SUP", Parser = SchemaValueParsers.SingleValue },
                new SchemaComponentDef{ Name = "STRUCTURAL", Parser = SchemaValueParsers.IsDefined },
                new SchemaComponentDef{ Name = "MUST", Parser = SchemaValueParsers.SingleOrMultipleDollar },
                new SchemaComponentDef{ Name = "MAY", Parser = SchemaValueParsers.SingleOrMultipleDollar },
                new SchemaComponentDef{ Name = "AUXILIARY", Parser = SchemaValueParsers.IsDefined },
                new SchemaComponentDef{ Name = "ABSTRACT", Parser = SchemaValueParsers.IsDefined },
            };
        }

        private void ParseAndAdd(List<string> items)
        {
            foreach (var item in items)
            {
                var dict = Parse(item);
                AddObjectClass(dict);
            }
        }
        private void AddObjectClass(Dictionary<string, object> dict)
        {
            string name = dict["NAME"] as string;
            var dto = new ObjectClassDTO { Name = name };
            dto.Description = dict["DESC"] as string;
            dto.SuperClass = dict["SUP"] as string;
            dto.Aux = dict["AUXILIARY"] as List<string>;
            dto.Must = dict["MUST"] as List<string>;
            dto.May = dict["MAY"] as List<string>;
            _data[name] = dto;
        }
    }
}

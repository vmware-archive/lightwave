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
using VMDir.Common.VMDirUtilities;
using VMDirInterop.LDAP;
using System.Linq;

namespace VMDir.Common.Schema
{

    public class AttributeTypeManager : ManagerBase
    {
        /*Declaring local constants here ,as these are case sensitive and to be used only in the function below
         * Not to be confused with similar contants being used in the Application in other places.
         * */
        public const string AttributeDescription = "description";
        public const string AttributeSyntax = "attributeSyntax";
        public const string AttributeSingleValued = "isSingleValued";
        public const string AttributeID = "attributeID";
        public const string ObjectClass = "objectClass";

        private Dictionary<string, AttributeTypeDTO> _data = new Dictionary<string, AttributeTypeDTO>();

        public Dictionary<string, AttributeTypeDTO> Data { get { return _data; } }


        public AttributeTypeManager(Dictionary<string, Dictionary<string,object>> dict)
        {
            AddAttributeTypeDTO(dict);
        }





        private void AddAttributeTypeDTO(Dictionary<string, Dictionary<string,object>> dict)

        {

            foreach (var key in dict.Keys)

            {

                var dto = new AttributeTypeDTO

                {

                    Name = key

                };

                Dictionary<string, object> attributes = dict[key];

                LdapValue[] val;

                val = Utilities.FetchLdapValueFromAttributesDictionary(AttributeDescription, attributes);

                dto.Description = val != null ? val[0].StringValue : string.Empty;

                val = Utilities.FetchLdapValueFromAttributesDictionary(AttributeSyntax, attributes);

                dto.AttributeSyntax = val != null ? val[0].StringValue : string.Empty;

                dto.Type = "System.String";

                var syntax = VMDirCommonEnvironment.Instance.SyntaxDefs[dto.AttributeSyntax];

                if (syntax != null)

                    dto.Type = syntax.Name;

                val = Utilities.FetchLdapValueFromAttributesDictionary(AttributeSingleValued, attributes);

                dto.SingleValue = val != null ? Convert.ToBoolean(val[0].StringValue) : true;

                val = Utilities.FetchLdapValueFromAttributesDictionary(AttributeID, attributes);

                dto.AttributeID = val != null ? val[0].StringValue : string.Empty;

                val = Utilities.FetchLdapValueFromAttributesDictionary(ObjectClass, attributes);

                dto.ObjectClass = val != null ? val.Select(data => data.StringValue).ToList() : null;

                _data[key] = dto;

            }

        }

        public AttributeTypeManager(List<string> items)
        {
            InitSchemaDefinition();
            ParseAndAdd(items);
        }

        private void InitSchemaDefinition()
        {
            _defs = new List<SchemaComponentDef> {
                new SchemaComponentDef{ Name = "NAME", Parser = SchemaValueParsers.SingleOrMultipleQuoted },
                new SchemaComponentDef{ Name = "DESC", Parser = SchemaValueParsers.SingleQuotedString },
                new SchemaComponentDef{ Name = "EQUALITY", Parser = SchemaValueParsers.SingleValue },
                new SchemaComponentDef{ Name = "SUBSTR", Parser = SchemaValueParsers.SingleValue },
                new SchemaComponentDef{ Name = "SYNTAX", Parser = SchemaValueParsers.SingleValue },
                new SchemaComponentDef{ Name = "SINGLE-VALUE", Parser = SchemaValueParsers.IsDefined },
                new SchemaComponentDef{ Name = "NO-USER-MODIFICATION", Parser = SchemaValueParsers.IsDefined },
                new SchemaComponentDef{ Name = "USAGE", Parser = SchemaValueParsers.SingleValue },
            };
        }

        private void ParseAndAdd(List<string> items)
        {
            foreach (var item in items)
            {
                var dict = Parse(item);
                AddAttributeType(dict);
            }
        }

        private void AddAttributeType(Dictionary<string, object> dict)
        {
            var val = dict["NAME"];
            if (val is string)
                AddAttributeType(val as string, dict);
            else
            {
                foreach (var name in val as List<string>)
                    AddAttributeType(name, dict);
            }

        }
        private void AddAttributeType(string name, Dictionary<string, object> dict)
        {
            var dto = new AttributeTypeDTO { Name = name };
            dto.Description = dict["DESC"] as string;
            if (dto.Description == null)
                dto.Description = "";

            dto.AttributeSyntax = dict["SYNTAX"] as string;

            if (VMDirCommonEnvironment.Instance.SyntaxDefs != null)
            {
                var syntax = VMDirCommonEnvironment.Instance.SyntaxDefs[dto.AttributeSyntax];
                if (syntax == null)
                {
                    dto.Type = "System.String";
                }
                else
                {
                    dto.Type = syntax.Name;
                }
                dto.SingleValue = (bool)dict["SINGLE-VALUE"];
                _data[name] = dto;
            }
        }
    }
}

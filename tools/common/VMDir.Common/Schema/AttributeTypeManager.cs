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
using VMDir.Common.VMDirUtilities;

namespace VMDir.Common.Schema
{
    public class AttributeTypeManager : ManagerBase
    {
        private Dictionary<string, AttributeTypeDTO> _data = new Dictionary<string, AttributeTypeDTO> ();

        public Dictionary<string, AttributeTypeDTO> Data { get { return _data; } }

        public AttributeTypeManager (List<string> items)
        {
            InitSchemaDefinition ();
            ParseAndAdd (items);
        }

        private void InitSchemaDefinition ()
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

        private void ParseAndAdd (List<string> items)
        {
            foreach (var item in items) {
                var dict = Parse (item);
                AddAttributeType (dict);
            }
        }

        private void AddAttributeType (Dictionary<string, object> dict)
        {
            var val = dict ["NAME"];
            if (val is string)
                AddAttributeType (val as string, dict);
            else {
                foreach (var name in val as List<string>)
                    AddAttributeType (name, dict);
            }

        }

        private int GetTypeLength (string syntax)
        {
            int length = 0;
            if (string.IsNullOrEmpty (syntax))
                return 0;

            int lengthStart = syntax.IndexOf ('{') + 1;
            if (lengthStart > 0) {
                int lengthEnd = syntax.IndexOf ('}');
                string lengthStr = syntax.Substring (lengthStart, lengthEnd - lengthStart);
                int.TryParse (lengthStr, out length);
            }
            return length;
        }

        private void AddAttributeType (string name, Dictionary<string, object> dict)
        {
            var dto = new AttributeTypeDTO { Name = name };
            dto.Description = dict ["DESC"] as string;
            if (dto.Description == null)
                dto.Description = "";

            string syntaxString = dict ["SYNTAX"] as string;
            dto.Length = GetTypeLength (syntaxString);
            if (dto.Length > 0) {
                string lengthString = "{" + dto.Length + "}";
                syntaxString = syntaxString.Replace (lengthString, "");
            }
            if (VMDirCommonEnvironment.Instance.SyntaxDefs != null) {
                var syntax = VMDirCommonEnvironment.Instance.SyntaxDefs [syntaxString];
                if (syntax == null) {
                    dto.Type = "System.String";
                    dto.SyntaxName = "Unknown";
                } else {
                    dto.Type = syntax.Type;
                    dto.SyntaxName = syntax.Name;
                }
                dto.ReadOnly = (bool)dict ["NO-USER-MODIFICATION"];
                dto.SingleValue = (bool)dict ["SINGLE-VALUE"];
                _data [name] = dto;
            }
        }
    }
}

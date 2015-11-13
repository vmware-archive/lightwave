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

namespace VMDir.Common.Schema
{
    public class ObjectClassManager : ManagerBase
    {
        private Dictionary<string, ObjectClassDTO> _data = new Dictionary<string, ObjectClassDTO> ();

        public Dictionary<string, ObjectClassDTO> Data { get { return _data; } }

        public ObjectClassManager (List<string> items)
        {
            InitSchemaDefinition ();
            ParseAndAdd (items);
        }

        private void InitSchemaDefinition ()
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

        private void ParseAndAdd (List<string> items)
        {
            foreach (var item in items) {
                var dict = Parse (item);
                AddObjectClass (dict);
            }
        }

        private void AddObjectClass (Dictionary<string, object> dict)
        {
            string name = dict ["NAME"] as string;

            var dto = new ObjectClassDTO { Name = name };
            dto.Description = dict ["DESC"] as string;
            dto.IsAbstract = (bool)dict ["ABSTRACT"];
            dto.IsStructural = (bool)dict ["STRUCTURAL"];
            dto.SuperClass = dict ["SUP"] as string;
            dto.Must = dict ["MUST"] as List<string>;
            dto.May = dict ["MAY"] as List<string>;
            _data [name] = dto;
        }
    }
}

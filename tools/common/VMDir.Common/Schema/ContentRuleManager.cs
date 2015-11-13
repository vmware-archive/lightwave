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
    public class ContentRuleManager : ManagerBase
    {
        private Dictionary<string, ContentRuleDTO> _data = new Dictionary<string, ContentRuleDTO> ();

        public Dictionary<string, ContentRuleDTO> Data { get { return _data; } }

        public ContentRuleManager (List<string> items)
        {
            InitSchemaDefinition ();
            ParseAndAdd (items);
        }

        private void InitSchemaDefinition ()
        {
            _defs = new List<SchemaComponentDef> {
                new SchemaComponentDef{ Name = "NAME", Parser = SchemaValueParsers.SingleValueQuoted },
                new SchemaComponentDef{ Name = "AUX", Parser = SchemaValueParsers.SingleOrMultipleDollar },
                new SchemaComponentDef{ Name = "MUST", Parser = SchemaValueParsers.SingleOrMultipleDollar },
                new SchemaComponentDef{ Name = "MAY", Parser = SchemaValueParsers.SingleOrMultipleDollar }
            };
        }

        private void ParseAndAdd (List<string> items)
        {
            foreach (var item in items) {
                var dict = Parse (item);
                AddContentRule (dict);
            }
        }

        private void AddContentRule (Dictionary<string, object> dict)
        {
            string name = dict ["NAME"] as string;

            var dto = new ContentRuleDTO { Name = name };
            dto.Aux = dict ["AUX"] as List<string>;
            dto.Must = dict ["MUST"] as List<string>;
            dto.May = dict ["MAY"] as List<string>;
            _data [name] = dto;
        }
    }
}

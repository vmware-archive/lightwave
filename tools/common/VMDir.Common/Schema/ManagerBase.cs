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
using System.Linq;

namespace VMDir.Common.Schema
{
    public class ManagerBase
    {
        protected List<SchemaComponentDef> _defs = null;

        protected Dictionary<string, object> Parse(string attr)
        {
            var dict = new Dictionary<string, object>();

            var bits = attr.Split(' ');
            var trimmedBits = bits.Where(x => !string.IsNullOrEmpty(x)).ToList();

            var indices = new List<int>();
            foreach (var entry in _defs)
            {
                int index = trimmedBits.IndexOf(entry.Name);
                entry.IndexRange.Start = index;
                indices.Add(index);
            }
            indices.Add(trimmedBits.Count - 1);
            indices.Sort();

            _defs.ForEach(x => x.IndexRange.End = indices[indices.IndexOf(x.IndexRange.Start) + 1]);
            foreach (var entry in _defs)
            {
                int start = entry.IndexRange.Start + 1;
                int end = entry.IndexRange.End;
                if (entry.Parser != null)
                {
                    dict[entry.Name] = entry.Parser(start == 0 ? null : trimmedBits.GetRange(start, end - start));
                }
            }
            return dict;
        }
    }
}

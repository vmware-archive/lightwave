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
using System.Collections.Generic;

namespace VMDir.Common.Schema
{
    public class IndexRange
    {
        public int Start { get; set; }

        public int End { get; set; }

        public IndexRange()
        {
            Start = End = -1;
        }
    }

    public delegate object ParserDelegate(List<string> parts);

    public class SchemaComponentDef
    {
        public string Name { get; set; }

        public IndexRange IndexRange { get; set; }

        public ParserDelegate Parser { get; set; }

        public SchemaComponentDef()
        {
            IndexRange = new IndexRange();
        }
    }
}
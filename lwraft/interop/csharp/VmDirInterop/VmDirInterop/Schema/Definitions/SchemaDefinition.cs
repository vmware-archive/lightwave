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
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using VmDirInterop.Schema.Interfaces;

namespace VmDirInterop.Schema.Definitions
{
    public abstract class SchemaDefinition
    {
        protected String defStr;
        public String name { get; protected set; }

        protected SchemaDefinition(String defStr)
        {
            this.defStr = defStr;
        }

        public int CompareTo(SchemaDefinition other)
        {
            return this.NameCompareTo(other);
        }

        public int NameCompareTo(SchemaDefinition other)
        {
            return String.Compare(this.name, other.name);
        }

        public int FullCompareTo(SchemaDefinition other)
        {
            return String.Compare(this.defStr, other.defStr);
        }

        public override string ToString()
        {
            return defStr;
        }
    }
}

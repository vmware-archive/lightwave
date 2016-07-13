﻿/*
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
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using VmDirInterop.Schema.Interfaces;

namespace VmDirInterop.Schema.Definitions
{
    public class AttributeType : SchemaDefinition, ISchemaComparable<AttributeType>
    {
        static Regex regex = new Regex(@"NAME '(?<name>[^\s]+)'");

        public AttributeType(String defStr) : base(defStr)
        {
            GroupCollection groups = regex.Match(defStr).Groups;
            name = groups["name"].Value.Trim();
        }

        public int CompareTo(AttributeType other)
        {
            return base.CompareTo(other);
        }

        public int NameCompareTo(AttributeType other)
        {
            return base.NameCompareTo(other);
        }

        public int FullCompareTo(AttributeType other)
        {
            return base.FullCompareTo(other);
        }
    }
}

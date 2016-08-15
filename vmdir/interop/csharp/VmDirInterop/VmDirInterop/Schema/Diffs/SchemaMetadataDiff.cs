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
using VmDirInterop.Schema.Entries;
using VmDirInterop.Schema.Utils;
using VmDirInterop.Schema.Interfaces;

namespace VmDirInterop.Schema.Diffs
{
    public class SchemaMetadataDiff
    {
        private TupleList<SchemaEntry, SchemaEntry> attributeTypeDiff;
        private TupleList<SchemaEntry, SchemaEntry> objectClassDiff;

        public SchemaMetadataDiff(
            SchemaComparableList<SchemaEntry> baseAtEntries,
            SchemaComparableList<SchemaEntry> baseOcEntries,
            SchemaComparableList<SchemaEntry> otherAtEntries,
            SchemaComparableList<SchemaEntry> otherOcEntries)
        {
            attributeTypeDiff = baseAtEntries.GetDiff(otherAtEntries);
            objectClassDiff = baseOcEntries.GetDiff(otherOcEntries);
        }

        public TupleList<SchemaEntry, SchemaEntry> GetAttributeTypeDiff()
        {
            return attributeTypeDiff;
        }

        public TupleList<SchemaEntry, SchemaEntry> GetObjectClassDiff()
        {
            return objectClassDiff;
        }
    }
}

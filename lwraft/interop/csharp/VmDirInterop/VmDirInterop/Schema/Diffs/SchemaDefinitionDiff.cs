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
using System.Linq;
using System.Text;
using VmDirInterop.Schema.Entries;
using VmDirInterop.Schema.Definitions;
using VmDirInterop.Schema.Utils;

namespace VmDirInterop.Schema.Diffs
{
    public class SchemaDefinitionDiff
    {
        private TupleList<AttributeType, AttributeType> attributeTypeDiff;
        private TupleList<ObjectClass, ObjectClass> objectClassDiff;

        public SchemaDefinitionDiff(SubSchemaSubEntry baseSubSchema, SubSchemaSubEntry otherSubSchema)
        {
            SchemaComparableList<AttributeType> baseAtList = baseSubSchema.GetAttributeTypeList();
            SchemaComparableList<AttributeType> otherAtList = otherSubSchema.GetAttributeTypeList();
            attributeTypeDiff = baseAtList.GetDiff(otherAtList);

            SchemaComparableList<ObjectClass> baseOcList = baseSubSchema.GetObjectClassList();
            SchemaComparableList<ObjectClass> otherOcList = otherSubSchema.GetObjectClassList();
            objectClassDiff = baseOcList.GetDiff(otherOcList);
        }

        public TupleList<AttributeType, AttributeType> GetAttributeTypeDiff()
        {
            return attributeTypeDiff;
        }

        public TupleList<ObjectClass, ObjectClass> GetObjectClassDiff()
        {
            return objectClassDiff;
        }
    }
}

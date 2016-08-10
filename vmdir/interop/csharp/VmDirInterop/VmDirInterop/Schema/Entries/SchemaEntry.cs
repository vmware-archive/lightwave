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
using VMDirInterop.LDAP;
using VMDirInterop.Interfaces;
using VmDirInterop.Schema.Constants;
using VmDirInterop.Schema.Exceptions;
using VmDirInterop.Schema.Interfaces;
using VmDirInterop.Schema.Metadata;
using VmDirInterop.Schema.Utils;

namespace VmDirInterop.Schema.Entries
{
    public class SchemaEntry : ISchemaComparable<SchemaEntry>
    {
        public String defName { get; private set; }
        private SchemaComparableList<AttributeMetadata> metadataList;

        public SchemaEntry(ILdapEntry entry)
        {
            LdapValue val = entry.getAttributeValues(AttributeConstants.CN)[0];
            defName = val.StringValue;

            List<LdapValue> vals = entry.getAttributeValues(AttributeConstants.ATTRIBUTE_METADATA);
            metadataList = ConstructMetadataList(vals);
        }

        public SchemaEntry(String defName, IList<AttributeMetadata> metadataList)
        {
            this.defName = defName;
            this.metadataList = new SchemaComparableList<AttributeMetadata>(metadataList);
        }

        public SchemaComparableList<AttributeMetadata> GetMetadataList()
        {
            return metadataList;
        }

        public int CompareTo(SchemaEntry other)
        {
            return this.NameCompareTo(other);
        }

        public int NameCompareTo(SchemaEntry other)
        {
            return String.Compare(this.defName, other.defName);
        }

        public int FullCompareTo(SchemaEntry other)
        {
            return this.metadataList.GetDiff(other.metadataList).Count;
        }

        private SchemaComparableList<AttributeMetadata> ConstructMetadataList(List<LdapValue> vals)
        {
            SchemaComparableList<AttributeMetadata> list = new SchemaComparableList<AttributeMetadata>();
            foreach (LdapValue v in vals)
            {
                list.Add(new AttributeMetadata(v.StringValue));
            }
            list.Sort();
            return list;
        }
    }
}

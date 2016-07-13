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
using VmDirInterop.Schema.Definitions;
using VmDirInterop.Schema.Exceptions;
using VmDirInterop.Schema.Utils;

namespace VmDirInterop.Schema.Entries
{
    public class SubSchemaSubEntry
    {
        private SchemaComparableList<AttributeType> attributeTypeList;
        private SchemaComparableList<ObjectClass> objectClassList;

        public SubSchemaSubEntry(ILdapEntry entry)
        {
            List<LdapValue> atVals = entry.getAttributeValues(AttributeConstants.ATTRIBUTE_TYPES);
            List<LdapValue> ocVals = entry.getAttributeValues(AttributeConstants.OBJECT_CLASSES);
            List<LdapValue> crVals = entry.getAttributeValues(AttributeConstants.DIT_CONTENT_RULES);

            attributeTypeList = ConstructAttributeTypeList(atVals);
            objectClassList = ConstructObjectClassList(ocVals, crVals);
        }

        public SubSchemaSubEntry(IList<AttributeType> attributeTypes, IList<ObjectClass> objectClasses)
        {
            attributeTypeList = new SchemaComparableList<AttributeType>(attributeTypes);
            objectClassList = new SchemaComparableList<ObjectClass>(objectClasses);
        }

        public SchemaComparableList<AttributeType> GetAttributeTypeList()
        {
            return attributeTypeList;
        }

        public SchemaComparableList<ObjectClass> GetObjectClassList()
        {
            return objectClassList;
        }

        private SchemaComparableList<AttributeType> ConstructAttributeTypeList(List<LdapValue> atVals)
        {
            SchemaComparableList<AttributeType> atList = new SchemaComparableList<AttributeType>();
            foreach (LdapValue v in atVals)
            {
                atList.Add(new AttributeType(v.StringValue));
            }
            atList.Sort();
            return atList;
        }

        private SchemaComparableList<ObjectClass> ConstructObjectClassList(List<LdapValue> ocVals, List<LdapValue> crVals)
        {
            SchemaComparableList<ObjectClass> ocList = new SchemaComparableList<ObjectClass>();
            SchemaComparableList<ContentRule> crList = new SchemaComparableList<ContentRule>();

            foreach (LdapValue v in ocVals)
            {
                ocList.Add(new ObjectClass(v.StringValue));
            }
            ocList.Sort();

            foreach (LdapValue v in crVals)
            {
                crList.Add(new ContentRule(v.StringValue));
            }
            crList.Sort();

            for (int i = 0, j = 0; i < ocList.Count && j < crList.Count; i++)
            {
                if (ocList[i].NameCompareTo(crList[j]) == 0)
                {
                    ocList[i].MergeContentRule(crList[j]);
                    j++;
                }
            }
            return ocList;
        }
    }
}

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

using VMDir.Common.VMDirUtilities;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;

namespace VMDir.Common.Schema
{
    public class SchemaManager
    {
        private const string ATTRIBUTETYPES = "attributetypes";
        private const string OBJECTCLASSES = "objectclasses";
        private const string DITCONTENTRULES = "ditcontentrules";

        private LdapConnectionService _conn;
        private AttributeTypeManager _attributeTypes = null;
        private ObjectClassManager _objectClasses = null;
        private ContentRuleManager _contentRules = null;

        public SchemaManager (LdapConnectionService conn)
        {
            _conn = conn;
        }

        public void RefreshSchema ()
        {
            const string baseDN = "cn=aggregate,cn=schemacontext";
            var attribs = new string[]{ ATTRIBUTETYPES, OBJECTCLASSES, DITCONTENTRULES};
            ILdapMessage ldMsg = null;
            try {
                List<ILdapEntry> response = _conn.SearchAndGetEntries (baseDN, LdapScope.SCOPE_SUBTREE, "(objectClass=*)", attribs, 0, ref ldMsg);
                if (response.Count == 0)
                    throw new Exception ("Failed to get schema");

                LdapEntry baseEntry = (LdapEntry)response [0];
                var dict = CollectData (baseEntry);
                ParseAttributes (dict [ATTRIBUTETYPES]);
                ParseObjectClasses (dict [OBJECTCLASSES]);
                ParseContentRules (dict [DITCONTENTRULES]);
            } catch (Exception) {
                throw;
            }
            finally
            {
                if (ldMsg != null)
                    (ldMsg as LdapMessage).FreeMessage();
            }

        }

        private Dictionary<string, List<string>> CollectData (LdapEntry baseEntry)
        {
            var dict = new Dictionary<string, List<string>> {
                { ATTRIBUTETYPES, new List<string> () },
                { OBJECTCLASSES, new List<string> () },
                { DITCONTENTRULES, new List<string> () }
            };

            foreach (var entry in dict) {
                //get attributes and values for each entry
                LdapValue[] val = baseEntry.getAttributeValues (entry.Key).ToArray ();// .GetAttributeValues(entry.Key);
                var list = entry.Value;
                int count = val.Count ();
                for (int i = 0; i < count; ++i)
                    list.Add (val [i].StringValue);
            }
            return dict;
        }

        private void ParseAttributes (List<string> items)
        {
            _attributeTypes = new AttributeTypeManager (items);
        }

        private void ParseObjectClasses (List<string> items)
        {
            _objectClasses = new ObjectClassManager (items);
        }

        private void ParseContentRules (List<string> items)
        {
            _contentRules = new ContentRuleManager (items);
        }

        public AttributeTypeDTO GetAttributeType (string key)
        {
            if (key == null)
                return null;
            AttributeTypeDTO dto = null;
            _attributeTypes.Data.TryGetValue (key, out dto);
            return dto;
        }

        public ObjectClassDTO GetObjectClass (string key)
        {
            if (key == null)
                return null;
            ObjectClassDTO dto = null;
            _objectClasses.Data.TryGetValue (key, out dto);
            return dto;
        }

        public List<AttributeTypeDTO> GetAttributes (string className, Func<ObjectClassDTO, List<string>> fnSelectAttribs)
        {
            var attributes = new List<AttributeTypeDTO> ();
            var dto = GetObjectClass (className);
            while (dto != null) {
                var list = fnSelectAttribs (dto);
                if (list != null)
                    list.ForEach (x => attributes.Add (GetAttributeType (x)));
                dto = GetObjectClass (dto.SuperClass);
            }
            return attributes;
        }

        public List<AttributeTypeDTO> GetRequiredAttributes (string className)
        {
            return GetAttributes (className, x => x.Must);
        }

        public List<AttributeTypeDTO> GetRequiredAttributesWithContentRules (string className)
        {
            var list = GetRequiredAttributes (className);
            var dto = GetContentRule (className);
            if (dto != null && dto.Must != null) {
                var dtoList = dto.Must.Select (x => GetAttributeType (x));
                list.AddRange (dtoList);
            }
            return list;
        }

        public List<AttributeTypeDTO> GetOptionalAttributes (string className)
        {
            return GetAttributes (className, x => x.May);
        }

        public ContentRuleDTO GetContentRule (string key)
        {
            if (key == null)
                return null;
            ContentRuleDTO dto = null;
            _contentRules.Data.TryGetValue (key, out dto);
            return dto;
        }

        public ObjectClassManager GetObjectClassManager ()
        {
            return _objectClasses;
        }
    }
}

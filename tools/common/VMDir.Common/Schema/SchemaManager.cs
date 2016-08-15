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

        private LdapConnectionService _conn;
        private AttributeTypeManager _attributeTypes = null;
        private ObjectClassManager _objectClasses = null;

        public SchemaManager(LdapConnectionService conn)
        {
            _conn = conn;
        }

        public void RefreshSchema()
        {
            const string baseDN = "cn=schemacontext";
            ILdapMessage ldMsg = null;
			try
			{
				List<ILdapEntry> attributesResponse = _conn.SearchAndGetEntries(baseDN, LdapScope.SCOPE_SUBTREE, "(objectclass=attributeschema)", null, 0, ref ldMsg);
				List<ILdapEntry> schemaResponse = _conn.SearchAndGetEntries(baseDN, LdapScope.SCOPE_SUBTREE, "(objectclass=classschema)", null, 0, ref ldMsg);

				if (attributesResponse.Count == 0 || schemaResponse.Count == 0)
					throw new Exception("Failed to get schema, possibly server version is incompatible.");

				var schemaDict = new Dictionary<string, Dictionary<string, object>>();
				var attrDict = new Dictionary<string, Dictionary<string, object>>();

				CollectData(schemaResponse, OBJECTCLASSES, schemaDict);
				CollectData(attributesResponse, ATTRIBUTETYPES, attrDict);
				ParseAttributes(attrDict);
				ParseObjectClasses(schemaDict);
			}
			finally
			{
				if (ldMsg != null)
					(ldMsg as LdapMessage).FreeMessage();
			}

        }

        private void CollectData(List<ILdapEntry> baseEntry, string attrib, Dictionary<string, Dictionary<string,object>> dict)
        {
            string entryDN = "";
            string entryName = "";

            foreach (var entry in baseEntry)
            {
                entryDN = entry.getDN();
                entryName = Utilities.DN2CN(entryDN);
                List<string> val = entry.getAttributeNames();
                Dictionary<string,object> localDict = new Dictionary<string, object>();
                foreach (string name in val)
                {
                    LdapValue[] vals = entry.getAttributeValues(name).ToArray();
                    localDict.Add(name, vals);
                }
                dict.Add(entryName, localDict);
            }
        }

        private void ParseAttributes(Dictionary<string, Dictionary<string,object>> items)
        {
            _attributeTypes = new AttributeTypeManager(items);
        }

        private void ParseObjectClasses(Dictionary<string, Dictionary<string,object>> items)
        {
            _objectClasses = new ObjectClassManager(items);
        }

        public AttributeTypeDTO GetAttributeType(string key)
        {
            if (key == null)
                return null;
            AttributeTypeDTO dto = null;
            _attributeTypes.Data.TryGetValue(key, out dto);
            return dto;
        }
        public bool isSingleValue(string key)
        {
            AttributeTypeDTO attrSynSTO = GetAttributeType(key);
            if (attrSynSTO != null)
                return attrSynSTO.SingleValue;
            else
                return false;
        }
        public ObjectClassDTO GetObjectClass(string key)
        {
            if (key == null)
                return null;
            ObjectClassDTO dto = null;
            _objectClasses.Data.TryGetValue(key, out dto);
            return dto;
        }

        private List<AttributeTypeDTO> GetAttributes(string className, Func<ObjectClassDTO, List<string>> fnSelectAttribs)
        {
            var attributes = new HashSet<AttributeTypeDTO>();
            var dto = GetObjectClass(className);
            while (dto != null)
            {
                var list = fnSelectAttribs(dto);
                if (list != null)
                {
                    foreach (var item in list)
                    {
                        var data = GetAttributeType(item);
                        if (data != null)
                            attributes.Add(data);
                    }
                }
                if (string.Equals(dto.SuperClass, dto.Name) != true)
                    dto = GetObjectClass(dto.SuperClass);
                else
                    break;

            }
            return attributes.ToList();
        }

        public List<AttributeTypeDTO> GetAllAttributes(string className)
        {
            var list = GetRequiredAttributes(className);
            var optionalList = GetOptionalAttributes(className);
            if (optionalList != null)
                list.AddRange(optionalList);
            return list;
        }

        public List<AttributeTypeDTO> GetRequiredAttributes(string className)
        {
            return GetAttributes(className, x => x.Must);
        }

        public List<AttributeTypeDTO> GetOptionalAttributes(string className)
        {
            return GetAttributes(className, x => x.May);
        }

        public ObjectClassManager GetObjectClassManager()
        {
            return _objectClasses;
        }

        public AttributeTypeManager GetAttributeTypeManager()
        {
            return _attributeTypes;
        }
    }
}

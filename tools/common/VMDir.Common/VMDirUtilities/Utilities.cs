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
using VMDir.Common.DTO;
using VMDirInterop.LDAP;
using VMDirInterop.Interfaces;
using VMDir.Common.Schema;

namespace VMDir.Common.VMDirUtilities
{
	public class VMDirBagItem
	{
		public object Value { get; set; }

		public bool IsReadOnly { get; set; }

		public bool IsRequired { get; set; }

		public string Description { get; set; }
	}

	public static class Utilities
	{
		//dont show attributes list
		public static HashSet<string> DontShowAttributes = new HashSet<string> {
			VMDirConstants.ATTR_OBJECT_SECURITY_DESCRIPTOR,
						  VMDirConstants.ATTR_OBJECT_SID
		};

		//returns LDAPValue  from string
		public static LdapValue StringToLdapValue(string stringVal)
		{
			LdapValue val = new LdapValue(stringVal);
			return val;
		}

		public static string[] SearchItemCN(string itemName, string searchObjectClass, string searchName, string[] attribs, VMDirServerDTO serverDTO)
		{
			ILdapMessage ldMsg = null;
			try
			{
				string filter = "(&(objectClass=" + searchObjectClass + ")(cn=" + searchName + "))";
				List<string> dn = new List<String>();
				List<ILdapEntry> dnEntries = serverDTO.Connection.SearchAndGetEntries(itemName, LdapScope.SCOPE_SUBTREE, filter, attribs, 0, ref ldMsg);
				foreach (LdapEntry entry in dnEntries)
				{
					dn.Add(entry.getDN());
				}
				return dn.ToArray();
			}
			catch (Exception e)
			{
				throw e;
			}
		}

		public static LdapValue[] FetchLdapValueFromAttributesDictionary(string attributeName, Dictionary<string, object> attributesDict)

		{
			LdapValue[] val = null;
			if (attributesDict.ContainsKey(attributeName))
			{
				val = attributesDict[attributeName] as LdapValue[];
			}
			return val;
		}

		public static string DN2CN(string dn)
		{
			if (string.IsNullOrEmpty(dn))
				return "";

			if (dn.StartsWith("dc"))
				return DN2DomainName(dn);
			else
				return dn.Split(',')[0].Split('=')[1];
		}

		public static string DN2DomainName(string dn)
		{
			return dn.Replace(",", ".").Replace("dc=", "");
		}

		//Remove specific System  Attributes from UI which are not editable
		public static void RemoveDontShowAttributes(Dictionary<string, VMDirAttributeDTO> properties)
		{
			foreach (string val in DontShowAttributes)
			{
				if (properties.ContainsKey(val))
					properties.Remove(val);
			}
		}

		public static List<AttributeDTO> ConvertToAttributeDTOList(Dictionary<string, VMDirAttributeDTO> _properties)
		{
			var attrList = new List<AttributeDTO>();
			foreach (var entry in _properties)
			{
				var valDTO = entry.Value;
				var attrTypeDTO = valDTO.AttrSyntaxDTO;
				if (attrTypeDTO == null)
					attrTypeDTO = new AttributeTypeDTO();
				foreach (var val in valDTO.Values)
					attrList.Add(new AttributeDTO(entry.Key, val.StringValue, attrTypeDTO));
			}
			attrList.Sort((x, y) => string.Compare(x.Name, y.Name, StringComparison.InvariantCultureIgnoreCase));
			return attrList;
		}

		public static string GetAttrLastVal(Dictionary<string, VMDirAttributeDTO> properties, string attr)
		{
			try
			{
				if (!properties.ContainsKey(attr))
					throw new Exception(VMDirConstants.ERR_FETCH_DATA);
				List<LdapValue> ocVal = properties[attr].Values;
				if (ocVal != null)
					return ocVal[(ocVal.Count - 1)].StringValue;
				else
					return string.Empty;
			}
			catch (Exception)
			{
				return string.Empty;
			}
		}

		public static string GetObjectClass(ILdapEntry entry)
		{
			var values = entry.getAttributeValues(VMDirConstants.ATTR_OBJECT_CLASS);
			return values[(values.Count - 1)].StringValue;
		}

		public static LdapMod MakeAttribute(KeyValuePair<string, VMDirAttributeDTO> entry)
		{
			List<LdapValue> entryVal = entry.Value.Values;
			List<string> vals = new List<string>() { null };
			if (entryVal != null)
			{
				vals = entryVal.ConvertAll(x => x.StringValue);
				vals.Add(null);
			}
			return new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, entry.Key, vals.ToArray());
		}
	}
}


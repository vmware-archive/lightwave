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

using System;
using System.Collections.Generic;
using VMDir.Common.DTO;
using VMDirInterop.LDAP;
using VMDirInterop.Interfaces;
using VMDir.Common.Schema;
using System.Text;
using System.Linq;

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
					attrList.Add(new AttributeDTO(entry.Key, val.StringValue, attrTypeDTO,false));
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

        public static List<string> GetObjectClassList(VMDirServerDTO ServerDTO, string searchBase, LdapScope searchScope)
        {
            QueryDTO qdto = new TextQueryDTO(searchBase, searchScope, VMDirConstants.SEARCH_ALL_OC, null, 0, IntPtr.Zero, 0);
            var ocList = new List<string>();
            ServerDTO.Connection.Search(qdto,
                 (l, e) =>
                 {
                     if (e.Count > 0)
                         ocList = new List<string>(e[0].getAttributeValues(VMDirConstants.ATTR_OBJECT_CLASS).Select(x => x.StringValue).ToArray());
                 });
            return ocList;
        }

        public static string ConvertGeneralizedTimeIntoReadableFormat(string gt)
        {
            try
            {
                int century=-1, year=-1, month=-1, day=-1, hour=-1, minute=-1, second=-1;
                string zone=string.Empty;
                string leftPart=string.Empty;
                string rightPart = string.Empty;

                if (gt.Contains("Z"))
                {
                    var parts = gt.Split('Z');
                    leftPart = parts[0];
                    rightPart = parts[1];
                    zone = "GMT";
                }
                else if (gt.Contains("-"))
                {
                    var parts = gt.Split('-');
                    leftPart = parts[0];
                    rightPart = parts[1];
                }
                else if (gt.Contains("+"))
                {
                    var parts = gt.Split('+');
                    leftPart = parts[0];
                    rightPart = parts[1];
                }

                if (rightPart.Length >= 2)
                {
                    StringBuilder sb = new StringBuilder();
                    sb.Append("GMT");
                    if (gt.Contains("+"))
                        sb.Append("+");
                    else if (gt.Contains("-"))
                        sb.Append("-");
                    sb.Append(rightPart.Substring(0, 2));
                    if (rightPart.Length >= 4)
                        sb.Append(":" + rightPart.Substring(2, 2));
                    zone = sb.ToString();
                }

                if (leftPart.Contains("."))
                {
                    var parts = gt.Split('.');
                    leftPart = parts[0];
                }
                if (leftPart.Length >= 10)
                {
                    century=int.Parse(leftPart.Substring(0, 2));
                    year=int.Parse(leftPart.Substring(2, 2));
                    month = int.Parse(leftPart.Substring(4, 2));
                    day = int.Parse(leftPart.Substring(6, 2));
                    hour = int.Parse(leftPart.Substring(8, 2));

                    if(leftPart.Length>=12)
                        minute = int.Parse(leftPart.Substring(10, 2));
                    if (leftPart.Length >= 14)
                        second = int.Parse(leftPart.Substring(12, 2));
                }
				if (day < 0 || month < 0 || century < 0 || year < 0 || hour < 0)
					return gt;
                StringBuilder sb2 = new StringBuilder();
                sb2.Append(day);
                sb2.Append("/");
                sb2.Append(month);
                sb2.Append("/");
                sb2.Append(century);
                sb2.Append(year);
                sb2.Append(", ");
                sb2.Append(hour);
                if(minute>=0)
                {
                    sb2.Append(":");
                    sb2.Append(minute);
                    if (second >= 0)
                        sb2.Append(":" + second);
                }
                sb2.Append(" ");
                sb2.Append(zone);
                return sb2.ToString();
            }
            catch (Exception ex)
            {
                return gt;
            }
        }
	}
}


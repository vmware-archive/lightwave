/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using System.Collections.Generic;
using AppKit;
using Foundation;
using System.Linq;
using VMDir.Common.VMDirUtilities;
using VMDirInterop.LDAP;
using VMDir.Common.DTO;
using VMDir.Common.Schema;
using VMDir.Common;

namespace VMDirSnapIn.DataSource
{
	public class PropertiesTableViewDataSource : NSTableViewDataSource
	{
		public Dictionary<string, VMDirAttributeDTO> properties { get; set; }
		public List<AttributeDTO> currAttrDTOList;
		public List<AttributeDTO> optAttrDTOList;
		public List<AttributeDTO> oprAttrDTOList;
		public HashSet<string> modData;
		private List<AttributeTypeDTO> mayAttrDTOList;
		public List<AttributeDTO> displayAttrDTOList;
		private string objectClass = string.Empty;
		public string dn = string.Empty;
		public VMDirServerDTO serverDTO;

		public PropertiesTableViewDataSource()
		{
			properties = new Dictionary<string, VMDirAttributeDTO>();
			currAttrDTOList = new List<AttributeDTO>();
			optAttrDTOList = new List<AttributeDTO>();
			oprAttrDTOList = new List<AttributeDTO>();
			mayAttrDTOList = new List<AttributeTypeDTO>();
			modData = new HashSet<string>();
			displayAttrDTOList = new List<AttributeDTO>();
		}

		public PropertiesTableViewDataSource(string dn, string oc, VMDirServerDTO serverDTO, Dictionary<string, VMDirAttributeDTO> classList) : this()
		{
			this.dn = dn;
			this.objectClass = oc;
			this.serverDTO = serverDTO;
			properties = classList;
			FillData();
		}

		public void FillData()
		{
			oprAttrDTOList.Clear();
			currAttrDTOList.Clear();
			displayAttrDTOList.Clear();
			optAttrDTOList.Clear();

			if (serverDTO.OperationalAttrFlag)
			{
				GetOperationalAttribute();
				displayAttrDTOList.AddRange(oprAttrDTOList);
			}

			currAttrDTOList = Utilities.ConvertToAttributeDTOList(properties);
			displayAttrDTOList.AddRange(currAttrDTOList);

			if (serverDTO.OptionalAttrFlag)
			{
				if (string.IsNullOrWhiteSpace(objectClass))
					objectClass = Utilities.GetAttrLastVal(properties, VMDirConstants.ATTR_OBJECT_CLASS);
				mayAttrDTOList = serverDTO.Connection.SchemaManager.GetOptionalAttributes(objectClass);
				foreach (var item in mayAttrDTOList)
					if (item != null)
						optAttrDTOList.Add(new AttributeDTO(item.Name, string.Empty, item,false));
				foreach (var item in currAttrDTOList)
					if (item.AttrSyntaxDTO.SingleValue)
						optAttrDTOList.RemoveAll(x => x.Name.Equals(item.Name));
				optAttrDTOList.Sort((x, y) => string.Compare(x.Name, y.Name, StringComparison.InvariantCultureIgnoreCase));
				displayAttrDTOList.AddRange(optAttrDTOList);
			}
		}

		private void GetOperationalAttribute()
		{
			TextQueryDTO dto = new TextQueryDTO(dn, LdapScope.SCOPE_BASE, VMDirConstants.SEARCH_ALL_OC,
												new string[] { "+" }, 0, IntPtr.Zero, 0);
			var operationalProperties = new Dictionary<string, VMDirAttributeDTO>();
			serverDTO.Connection.Search(dto, (l, e) =>
			{
				if (e.Count > 0)
					operationalProperties = serverDTO.Connection.GetEntryProperties(e[0]);
			});
			oprAttrDTOList = Utilities.ConvertToAttributeDTOList(operationalProperties);
		}

		public void ReloadData()
		{
			TextQueryDTO dto = new TextQueryDTO(dn, LdapScope.SCOPE_BASE, VMDirConstants.SEARCH_ALL_OC, null, 0, IntPtr.Zero, 0);

			serverDTO.Connection.Search(dto,
				(l, e) =>
				{
					if (e.Count > 0)
					{
						dn = e[0].getDN();
						properties = serverDTO.Connection.GetEntryProperties(e[0]);
					}
				});

			FillData();
		}

		public override nint GetRowCount(NSTableView tableView)
		{
			if (displayAttrDTOList != null)
				return displayAttrDTOList.Count;
			else
				return 0;
		}
	}
}
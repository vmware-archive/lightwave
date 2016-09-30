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
using System.Threading.Tasks;
using System.Xml.Serialization;
using VMDir.Common.VMDirUtilities;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;
using VMIdentity.CommonUtils;

namespace VMDir.Common.DTO
{
    public class VMDirServerDTO
    {
        public string Server { get; set; }
        public string BaseDN { get; set; }
        public string BindDN { get; set; }
        [XmlIgnore]
        public string Password { get; set; }
        public string GUID { get; set; }
        [XmlIgnore]
        public LdapConnectionService Connection { get; set; }

        private int _pageSize;
        public int PageSize { 
            get {
                if (_pageSize <= 0 || _pageSize > VMDirConstants.DEFAULT_PAGE_SIZE * 10)
                    return VMDirConstants.DEFAULT_PAGE_SIZE;
                else
                    return _pageSize;
            }
            set
            {
                _pageSize = value;
            }
        }
		public bool OperationalAttrFlag { get; set; }
        public bool OptionalAttrFlag { get; set; }
        [XmlIgnore]
        public bool IsLoggedIn { get; set; }
        public  static VMDirServerDTO CreateInstance ()
        {

            var dto = new VMDirServerDTO { GUID = Guid.NewGuid().ToString(), _pageSize = VMDirConstants.DEFAULT_PAGE_SIZE, OperationalAttrFlag = false, OptionalAttrFlag = false, IsLoggedIn=false };
            return dto;
        }

        public static string DN2CN (string dn)
        {
            if (string.IsNullOrEmpty (dn))
                return "";

            if (dn.StartsWith ("dc"))
                return DN2DomainName (dn);
            else
                return dn.Split (',') [0];
        }

        private static string DN2DomainName (string dn)
        {
            return dn.Replace (",", ".").Replace ("dc=", "");
        }

		public async Task DoLogin()
		{
			try
			{
				Task t = new Task(ServerConnect);
				t.Start();
				if (await Task.WhenAny(t, Task.Delay(CommonConstants.TEN_SEC * 3)) == t)
				{
					await t;
				}
				else {
					throw new Exception(CommonConstants.SERVER_TIMEOUT);
				}
			}
			catch (Exception ex)
			{
				throw ex;
			}
		}

		public void ServerConnect()
		{
			try
			{
				Connection = new LdapConnectionService(Server, BindDN, Password);

				if (Connection.CreateConnection() == 1)
				{
					if (string.IsNullOrWhiteSpace(BaseDN))
					{
						TextQueryDTO dto = new TextQueryDTO("", LdapScope.SCOPE_BASE, VMDirConstants.SEARCH_ALL_OC,
							new string[] { VMDirConstants.ATTR_ROOT_DOMAIN_NAMING_CONTEXT }, 0, IntPtr.Zero, 0);
						try
						{
							Connection.Search(dto,
								delegate (ILdapMessage searchRequest, List<ILdapEntry> entries)
								{
									BaseDN = GetRootDomainNamingContext(entries);
								});
						}
						catch (Exception)
						{
							throw new Exception(VMDirConstants.ERR_DN_RETRIEVAL);
						}
					}
					else
					{
						TextQueryDTO dto = new TextQueryDTO(BaseDN, LdapScope.SCOPE_BASE, VMDirConstants.SEARCH_ALL_OC,
							new string[] { VMDirConstants.ATTR_DN }, 0, IntPtr.Zero, 0);
						Connection.Search(dto, null);
					}
					IsLoggedIn = true;
				}
				else {
					IsLoggedIn = false;
				}
			}
			catch (Exception)
			{
				IsLoggedIn = false;
				throw;
			}
		}
		private string GetRootDomainNamingContext(List<ILdapEntry> entries)
		{
			if (entries != null)
			{
				var value = entries[0].getAttributeValues(VMDirConstants.ATTR_ROOT_DOMAIN_NAMING_CONTEXT);
				if (value != null && value.Count > 0)
					return value[0].StringValue;
			}
			return string.Empty;
		}

    }

}


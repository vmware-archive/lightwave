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
using VMDirInterop.LDAP;

namespace VMDir.Common.DTO
{
    public class TextQueryDTO : QueryDTO
    {
        public string FilterString { get; set; }
        public TextQueryDTO() { }
        public TextQueryDTO(string searchBase, LdapScope searchScope, string filterString, string[] attrToReturn,
            int attrOnly, IntPtr timeOut, int sizeLimit)
        {
            this.SearchBase = searchBase;
            this.SearchScope = searchScope;
            this.AttrToReturn = attrToReturn;
            this.AttrOnly = attrOnly;
            this.TimeOut = timeOut;
            this.SizeLimit = sizeLimit;
            this.FilterString = filterString;
        }
        public override string GetFilterString()
        {
            return FilterString;
        }
    }
}

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
using System.Text;
using VMDirInterop.LDAP;

namespace VMDir.Common.DTO
{
    public class BuildQueryDTO : QueryDTO
    {
        public LogicalOp Operator { get; set; }
        public List<FilterDTO> CondList { get; set; }
        public BuildQueryDTO() { }
        public BuildQueryDTO(string searchBase, LdapScope searchScope, LogicalOp op, List<FilterDTO> condList,

            string[] attrToReturn, int attrOnly, IntPtr timeOut, int sizeLimit)
        {
            this.SearchBase = searchBase;
            this.SearchScope = searchScope;
            this.AttrToReturn = attrToReturn;
            this.AttrOnly = attrOnly;
            this.TimeOut = timeOut;
            this.SizeLimit = sizeLimit;
            this.Operator = op;
            this.CondList = condList;
        }
        public override string GetFilterString()
        {
            StringBuilder conditionsSB = new StringBuilder();
            if (CondList != null)
            {
                foreach (var filterDTO in CondList)
                {
                    string cond = filterDTO.ToString();
                    conditionsSB.Append(cond);
                }
                string filterString = "{0}{1}{2}{3}";

                if (CondList.Count > 1)
                {
                    string opChar = string.Empty;
                    if (Operator == LogicalOp.AND)
                        opChar = "&";
                    else if (Operator == LogicalOp.OR)
                        opChar = "|";
                    filterString = String.Format(filterString, "(", opChar, conditionsSB.ToString(), ")");
                }
                else
                {
                    filterString = String.Format(filterString, "", "", conditionsSB.ToString(), "");
                }
                return filterString;
            }
            return string.Empty;
        }
    }
}

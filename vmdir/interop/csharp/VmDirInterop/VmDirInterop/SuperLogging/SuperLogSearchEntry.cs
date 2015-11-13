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

namespace VmDirInterop.SuperLogging
{
    public class SuperLogSearchEntry : SuperLogEntry
    {
        private LDAP_SEARCH_INFO _searchInfo;

        public SuperLogSearchEntry(VMDIR_SUPERLOG_ENTRY_LDAPOPERATION entry) : base(entry)
        {
            _searchInfo = entry.opInfo.searchInfo;
        }

        public String getAttributes()
        {
            return _searchInfo.pwszAttributes;
        }

        public String getBaseDN()
        {
            return _searchInfo.pwszBaseDN;
        }

        public String getScope()
        {
            return _searchInfo.pwszScope;
        }

        public String getIndexResults()
        {
            return _searchInfo.pwszIndexResults;
        }

        public UInt32 getNumScanned()
        {
            return _searchInfo.dwScanned;
        }

        public UInt32 getNumReturned()
        {
            return _searchInfo.dwReturned;
        }

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendFormat("LOGIN_DN: {0}\n", getLoginDN());
            sb.AppendFormat("\tIP: {0}\n", getClientIP());
            sb.AppendFormat("\tPORT: {0}\n", getClientPort());
            sb.AppendFormat("\tOPERATION: {0}\n", getOperation());
            sb.AppendFormat("\t\tSTRING: {0}\n", getString());
            sb.AppendFormat("\t\tATTRIBUTE(S): {0}\n", getAttributes());
            sb.AppendFormat("\t\tBASE_DN: {0}\n", getBaseDN());
            sb.AppendFormat("\t\tSCOPE: {0}\n", getScope());
            sb.AppendFormat("\t\tFILTER(S): {0}\n", getIndexResults());
            sb.AppendFormat("\t\tSCANNED: {0}\n", getNumScanned());
            sb.AppendFormat("\t\tRETURNED: {0}\n", getNumReturned());
            sb.AppendFormat("\tERROR_CODE: {0}\n", getErrorCode());
            sb.AppendFormat("\tTIME: {0}\n", getEndTime() - getStartTime());
            sb.Append("--------------------\n");
            return sb.ToString();
        }
    }
}

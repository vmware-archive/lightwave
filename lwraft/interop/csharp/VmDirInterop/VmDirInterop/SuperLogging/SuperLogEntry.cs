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
using VmDirInterop.SuperLogging.Interfaces;
using System.Runtime.InteropServices;

namespace VmDirInterop.SuperLogging
{
    public class SuperLogEntry : ISuperLogEntry
    {
        private VMDIR_SUPERLOG_ENTRY_LDAPOPERATION _entry;

        public SuperLogEntry(VMDIR_SUPERLOG_ENTRY_LDAPOPERATION entry)
        {
            _entry = entry;
        }

        public String getLoginDN()
        {
            return _entry.pwszLoginDN;
        }

        public String getClientIP()
        {
            return _entry.pwszClientIP;
        }

        public String getServerIP()
        {
            return _entry.pwszServerIP;
        }

        public String getOperation()
        {
            return _entry.pwszOperation;
        }

        public String getString()
        {
            return _entry.pwszString;
        }

        public UInt32 getClientPort()
        {
            return _entry.dwClientPort;
        }

        public UInt32 getServerPort()
        {
            return _entry.dwServerPort;
        }

        public UInt32 getErrorCode()
        {
            return _entry.dwErrorCode;
        }

        public UInt64 getStartTime()
        {
            return _entry.iStartTime;
        }

        public UInt64 getEndTime()
        {
            return _entry.iEndTime;
        }

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendFormat("LOGIN_DN: {0}\n", getLoginDN());
            sb.AppendFormat("\tIP: {0}\n", getClientIP());
            sb.AppendFormat("\tPORT: {0}\n", getClientPort());
            sb.AppendFormat("\tOPERATION: {0}\n", getOperation());
            if (!String.IsNullOrEmpty(getString()))
            {
                sb.AppendFormat("\t\tSTRING: {0}\n", getString());
            }
            sb.AppendFormat("\tERROR_CODE: {0}\n", getErrorCode());
            sb.AppendFormat("\tTIME: {0}\n", getEndTime() - getStartTime());
            sb.Append("--------------------\n");
            return sb.ToString();
        }
    }
}

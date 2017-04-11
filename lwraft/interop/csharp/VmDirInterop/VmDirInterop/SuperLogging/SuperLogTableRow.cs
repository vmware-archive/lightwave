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
using VmDirInterop.SuperLogging.Constants;
using VmDirInterop.SuperLogging.Interfaces;

namespace VmDirInterop.SuperLogging
{
    public class SuperLogTableRow : ISuperLogTableRow
    {
        private VMDIR_SUPERLOG_TABLE_ROW _row;

        public SuperLogTableRow(VMDIR_SUPERLOG_TABLE_ROW row)
        {
            _row = row;
        }

        public string getLoginDN()
        {
            return _row.colVals[(int)SuperLogTableColumn.LOGIN_DN];
        }

        public string getIP()
        {
            return _row.colVals[(int)SuperLogTableColumn.IP];
        }

        public string getPort()
        {
            return _row.colVals[(int)SuperLogTableColumn.PORT];
        }

        public string getOperation()
        {
            return _row.colVals[(int)SuperLogTableColumn.OPERATION];
        }

        public string getString()
        {
            return _row.colVals[(int)SuperLogTableColumn.STRING];
        }

        public string getErrorCode()
        {
            return _row.colVals[(int)SuperLogTableColumn.ERROR_CODE];
        }

        public string getAvgTime()
        {
            return _row.colVals[(int)SuperLogTableColumn.AVG_TIME];
        }

        public ulong getAvgTimeULong()
        {
            return _row.totalTime / _row.count;
        }

        public uint getCount()
        {
            return _row.count;
        }

        public override string ToString()
        {
            string retval = "|";
            if (getLoginDN() != null)
            {
                retval += getLoginDN() + "|";
            }
            if (getIP() != null)
            {
                retval += getIP() + "|";
            }
            if (getPort() != null)
            {
                retval += getPort() + "|";
            }
            if (getOperation() != null)
            {
                retval += getOperation() + "|";
            }
            if (getString() != null)
            {
                retval += getString() + "|";
            }
            if (getErrorCode() != null)
            {
                retval += getErrorCode() + "|";
            }
            if (getAvgTime() != null)
            {
                retval += getAvgTime() + "|";
            }
            retval += getCount() + "|";
            return retval;
        }
    }
}

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
using System.Runtime.InteropServices;

namespace VmDirInterop.SuperLogging
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public struct LDAP_SEARCH_INFO
    {
        [MarshalAs(UnmanagedType.LPWStr)]
        public String pwszAttributes;
        [MarshalAs(UnmanagedType.LPWStr)]
        public String pwszBaseDN;
        [MarshalAs(UnmanagedType.LPWStr)]
        public String pwszScope;
        [MarshalAs(UnmanagedType.LPWStr)]
        public String pwszIndexResults;
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 dwScanned;
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 dwReturned;
    }

    [StructLayout(LayoutKind.Explicit, CharSet = CharSet.Auto)]
    public struct LDAP_OPERATION_INFO
    {
        [FieldOffset(0)]
        public LDAP_SEARCH_INFO searchInfo;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public struct VMDIR_SUPERLOG_ENTRY_LDAPOPERATION
    {
        [MarshalAs(UnmanagedType.LPWStr)]
        public String pwszLoginDN;
        [MarshalAs(UnmanagedType.LPWStr)]
        public String pwszClientIP;
        [MarshalAs(UnmanagedType.LPWStr)]
        public String pwszServerIP;
        [MarshalAs(UnmanagedType.LPWStr)]
        public String pwszOperation;
        [MarshalAs(UnmanagedType.LPWStr)]
        public String pwszString;
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 dwClientPort;
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 dwServerPort;
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 dwErrorCode;
        [MarshalAs(UnmanagedType.U8)]
        public UInt64 iStartTime;
        [MarshalAs(UnmanagedType.U8)]
        public UInt64 iEndTime;
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 opType;
        public LDAP_OPERATION_INFO opInfo;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY
    {
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 dwCount;
        public IntPtr entries;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VMDIR_SUPERLOG_TABLE_COLUMN_SET
    {
        [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U4, SizeConst = 7)]
        public UInt32[] isColumnSet;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public struct VMDIR_SUPERLOG_TABLE_ROW
    {
        [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.LPStr, SizeConst = 7)]
        public string[] colVals;
        [MarshalAs(UnmanagedType.U8)]
        public UInt64 totalTime;
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 count;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VMDIR_SUPERLOG_TABLE
    {
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 numRows;
        public IntPtr cols;
        public IntPtr rows;
    }
}

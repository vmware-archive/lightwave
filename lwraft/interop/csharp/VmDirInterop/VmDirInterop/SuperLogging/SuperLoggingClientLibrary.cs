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
    class SuperLoggingClientLibrary
    {
        public const string CLIENT_DLL = "libvmdirclient.dll";

        public static void SetPath(string path)
        {
            SetDllDirectory(path);
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool SetDllDirectory(string lpPathName);

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirOpenServerA")]
        public static extern UInt32 VmDirOpenServerA(
            string networkAddress,
            string userName,
            string domain,
            string password,
            UInt32 flags,
            IntPtr reserved,
            out IntPtr serverContext
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirOpenServerW")]
        public static extern UInt32 VmDirOpenServerW(
            string networkAddress,
            string userName,
            string domain,
            string password,
            UInt32 flags,
            IntPtr reserved,
            out IntPtr serverContext
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirCloseServer")]
        public static extern UInt32 VmDirCloseServer(
            IntPtr serverContext
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirSuperLogQueryServerData")]
        public static extern UInt32 VmDirSuperLogQueryServerData(
            IntPtr context,
            out IntPtr serverData
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirSuperLogEnable")]
        public static extern UInt32 VmDirSuperLogEnable(
            IntPtr context
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirSuperLogDisable")]
        public static extern UInt32 VmDirSuperLogDisable(
            IntPtr context
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirIsSuperLogEnabled")]
        public static extern UInt32 VmDirIsSuperLogEnabled(
            IntPtr context,
            out byte enabled
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirSuperLogFlush")]
        public static extern UInt32 VmDirSuperLogFlush(
            IntPtr context
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirSuperLogSetSize")]
        public static extern UInt32 VmDirSuperLogSetSize(
            IntPtr context,
            UInt32 size
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirSuperLogGetSize")]
        public static extern UInt32 VmDirSuperLogGetSize(
            IntPtr context,
            out UInt32 size
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirSuperLogGetEntriesLdapOperation")]
        public static extern UInt32 VmDirSuperLogGetEntriesLdapOperation(
            IntPtr context,
            IntPtr enumerationCookie,
            UInt32 count,
            out IntPtr entries
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirSuperLogGetTable")]
        public static extern UInt32 VmDirSuperLogGetTable(
            IntPtr entries,
            IntPtr columnSet,
            out IntPtr table
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirFreeSuperLogEntryLdapOperationArray")]
        public static extern UInt32 VmDirFreeSuperLogEntryLdapOperationArray(
            IntPtr entries
            );

        [DllImport(CLIENT_DLL, EntryPoint = "VmDirFreeSuperLogTable")]
        public static extern UInt32 VmDirFreeSuperLogTable(
            IntPtr table
            );
    }
}

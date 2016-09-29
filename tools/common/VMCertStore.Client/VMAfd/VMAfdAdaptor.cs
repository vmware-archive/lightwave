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
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace VMAfd
{
    public enum VMAfdStatus
    {
        Unknown = 0,
        Initializing,
        Paused,
        Running,
        Stopping,
        Stopped,
    }

    public class VMAfdAdaptor
    {
        public static UInt32 SUCCESS = 0;

        const string LIBNAME = @"libvmafdclient.dll";

        [DllImport (LIBNAME)]
        public static extern UInt32
            VmAfdGetStatusA (
            string serverName,
            out VMAfdStatus status);

        [DllImport (LIBNAME)]
        public static extern UInt32
            VmAfdGetDomainNameA (
            string serverName,
            out IntPtr domain);

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdSetDomainNameA (
            string pszServerName,
            string pszDomain
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdGetLDUA (
            string pszServerName,
            out IntPtr ppszLDU
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdSetLDUA (
            string pszServerName,
            string pszLDU
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdGetCMLocationA (
            string pszServerName,
            out IntPtr ppszCMLocation
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdSetCMLocationA (
            string pszServerName,
            string pszCMLocation
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdGetDCNameA (
            string pszServerName,
            out IntPtr ppszDCName
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdSetDCNameA (
            string pszServerName,
            string pszDCName
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdAddPasswordEntryA (
            string pszServerName,
            string pszUPN,
            string pszPassword
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdSetPasswordEntryA (
            string pszServerName,
            string pszUPN,
            string pszPassword
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdDeletePasswordEntryA (
            string pszServerName,
            string pszUPN
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdGetMachineAccountInfoA (
            string pszServerName,
            out IntPtr ppszAccount,
            out IntPtr ppszPassword
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdSetMachineSSLCertificateA (
            string pszServerName,
            string pszPrivateKey,
            IntPtr pPublicKey,
            UInt32 dwKeyLength
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdGetMachineSSLCertificateA (
            string  pszServerName,
            out IntPtr  ppszPrivateKey,
            out IntPtr ppPublicKey,
            out UInt32 pdwKeyLength
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdJoinVMwareDirectoryA (
            string pszServerName,
            string pszDomainName,
            string pszOU,
            string pszUserName,
            string pszPassword
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdJoinActiveDirectoryA (
            string pszServerName,
            string pszDomainName,
            string pszOU,
            string pszUserName,
            string pszPassword
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdVdcSplitA (
            string serverName,
            string sourceUserName,
            string sourcePassword,
            string targetDomain,
            string targetUserName,
            string targetPassword
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdVdcMergeA (
            string serverName,
            string sourceUserName,
            string sourcePassword,
            string targetHost,
            string targetUserName,
            string targetPassword
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdForceReplicationA (
            string serverName
        );

        [DllImport (LIBNAME)]
        public static extern String
        VmAfdGetErrorMessageA (
            UInt32 errorCode
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdOpenServerA (
            string serverName,
            string userName,
            string password,
            out IntPtr ppServer);

        [DllImport(LIBNAME)]
        public static extern UInt32
        VmAfdCloseServer(IntPtr pServer);
    }
}

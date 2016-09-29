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

namespace VMCertStore
{
    public class VMCertStoreAdaptor
    {
        public static UInt32 SUCCESS = 0;

        [StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public class VMAFD_CERT_CONTAINER
        {
            public UInt32 dwStoreType;
            public IntPtr pCert;
            public IntPtr pAlias;
            public IntPtr pPassword;
            public IntPtr pPrivateKey;
        }

        [StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public class VMAFD_CERT_ARRAY
        {
            public UInt32 dwCount;
            public IntPtr certificates;
        }


        const string LIBNAME = @"/usr/lib/vmware-vmafd/lib64/libvmafdclient.dylib";
        //Cert
        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdRpcEnumCertificates (
            UInt32 hBinding,
            UInt32 hStore,
            UInt32 dwStartIndex,
            UInt32 dwNumCertificates,
            VMAFD_CERT_CONTAINER ppCertContainer
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdOpenCertStore (
            string pszServerName,
            UInt32 type,
            out UInt32 hStore);

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdCloseCertStore (
            string pszServerName,
            UInt32 hStore);

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdAddCertificate (
            string pszServerName,
            UInt32 hStore,
            string pszAlias,
            string pszCertificate,
            string pszPrivateKey,
            UInt32 uAutoRefresh);

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdDeleteCertificate (
            string pszServerName,
            UInt32 hStore,
            string pszCertificate);

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdVerifyCertificateTrust (
            string pszServerName,
            string pszCertificate);

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdGetCertificateChain (
            string pszServerName,
            string pszCertificate,
            out IntPtr ppszCertChain);

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfGetCertificateByAlias (
            string pszServerName,
            UInt32 hStore,
            string pszAlias,
            out IntPtr ppszCertificate,
            out IntPtr ppszPrivateKey);

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdEnumCertificates (
            string pszServerName,
            UInt32 storeType,
            UInt32 startIndex,
            UInt32 numCertificates,
            out IntPtr ppCerts
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdFreeCertArray (
            IntPtr ppCerts
        );
    }
}


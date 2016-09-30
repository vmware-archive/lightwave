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
using System.Runtime.InteropServices;

namespace Vecs
{
    public class VecsAdaptor
    {
        public static UInt32 SUCCESS = 0;
        public static UInt32 ERROR_NO_MORE_ITEMS = 259;

        public enum CertEntryType
        {
            Unknown = 0,
            PrivateKey,
            SecretKey,
            TrustedCert,
            RevokedCert,
            EncryptedPrivateKey
        }

        public enum EntryInfoLevel
        {
            Undefined = 0,
            Level1,
            Level2
        }


        [StructLayout (LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public class VECS_CERT_ENTRY
        {
            public UInt32 entryType;
            public IntPtr pszAlias;
            public IntPtr pszCertificate;
            public IntPtr pszPassword;
        };

        const string LIBNAME = @"libvmafdclient.dll";

        [DllImport (LIBNAME)]
        public static extern UInt32
        VecsGetEntryCount (
            IntPtr hStore,
            out UInt32 pdwCount
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VecsEnumCertStoreHA (
            IntPtr pServer,
            out IntPtr ppszStoreNameArray,
            out UInt32 pdwCount
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VecsCreateCertStoreHA (
            IntPtr pServer,
            string pszStoreName,
            string pszPassword,
            out IntPtr hStore
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VecsOpenCertStoreHA (
            IntPtr pServer,
            string pszStoreName,
            string pszPassword,
            out IntPtr hStore
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VecsCloseCertStore (
            IntPtr hStore);

        [DllImport (LIBNAME)]
        public static extern UInt32
        VecsBeginEnumEntries (
            IntPtr pStore,
            UInt32 dwEntryCount,
            EntryInfoLevel infoLevel,
            out IntPtr ppEnumContext
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VecsEnumEntriesA (
            IntPtr hEnumContext,
            out IntPtr ppEntries,
            out UInt32 pdwEntryCount
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VecsEndEnumEntries (
            IntPtr hEnumContext
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VecsAddEntryA (
            IntPtr hStore,
            CertEntryType entryType,
            string pszAlias,
            string pszCertificate,
            string pszPrivateKey,
            string pszPassword,
            bool bAutoRefresh
        );

        [DllImport (LIBNAME)]
        public static extern void
        VecsFreeCertEntryArrayA (
            IntPtr pCertEntryArray,
            UInt32 dwCount
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VecsDeleteCertificateA (
            IntPtr hStore,
            string pszAlias
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VecsDeleteEntryA (
            IntPtr hStore,
            string pszAlias
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VecsDeleteCertStoreHA (
            IntPtr pServer,
            string pszStoreName
        );

        [DllImport (LIBNAME)]
        public static extern UInt32
        VmAfdGetErrorMsgByCode (
            UInt32 dwErrorCode,
            out IntPtr errorMsg
        );
    }
}


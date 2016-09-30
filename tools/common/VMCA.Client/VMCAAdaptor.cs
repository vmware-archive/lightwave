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
using System.Runtime.InteropServices;

namespace VMCA
{
    public enum CertificateState
    {
        Active = 0,
        Revoked = 1,
        Expired = 2,
        All = 4
    }

    public enum VMCAEnum
    {
        Success = 0,
        End = 1,
        Error = 2
    }

    public class VMCAAdaptor
    {
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public class VMCA_PKCS_10_REQ_DATA
        {
            public string pszName;
            public string pszDomainName;
            public string pszCountry;
            public string pszLocality;
            public string pszState;
            public string pszOrganization;
            public string pszOU;
            public string pszDNSName;
            public string pszURIName;
            public string pszEmail;
            public string pszIPAddress;
            public UInt32 dwKeyUsageConstraints;
        }

        public const UInt32 VMCA_SUCCESS = 0;
        public const UInt32 VMCA_ENUM_END = 1;
        const string LIBNAME = @"libvmcaclient.dll";

        [DllImportAttribute(LIBNAME)]
        public static extern UInt32
            VMCAOpenServerA(
            string serverName,
            string userName,
            string domainName,
            string password,
            UInt32 dwFlags,
            IntPtr pReserved,
            out IntPtr ppServerContext
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
            VMCACloseServer(
            IntPtr pServerContext
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
            VMCAAddRootCertificate(
            string pszServerName,
            string pszRootCertificate,
            string pszPassPhrase,
            string pszPrivateKey
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
            VMCAGetRootCACertificateHA(
            IntPtr pServerContext,
            string argServerName,
            out IntPtr pCert);

        [DllImport(LIBNAME)]
        public static extern UInt32
            VMCAOpenEnumContextHA(
            IntPtr pServerContext,
            string argServerName, 
            Int32 dwStatus, 
            out IntPtr pContext);

        [DllImport(LIBNAME)]
        public static extern UInt32 
            VMCAGetNextCertificate(
            IntPtr pContext, 
            out IntPtr pCertificate,
            out Int32 currIndex, 
            out Int32 enumStatus);

        [DllImport(LIBNAME)]
        public static extern UInt32
            VMCACloseEnumContext(
            IntPtr pContext);

        [DllImport(LIBNAME)]
        public static extern UInt32
            VMCARevokeCertificate(
            string argServerName, 
            string cert);

        [DllImport(LIBNAME)]
        public static extern UInt32
            VMCACreatePrivateKey(
            string pszPassPhrase, 
            UInt32 uiKeyLength, 
            out IntPtr ppPrivateKey, 
            out IntPtr ppPublicKey);

        [DllImport(LIBNAME)]
        public static extern UInt32
        VMCACreateSelfSignedCertificate(
            VMCA_PKCS_10_REQ_DATA pCertRequest, 
            string pszPrivateKey, 
            string pszPassPhrase, 
            Int64 tmNotBefore,
            Int64 tmNotAfter,
            out IntPtr ppCertificate
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VMCACreateSigningRequest(
            VMCA_PKCS_10_REQ_DATA pCertRequest,
            string pPrivateKey,
            string pszPassPhrase,
            out IntPtr ppCSR
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VMCACreateSubjectDN(
            IntPtr pCertRequestData
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VMCAGetCertificateAsString(
            string pCertificate,
            out IntPtr ppszCertString
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
        VMCAGetServerVersionHA(
            IntPtr pServerContext,
            string pszServerName,
            out IntPtr ppszServerVersionString
        );

        [DllImport(LIBNAME)]
        public static extern UInt32
            VMCAValidateCACertificate(
            string pszCertificate
        );
        //Free
        [DllImport(LIBNAME)]
        public static extern UInt32
        VMCAAllocatePKCS10Data(
            out IntPtr ppCertRequestData
        );

        [DllImport(LIBNAME)]
        public static extern void
            VMCAFreeCertificate(
            IntPtr pString);

        [DllImport(LIBNAME)]
        public static extern void
            VMCAFreeCSR(
            IntPtr pString);

        [DllImport(LIBNAME)]
        public static extern void
            VMCAFreeKey(
            IntPtr pString);

        [DllImport(LIBNAME)]
        public static extern void
            VMCAFreePKCS10Data(
            IntPtr pString);

        [DllImport(LIBNAME)]
        public static extern UInt32
        VMCAGetErrorString(
            UInt32 dwErrorCode,
            out IntPtr errorMsg);

        [DllImport(LIBNAME)]
        public static extern UInt32
        VMCAGetSignedCertificateFromCSRHA(
            IntPtr pServerContext,
            string pszServerName,
            string pCertRequest,
            Int64 tmNotBefore,
            Int64 tmNotAfter,
            out IntPtr ppCertificate
        );


        [DllImport(LIBNAME)]
        public static extern UInt32
        VMCAGetSignedCertificateHA(
            IntPtr pServerContext,
            string pszServerName,
            VMCA_PKCS_10_REQ_DATA pCertRequest,
            string pszPrivateKey,
            string pszPassPhrase,
            Int64 tmNotBefore,
            Int64 tmNotAfter,
            out IntPtr ppCertificate
        );

        public static void VMCAFreeString(IntPtr pString)
        {
            VMCAFreeKey(pString);
        }

        public static void VMCAFreeVersion(IntPtr pString)
        {
            VMCAFreeKey(pString);
        }
    }
}

/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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



/*
 * Module Name: VMware Certificate Service
 *
 * Filename: vmcadb.h
 *
 * Abstract:
 *
 * VMware Certificate Service Database
 *
 */

#ifndef _VECSDB_H__
#define _VECSDB_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __VECS_DB_CONTEXT* PVECS_DB_CONTEXT;


typedef struct _VECS_DB_CERTIFICATE_ENTRY
{
    DWORD   dwID;
    DWORD   dwStoreID;
    PWSTR   pwszAlias;
    PWSTR   pwszSerial;
    PWSTR   pwszDate; //TODO: Change
    DWORD   dwCertSize;
    PBYTE   pCertBlob;
    PWSTR   pwszPassword;
    DWORD   dwKeySize;
    PBYTE   pPrivateKey;
    DWORD   dwStoreType; //TODO: Remove?
} VECS_DB_CERTIFICATE_ENTRY, *PVECS_DB_CERTIFICATE_ENTRY;

// this is deined in idl interface, make sure update both
#ifndef _VMCA_CERTIFICATE_CONTAINER_DEFINED_
typedef /* [public][public] */
enum __MIDL_vmca_0001
    {   CERTIFICATE_ACTIVE      = 0,
        CERTIFICATE_REVOKED     = 1,
        CERTIFIFCATE_EXPIRED    = 2,
        CERTIFICATE_ALL         = 4
    }   CERTIFICATE_STATUS;
#endif

DWORD
VecsDbInitialize(
    PCSTR pszDbPath
    );

VOID
VecsDbShutdown(
    VOID
    );

DWORD
VecsDbReset(
    VOID
    );

DWORD
VecsDbCleanupPermissions(
    VOID
    );

DWORD
VecsDbCreateCertStore(
    PCWSTR pszStoreName,
    PCWSTR pszPassword
    );

DWORD
VecsDbGetCertStore(
    PCWSTR pszStoreName,
    PCWSTR pszPassword,
    PDWORD pdwStoreId
    );

DWORD
VecsDbGetCertStoreName(
        DWORD   dwStoreId,
        PWSTR*  ppwszStoreName
        );

DWORD
VecsDbEnumCertStore(
    PWSTR **pszStoreNameArray,
    PDWORD pdwCount
    );

DWORD
VecsDbDeleteCertStore(
    PCWSTR pwszStoreName
    );

DWORD
VecsDbAddCert(
    DWORD dwStoreId,
    CERT_ENTRY_TYPE entryType,
    PWSTR pszAliasName,
    PWSTR pszCertificate,
    PWSTR pszPrivateKey,
    PWSTR pszPassword,
    BOOLEAN bAutoRefresh
    );

DWORD
VecsDbGetEntriesCount(
    DWORD dwStoreId,
    PDWORD pdwSize
    );

DWORD
VecsDbEnumInfoLevel1(
    DWORD dwStoreId,
    DWORD dwIndex,
    DWORD dwLimit,
    PVMAFD_CERT_ARRAY *ppCertContainer
    );

DWORD
VecsDbEnumInfoLevel2(
    DWORD dwStoreId,
    DWORD dwIndex,
    DWORD dwLimit,
    PVMAFD_CERT_ARRAY *ppCertContainer
    );

DWORD
VecsDbGetEntryByAliasInfoLevel1(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    PVMAFD_CERT_ARRAY *ppCertContainer
    );

DWORD
VecsDbGetEntryByAliasInfoLevel2(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    PVMAFD_CERT_ARRAY *ppCertContainer
    );

DWORD
VecsDbGetEntryTypeByAlias(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    CERT_ENTRY_TYPE *pEntryType
    );

DWORD
VecsDbGetEntryDateByAlias(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    PDWORD pdwDate
    );

DWORD
VecsDbGetCertificateByAlias(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    PWSTR *ppszCertificate
    );

DWORD
VecsDbGetPrivateKeyByAlias(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    PWSTR pszPassword,
    PWSTR *ppszPrivateKey
    );

DWORD
VecsDbDeleteCert(
    DWORD dwStoreId,
    PWSTR pszAliasName
    );

DWORD
VecsDbSetEntryDwordAttributeByAlias(
    DWORD dwStoreId,
    PWSTR pszAliasName,
    PSTR  pszAttributeName,
    DWORD dwValue
    );

DWORD
VecsDbAddCertificate(
    PVECS_DB_CONTEXT           pDbContext,
    PVECS_DB_CERTIFICATE_ENTRY pCertEntry
    );

DWORD
VecsDbGetTotalCertificateCount(
    PVECS_DB_CONTEXT pDbContext,
    PDWORD           pdwNumCerts,
    DWORD            dwFilter
    );

DWORD
VecsDbQueryAllCertificates(
    PVECS_DB_CONTEXT            pDbContext,
    PVECS_DB_CERTIFICATE_ENTRY* ppCertEntryArray,
    PDWORD                      pdwCount
    );

DWORD
VecsDbQueryCertificatesPaged(
    PVECS_DB_CONTEXT         pDbContext,
    DWORD                    dwStartIndex,
    DWORD                    dwNumPackages,
    CERTIFICATE_STORE_TYPE   dwStoreType,
    PVECS_DB_CERTIFICATE_ENTRY*  ppCertEntryArray,
    PDWORD                   pdwCount
    );


DWORD
VecsDbDeleteCertificate(
    PVECS_DB_CONTEXT pDbContext,
    PCWSTR           pwszSubjectDN
    );

DWORD
VecsDbRevokeCertificate(
    PVECS_DB_CONTEXT pDbContext,
    PCWSTR           pwszSerial
    );

VOID
VecsDbReleaseContext(
    PVECS_DB_CONTEXT pDbContext
    );

VOID
VecsDbFreeContext(
    PVECS_DB_CONTEXT pDbContext
    );

VOID
VecsDbFreeCertEntry(
    PVECS_DB_CERTIFICATE_ENTRY pCertEntry
    );

VOID
VecsDbFreeCertEntryArray(
    PVECS_DB_CERTIFICATE_ENTRY pCertEntryArray,
    DWORD                   dwCount
    );

VOID
VecsDbFreeCertEntryContents(
    PVECS_DB_CERTIFICATE_ENTRY pCertEntry
    );

DWORD
VecsDbVerifyCertificate(
    PVECS_DB_CONTEXT pDbContext,
    PCWSTR           pwszSerial,
    DWORD            *dwStatus
    );

DWORD
VecsDbQueryCertificateByAlias(
    PVECS_DB_CONTEXT            pDbContext,
    PVECS_DB_CERTIFICATE_ENTRY* ppCertEntryArray,
    PDWORD                      pdwCount,
    PCWSTR                      pwszAlias
);

DWORD
VecsDbSetDbVersion(
    DWORD dwVersion
    );

DWORD
VecsDbGetDbVersion(
    PDWORD pdwVersion
    );

#ifdef __cplusplus
}
#endif

#endif /* _VECSDB_H__ */



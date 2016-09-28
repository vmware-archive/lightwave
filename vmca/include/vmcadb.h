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

#ifndef __VMCADB_H__
#define __VMCADB_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _VMCA_DB_CONTEXT* PVMCA_DB_CONTEXT;

#define CRL_NUMBER_PROPERTY "crl_number"
#define CRL_NUMBER_PROPERTY_W {'c','r','l','_','n','u','m','b','e','r',0}


typedef struct _VMCA_DB_CERTIFICATE_ENTRY
{
    PWSTR   pwszCommonName;
    PWSTR   pwszAltNames;
    PWSTR   pwszOrgName;
    PWSTR   pwszOrgUnitName;
    PWSTR   pwszIssuerName;
    PWSTR   pwszCountryName;
    PWSTR   pwszSerial;
    PWSTR   pwszTimeValidFrom;
    PWSTR   pwszTimeValidTo;
    PBYTE   pCertBlob;
    DWORD   dwCertSize;
    DWORD   dwRevoked;
} VMCA_DB_CERTIFICATE_ENTRY, *PVMCA_DB_CERTIFICATE_ENTRY;

typedef struct _VMCA_DB_REVOKED
{
    PWSTR pwszSerial;
    DWORD dwRevokedDate;
    DWORD dwRevokedReason;
} VMCA_DB_REVOKED_CERTS, *PVMCA_DB_REVOKED_CERTS;

typedef enum
    {   VMCA_DB_CERTIFICATE_STATUS_ACTIVE      = 0,
        VMCA_DB_CERTIFICATE_STATUS_REVOKED     = 1,
        VMCA_DB_CERTIFICATE_STATUS_EXPIRED    = 2,
        VMCA_DB_CERTIFICATE_STATUS_ALL         = 4
    }   VMCA_DB_CERTIFICATE_STATUS;

DWORD
VmcaDbInitialize(
    PCSTR pszDbPath
    );

VOID
VmcaDbShutdown(
    VOID
    );

DWORD
VmcaDbReset(
    VOID
    );

DWORD
VmcaDbCreateContext(
    PVMCA_DB_CONTEXT* ppDbContext
    );

DWORD
VmcaDbCtxBeginTransaction(
    PVMCA_DB_CONTEXT	pDbContext
    );

DWORD
VmcaDbCtxCommitTransaction(
    PVMCA_DB_CONTEXT	pDbContext
    );

DWORD
VmcaDbCtxRollbackTransaction(
    PVMCA_DB_CONTEXT	pDbContext
    );

DWORD
VmcaDbAddCertificate(
    PVMCA_DB_CONTEXT           pDbContext,
    PVMCA_DB_CERTIFICATE_ENTRY pCertEntry
    );

DWORD
VmcaDbGetTotalCertificateCount(
    PVMCA_DB_CONTEXT pDbContext,
    PDWORD           pdwNumCerts,
    DWORD            dwFilter
    );

DWORD
VmcaDbQueryAllCertificates(
    PVMCA_DB_CONTEXT            pDbContext,
    PVMCA_DB_CERTIFICATE_ENTRY* ppCertEntryArray,
    PDWORD                      pdwCount
    );

DWORD
VmcaDbQueryCertificatesPaged(
    PVMCA_DB_CONTEXT         pDbContext,
    DWORD                    dwStartIndex,
    DWORD                    dwNumPackages,
    VMCA_DB_CERTIFICATE_STATUS       dwStatus,
    PVMCA_DB_CERTIFICATE_ENTRY*  ppPackageEntryArray,
    PDWORD                   pdwCount
    );

DWORD
VmcaDbDeleteCertificate(
    PVMCA_DB_CONTEXT pDbContext,
    PCWSTR           pwszSubjectDN
    );

DWORD
VmcaDbRevokeCertificate(
    PVMCA_DB_CONTEXT pDbContext,
    PCWSTR           pwszSerial
    );

DWORD
VmcaDbRevokeCert(
    PCWSTR           pwszSerial,
    PCWSTR           pwszIssuerName,
    DWORD            certRevokeReason
    );

DWORD
VmcaDbGetRevokedCerts(
    PVMCA_DB_REVOKED_CERTS* ppRevokedEntryArray,
    PDWORD pdwCertsReturned
    );

VOID
VmcaDbFreeRevokedCerts(
    PVMCA_DB_REVOKED_CERTS pRevokedEntryArray,
    DWORD dwCertCount
    );

DWORD
VmcaDbSetCurrentCRLNumber(
    DWORD dwCrlNumber
    );

DWORD
VmcaDbGetCurrentCRLNumber(
    PDWORD pdwCrlNumber
    );

VOID
VmcaDbReleaseContext(
    PVMCA_DB_CONTEXT pDbContext
    );

VOID
VmcaDbFreeContext(
    PVMCA_DB_CONTEXT pDbContext
    );

const char*
VmcaDbErrorCodeToName(
    int code
    );

VOID
VmcaDbFreeCertEntry(
    PVMCA_DB_CERTIFICATE_ENTRY pCertEntry
    );

VOID
VmcaDbFreeCertEntryArray(
    PVMCA_DB_CERTIFICATE_ENTRY pCertEntryArray,
    DWORD                   dwCount
    );

VOID
VmcaDbFreeCertEntryContents(
    PVMCA_DB_CERTIFICATE_ENTRY pCertEntry
    );

DWORD
VmcaDbVerifyCertificate(
    PVMCA_DB_CONTEXT pDbContext,
    PCWSTR           pwszSerial,
    DWORD            *dwStatus
    );

#ifdef __cplusplus
}
#endif

#endif /* __VMCADB_H__ */



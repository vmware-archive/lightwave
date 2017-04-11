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

#include "includes.h"
#include <vmcacommon.h>

#ifdef _WIN32
#pragma warning(disable : 4996 4267)
#define stat _stat
#include <direct.h> // for _mkdir
#endif

#include <vmca_error.h>
#include <vmcadb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vmca_h.h>

DWORD
VMCAFreeDBEntryFields(
    PVMCA_DB_CERTIFICATE_ENTRY pDBEntry
)
{
    DWORD dwError = 0;
    if(pDBEntry == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if( pDBEntry->pwszCommonName) {
        VMCAFreeMemory(pDBEntry->pwszCommonName);
    }

    if( pDBEntry->pwszAltNames) {
        VMCAFreeMemory(pDBEntry->pwszAltNames);
    }

    if( pDBEntry->pwszOrgName) {
        VMCAFreeMemory(pDBEntry->pwszOrgName);
    }

    if( pDBEntry->pwszOrgUnitName) {
        VMCAFreeMemory(pDBEntry->pwszOrgUnitName);
    }

    if( pDBEntry->pwszIssuerName) {
        VMCAFreeMemory(pDBEntry->pwszIssuerName);
    }

    if(pDBEntry->pwszCountryName) {
        VMCAFreeMemory(pDBEntry->pwszCountryName);
    }

    if( pDBEntry->pwszSerial) {
        VMCAFreeMemory(pDBEntry->pwszSerial);
    }


     if ( pDBEntry->pwszTimeValidFrom){
        VMCAFreeMemory(pDBEntry->pwszTimeValidFrom);
    }

    if ( pDBEntry->pwszTimeValidTo){
        VMCAFreeMemory(pDBEntry->pwszTimeValidTo);
    }

    if(pDBEntry->pCertBlob) {
        VMCAFreeMemory(pDBEntry->pCertBlob);
    }

    VMCAFreeMemory(pDBEntry);
    pDBEntry = NULL;
error:
    return dwError;
}

DWORD
VMCADecodeCert(
    PSTR pszCertificate,
    PVMCA_DB_CERTIFICATE_ENTRY* ppEntry
)
{
    DWORD dwError = 0;
    PVMCA_DB_CERTIFICATE_ENTRY pDBEntry = NULL;
    PSTR pszCertName = NULL;
    PSTR pszCertSerial = NULL;
    PSTR pszNotBefore = NULL;
    PSTR pszNotAfter = NULL;
    PSTR pszIssuerName = NULL;

    X509 *pCert = NULL;

    if (pszCertificate == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (ppEntry == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                        sizeof(VMCA_DB_CERTIFICATE_ENTRY),
                        (PVOID*) &pDBEntry);
    BAIL_ON_VMCA_ERROR(dwError);

    memset(pDBEntry,0, sizeof(VMCA_DB_CERTIFICATE_ENTRY));
    VMCAAllocateStringA(pszCertificate, (PSTR*)&pDBEntry->pCertBlob);
    //pDBEntry->pCertBlob = (PBYTE) pszCertificate;

    pDBEntry->dwCertSize = (DWORD)strlen(pszCertificate);

    dwError = VMCAPEMToX509(pszCertificate, &pCert);
    BAIL_ON_VMCA_ERROR(dwError);

    if (X509_cmp_current_time(X509_get_notAfter(pCert)) < 0)
    {
        pDBEntry->dwRevoked = VMCA_CERTIFICATE_EXPIRED;
    }

    dwError = VMCAGetCertificateName(pCert, &pszCertName);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringWFromA(pszCertName,
                            &pDBEntry->pwszCommonName);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetCertificateSerial(pCert, &pszCertSerial);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringWFromA(pszCertSerial,
                            &pDBEntry->pwszSerial);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetIssuerName(pCert, &pszIssuerName);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringWFromA(pszIssuerName,
                            &pDBEntry->pwszIssuerName);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetCertificateTime(pCert, &pszNotBefore, &pszNotAfter);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringWFromA(pszNotBefore,
                            &pDBEntry->pwszTimeValidFrom);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringWFromA(pszNotBefore,
                            &pDBEntry->pwszTimeValidTo);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppEntry = pDBEntry;

cleanup:
    if(pCert) {
        X509_free(pCert);
    }
 
    VMCA_SAFE_FREE_STRINGA(pszCertName);
    VMCA_SAFE_FREE_STRINGA(pszCertSerial);
    VMCA_SAFE_FREE_STRINGA(pszNotBefore);
    VMCA_SAFE_FREE_STRINGA(pszNotAfter);
    VMCA_SAFE_FREE_STRINGA(pszIssuerName);

    return dwError;

error :
    VMCAFreeDBEntryFields(pDBEntry);
    goto cleanup;

}

DWORD
VMCAGetServerVersion(
    PSTR* serverVersion
    )
{
    DWORD dwError = 0;

    dwError = VMCAAllocateStringA(VMCA_SERVER_VERSION_STRING, serverVersion);

    return dwError;
}

// db entry util 1
DWORD
VMCAClonePkgEntryContentsFromDbPkgEntry(
    PVMCA_DB_CERTIFICATE_ENTRY      pDbCertEntrySrc,
    PVMCA_CERTIFICATE_CONTAINER     pCertEntryDst
    )
{
    DWORD dwError =0;

    pCertEntryDst->dwCount = pDbCertEntrySrc->dwCertSize;
    dwError = VMCAAllocateStringA (
                            (RP_PSTR)pDbCertEntrySrc->pCertBlob,
                            (RP_PSTR*)&pCertEntryDst->pCert);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

// db entry util
unsigned int
VMCACloneCertContainerFromDbCertEntryArray(
    PVMCA_DB_CERTIFICATE_ENTRY  pDbCertEntryArray,
    DWORD                       dwDbCertEntryNums,
    VMCA_CERTIFICATE_ARRAY**    ppCertArray
    )
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE_ARRAY pCertArray = NULL;

    dwError = VMCAAllocateMemory(
                    sizeof(VMCA_CERTIFICATE_ARRAY),
                    (PVOID*)&pCertArray);
    BAIL_ON_VMCA_ERROR(dwError);
    // get array size
    pCertArray->dwCount = dwDbCertEntryNums;

    if (pCertArray->dwCount > 0)
    {
        DWORD iEntry = 0;
        dwError = VMCAAllocateMemory(
                    pCertArray->dwCount * sizeof(pCertArray->certificates[0]),
                    (PVOID*)&pCertArray->certificates);
        BAIL_ON_VMCA_ERROR(dwError);
        for (; iEntry < pCertArray->dwCount; iEntry++)
        {
            PVMCA_DB_CERTIFICATE_ENTRY pDbCertEntrySrc
                    = &pDbCertEntryArray[iEntry];
            PVMCA_CERTIFICATE_CONTAINER pCertContainerDst
                    = &pCertArray->certificates[iEntry];

            // copy pCert string to container
            dwError = VMCAClonePkgEntryContentsFromDbPkgEntry(
                      pDbCertEntrySrc,
                      pCertContainerDst);

            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

    *ppCertArray = pCertArray;

cleanup:

    return dwError;

error:

    *ppCertArray = NULL;

    if (pCertArray)
    {
        VMCAFreeCertificateArray(pCertArray);
    }

    goto cleanup;
}


DWORD
VMCADbEnumCerts(
    PVMCA_DB_CONTEXT         pDbContext,
    DWORD                    dwStartIndex,
    DWORD                    dwNumCerts,
    CERTIFICATE_STATUS       dwStatus,
    PVMCA_DB_CERTIFICATE_ENTRY*  ppCertEntryArray,
    PDWORD                   pdwCount
    )
{
    DWORD dwError = 0;
    PVMCA_DB_CERTIFICATE_ENTRY pCertEntryArray = NULL;
    DWORD dwCount = 0;

    if (dwNumCerts > 0)
    {
        // TODO: add status filter
        dwError = VmcaDbQueryCertificatesPaged(
                        pDbContext,
                        dwStartIndex,
                        dwNumCerts,
                        VMCAMapToDBStatus(dwStatus),
                        &pCertEntryArray,
                        &dwCount);
    }
    else
    {
        dwError = VmcaDbQueryAllCertificates(
                        pDbContext,
                        &pCertEntryArray,
                        &dwCount);
    }
    BAIL_ON_VMCA_ERROR(dwError);

    *ppCertEntryArray = pCertEntryArray;
    *pdwCount = dwCount;

cleanup:

    return dwError;

error:

    *ppCertEntryArray = NULL;
    *pdwCount = 0;

    if (pCertEntryArray)
    {
        VmcaDbFreeCertEntryArray(pCertEntryArray, dwCount);
    }

    goto cleanup;
}

unsigned int
VMCAEnumCertificates(
    unsigned int dwStartIndex,
    unsigned int dwNumCertificates,
    CERTIFICATE_STATUS dwStatus,
    VMCA_CERTIFICATE_ARRAY** ppCertArray
)
{
    DWORD dwError = 0;
    PVMCA_DB_CONTEXT pDbContext = NULL;
    PVMCA_CERTIFICATE_ARRAY pCertArray = NULL;
    PVMCA_DB_CERTIFICATE_ENTRY  pDbCertEntryArray = NULL;
    DWORD dwDbCertEntryNums = 0;
    BOOLEAN bLocked = FALSE;

    VMCA_LOCK_MUTEX_SHARED(&gVMCAServerGlobals.svcMutex, bLocked);

    dwError = VMCASrvValidateCA();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbCreateContext(&pDbContext);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCADbEnumCerts(
                        pDbContext,
                        dwStartIndex,
                        dwNumCertificates,
                        dwStatus,
                        &pDbCertEntryArray,
                        &dwDbCertEntryNums);
    BAIL_ON_VMCA_ERROR(dwError);

    // convert db struct into rpc struct ...
    dwError = VMCACloneCertContainerFromDbCertEntryArray(
                        pDbCertEntryArray,
                        dwDbCertEntryNums,
                        &pCertArray);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppCertArray = pCertArray;

cleanup:

    VMCA_LOCK_MUTEX_UNLOCK(&gVMCAServerGlobals.svcMutex, bLocked);

    if (pDbContext)
    {
        VmcaDbReleaseContext(pDbContext);
    }

    if (pDbCertEntryArray)
    {
        VmcaDbFreeCertEntryArray(pDbCertEntryArray, dwDbCertEntryNums);
    }

    // if 0 certificate returned, then we need to set memory to NULL to make sure no rpc free being called on grabage pointer.
    if (pCertArray){
        if (pCertArray->dwCount == 0){
            pCertArray->certificates = NULL;
            VMCARpcFreeCertificateArray(pCertArray);
            pCertArray = NULL;
        }
    }


    return dwError;

error:

    *ppCertArray = NULL;

    if (pCertArray)
    {
        VMCARpcFreeCertificateArray(pCertArray);
    }

    goto cleanup;
}

unsigned int
VMCAAddRootCertificate(
    unsigned char *pszRootCertificate,
    PWSTR pszPassPhrase,
    unsigned char *pszPrivateKey,
    unsigned int dwOverWrite)
{

    DWORD dwError = 0;
    BOOL bFileExists = FALSE;
    BOOL bOverWrite = FALSE;
    BOOLEAN bLocked = FALSE;

    PSTR pszRootCertFile = NULL;
    PSTR pszPrivateKeyFile = NULL;
    PSTR pszPasswordFile = NULL;
    PSTR pszDataDirectory = NULL;
#ifndef _WIN32
    struct stat buf = { 0 };
#else
    struct _stat buf = { 0 };
#endif

    bOverWrite = (dwOverWrite == 1);
    //
    // Grab exclusive lock since we are writing the Root Cert,
    // and all operations must serialize for this op.
    //

    VMCA_LOCK_MUTEX_EXCLUSIVE(&gVMCAServerGlobals.svcMutex, bLocked);

    dwError = VMCAValidateCACertificatePrivate((LPSTR) pszRootCertificate,NULL, (LPSTR)pszPrivateKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetRootCertificateFilePath(&pszRootCertFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetPrivateKeyPath(&pszPrivateKeyFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetPrivateKeyPasswordPath(&pszPasswordFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetDataDirectory(&pszDataDirectory);
    BAIL_ON_VMCA_ERROR(dwError);

    bFileExists = (stat(pszRootCertFile,&buf) == ERROR_SUCCESS);

    if (!bOverWrite && bFileExists)
    {
        dwError = VMCA_ROOT_CA_ALREADY_EXISTS;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    if (bOverWrite && bFileExists)
    {
        dwError = VMCABackupRootCAFiles(pszRootCertFile, pszPrivateKeyFile, pszPasswordFile);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!bFileExists)
    {
       dwError = VMCACreateDirectory(pszDataDirectory, TRUE);
       BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError =  VMCAWriteCertificateChainToFile(pszRootCertFile,
                                              (PVMCA_CERTIFICATE) pszRootCertificate);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAWritePrivateKeyToFile( pszPrivateKeyFile,
                                         (LPSTR) pszPrivateKey,
                                         pszPasswordFile,
                                         pszPassPhrase);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASetKeyPerm(pszPrivateKeyFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASrvInitCA();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASrvNotifyDirSync();
    BAIL_ON_VMCA_ERROR(dwError);

#if 0
#ifdef DEBUG
    PrintCurrentState();
#endif
#endif

error :

    VMCA_LOCK_MUTEX_UNLOCK(&gVMCAServerGlobals.svcMutex, bLocked);

    if ( pszRootCertFile != NULL) {
        VMCAFreeStringA(pszRootCertFile);
    }

    if( pszPrivateKeyFile != NULL) {
        VMCAFreeStringA(pszPrivateKeyFile);
    }

    if( pszPasswordFile != NULL ) {
        VMCAFreeStringA(pszPasswordFile);
    }

    if (pszDataDirectory != NULL) {
        VMCAFreeStringA(pszDataDirectory);
    }
    return dwError;
}


unsigned int
VMCAGetSignedCertificate(
    unsigned char *pszPEMEncodedCSRRequest,
    unsigned int  dwtmNotBefore,
    unsigned int  dwtmNotAfter,
    PVMCA_CERTIFICATE_CONTAINER *ppCertContainer
)
{
    DWORD dwError = 0;
    time_t now = dwtmNotBefore;
    time_t expire = dwtmNotAfter;
    PSTR pCert = NULL;
    PVMCA_DB_CERTIFICATE_ENTRY pEntry = NULL;
    PVMCA_X509_CA pCA = NULL;
    PVMCA_CERTIFICATE_CONTAINER pCertContainer = NULL;
    BOOLEAN bLocked = FALSE;

    VMCA_LOCK_MUTEX_EXCLUSIVE(&gVMCAServerGlobals.svcMutex, bLocked);

    dwError = VMCASrvValidateCA();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASrvGetCA(&pCA);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASignedRequestPrivate(
                  pCA,
                  (char *)pszPEMEncodedCSRRequest,
                  (char**)&pCert,
                  now,
                  expire);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateCertificateContainer(pCert, &pCertContainer);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCADecodeCert(pCert, &pEntry);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAInsertCertificate(pEntry);
    BAIL_ON_VMCA_ERROR(dwError);

    strcpy((char*)pCertContainer->pCert, pCert);

    *ppCertContainer = pCertContainer;

cleanup :

    VMCA_LOCK_MUTEX_UNLOCK(&gVMCAServerGlobals.svcMutex, bLocked);

    VMCAFreeDBEntryFields(pEntry);
    VMCA_SAFE_FREE_STRINGA(pCert);
    if (pCA)
    {
        VMCAReleaseCA(pCA);
    }
    return dwError;

error:

    if (ppCertContainer)
    {
        *ppCertContainer = NULL;
    }

    VMCAFreeCertificateContainer(pCertContainer);

    goto cleanup;
}


DWORD
VMCAInsertCertificate(PVMCA_DB_CERTIFICATE_ENTRY pEntry)
{
    DWORD dwError = 0;
    PVMCA_DB_CONTEXT pContext = NULL;

    dwError = VmcaDbCreateContext(&pContext);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbAddCertificate(pContext,pEntry);
    BAIL_ON_VMCA_ERROR(dwError);

error :
    if (pContext != NULL) {
        VmcaDbReleaseContext(pContext);
    pContext = NULL;
    }

    return dwError;
}

VOID
VMCAFreeDBEntry(PVMCA_DB_CERTIFICATE_ENTRY pEntry)
{
    if ( pEntry == NULL ) {
        return ;
    }

    if (pEntry->pwszCommonName != NULL ) {
        VMCAFreeStringW((RP_PWSTR)pEntry->pwszCommonName);
    }

    if (pEntry->pwszSerial != NULL ) {
        VMCAFreeStringW((RP_PWSTR)pEntry->pwszSerial);
    }

    if (pEntry->pwszTimeValidFrom != NULL) {
        VMCAFreeStringW((RP_PWSTR)pEntry->pwszTimeValidFrom);
    }

    if (pEntry->pwszTimeValidTo != NULL) {
        VMCAFreeStringW((RP_PWSTR)pEntry->pwszTimeValidTo);
    }

    // VMCAFreeStringW(pEntry->pwszCommonName);
    // VMCAFreeStringW(pEntry->pwszAltNames);
    // VMCAFreeStringW(pEntry->pwszOrgName);
    // VMCAFreeStringW(pEntry->pwszOrgUnitName);
    // VMCAFreeStringW(pEntry->pwszCountryName);
    // VMCAFreeStringW(pEntry->pwszIssuerName);
    // VMCAFreeStringW(pEntry->pwszSerial);
}


unsigned int
VMCAGetRootCACertificate(
    unsigned int *dwCertLength,
    PVMCA_CERTIFICATE *ppCertificate
    )
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pTempCertificate = NULL;
    PSTR pszRootCertFile = NULL;
    BOOLEAN bLocked = FALSE;

    VMCA_LOCK_MUTEX_SHARED(&gVMCAServerGlobals.svcMutex, bLocked);

    dwError = VMCASrvValidateCA();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetRootCertificateFilePath(&pszRootCertFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAReadCertificateChainFromFile(
                                pszRootCertFile,
                                &pTempCertificate);
    BAIL_ON_VMCA_ERROR(dwError);



    *ppCertificate = pTempCertificate;

cleanup:
    VMCA_LOCK_MUTEX_UNLOCK(&gVMCAServerGlobals.svcMutex, bLocked);

    VMCA_SAFE_FREE_STRINGA(pszRootCertFile);
    return dwError;

error :

    VMCA_SAFE_FREE_MEMORY(pTempCertificate);

    goto cleanup;
}

VOID
VMCARpcFreeCertificateContainer(PVMCA_CERTIFICATE_CONTAINER pCertContainer)
{
 if (pCertContainer != NULL) {
    VMCARpcFreeMemory(pCertContainer->pCert);
    VMCARpcFreeMemory(pCertContainer);
    pCertContainer = NULL;
 }
}

VOID
VMCAFreeCertificateContainer(PVMCA_CERTIFICATE_CONTAINER pCertContainer)
{
 if (pCertContainer != NULL) {
    VMCA_SAFE_FREE_MEMORY(pCertContainer->pCert);
    VMCA_SAFE_FREE_MEMORY(pCertContainer);
    pCertContainer = NULL;
 }
}


DWORD
VMCARpcAllocateCertificateContainer(PSTR pCert, PVMCA_CERTIFICATE_CONTAINER *ppCertContainer)
{
    DWORD dwError = 0;
    dwError = VMCARpcAllocateMemory(sizeof(VMCA_CERTIFICATE_CONTAINER) ,(PVOID*)ppCertContainer);
    BAIL_ON_VMCA_ERROR(dwError);
    memset(*ppCertContainer, 0 , sizeof(VMCA_CERTIFICATE_CONTAINER));

    (*ppCertContainer)->dwCount = (unsigned long)strlen(pCert);
    dwError = VMCARpcAllocateMemory(strlen(pCert)+1, (PVOID*) &((*ppCertContainer)->pCert));
    BAIL_ON_VMCA_ERROR(dwError);
    memset((*ppCertContainer)->pCert, 0 ,strlen(pCert)+1);
error :
    if (dwError != 0) {
        VMCARpcFreeCertificateContainer(*ppCertContainer);
    }
    return dwError;
}

DWORD
VMCAAllocateCertificateContainer(PSTR pCert, PVMCA_CERTIFICATE_CONTAINER *ppCertContainer)
{
    DWORD dwError = 0;

    dwError = VMCAAllocateMemory(sizeof(VMCA_CERTIFICATE_CONTAINER) ,(PVOID*)ppCertContainer);
    BAIL_ON_VMCA_ERROR(dwError);
    memset(*ppCertContainer, 0 , sizeof(VMCA_CERTIFICATE_CONTAINER));

    (*ppCertContainer)->dwCount = (unsigned long)strlen(pCert);
    dwError = VMCAAllocateMemory(strlen(pCert)+1, (PVOID*) &((*ppCertContainer)->pCert));
    BAIL_ON_VMCA_ERROR(dwError);
    memset((*ppCertContainer)->pCert, 0 ,strlen(pCert)+1);
error :
    if (dwError != 0) {
        VMCAFreeCertificateContainer(*ppCertContainer);
    }
    return dwError;
}

DWORD
VMCARevokeCertificate(
    unsigned char *pszCertificate)
{
    DWORD dwError = 0;
    PVMCA_DB_CERTIFICATE_ENTRY pEntry = NULL;
    PVMCA_DB_CONTEXT pContext = NULL;
    BOOLEAN bLocked = FALSE;

    VMCA_LOCK_MUTEX_EXCLUSIVE(&gVMCAServerGlobals.svcMutex, bLocked);

    dwError = VMCASrvValidateCA();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbCreateContext(&pContext);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCADecodeCert((LPSTR)pszCertificate, &pEntry);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbRevokeCertificate(pContext, pEntry->pwszSerial);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAUpdateCRL((LPSTR)pszCertificate, 0); //Reason Unknown
    BAIL_ON_VMCA_ERROR(dwError);

error:

    VMCA_LOCK_MUTEX_UNLOCK(&gVMCAServerGlobals.svcMutex, bLocked);

    if (pContext != NULL) {
        VmcaDbReleaseContext(pContext);
        pContext = NULL;
    }

    if(pEntry != NULL) {
        VMCAFreeDBEntryFields(pEntry);
    }
    pEntry = NULL;
    return dwError;
}

DWORD
VMCAVerifyCertificate(
    unsigned char *pszPEMEncodedCertificate,
    unsigned int *dwStatus
    )
{
    DWORD dwError = 0;
    PVMCA_DB_CERTIFICATE_ENTRY pEntry = NULL;
    PVMCA_DB_CONTEXT pContext = NULL;
    BOOLEAN bLocked = FALSE;

    *dwStatus = VMCA_CERTIFICATE_ALL;

    VMCA_LOCK_MUTEX_SHARED(&gVMCAServerGlobals.svcMutex, bLocked);

    dwError = VMCASrvValidateCA();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCADecodeCert((LPSTR)pszPEMEncodedCertificate, &pEntry);
    BAIL_ON_VMCA_ERROR(dwError);

    if (pEntry->dwRevoked == VMCA_CERTIFICATE_EXPIRED)
    {
        *dwStatus = VMCA_CERTIFICATE_EXPIRED;
        goto error;
    }

    dwError = VmcaDbCreateContext(&pContext);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError =  VmcaDbVerifyCertificate(pContext, pEntry->pwszSerial, (DWORD*)dwStatus);
    BAIL_ON_VMCA_ERROR(dwError);

error :

    VMCA_LOCK_MUTEX_UNLOCK(&gVMCAServerGlobals.svcMutex, bLocked);

    if (pContext != NULL)
    {
        VmcaDbReleaseContext(pContext);
        pContext = NULL;
    }
    if(pEntry != NULL)
    {
        VMCAFreeDBEntryFields(pEntry);
        pEntry = NULL;
    }
    return dwError;
}



DWORD
VMCARpcAllocateString(
    PSTR  pszSrc,
    PSTR* ppszDst
    )
{
    DWORD dwError = 0;
    size_t len = 0;
    PSTR pszDst = NULL;

    if (!pszSrc || !ppszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    len = strlen(pszSrc);
    // + 1 for \'0'

    dwError = VMCARpcAllocateMemory(
                    sizeof(RP_STR) * (len + 1),
                    (PVOID*)&pszDst);
    BAIL_ON_VMCA_ERROR(dwError);
    memcpy(pszDst, pszSrc, len);
    *ppszDst = pszDst;

cleanup:

    return dwError;

error:

    if (ppszDst)
    {
        *ppszDst = NULL;
    }

    if (pszDst)
    {
        VMCARpcFreeMemory(pszDst);
    }

    goto cleanup;
}

VOID
VMCAFreeCertificateArray(PVMCA_CERTIFICATE_ARRAY pCertArray)
{
    if (pCertArray == NULL)
    {
        goto cleanup;
    }

    if (pCertArray->dwCount > 0) {
        unsigned int iEntry  = 0;
        // free string in each container
        for (; iEntry < pCertArray->dwCount; iEntry++)
        {
            VMCAFreeMemory( pCertArray->certificates[iEntry].pCert);
        }
        VMCAFreeMemory(pCertArray->certificates);
        VMCAFreeMemory(pCertArray);
    }
cleanup:
    return;
}

VOID
VMCARpcFreeCertificateArray(PVMCA_CERTIFICATE_ARRAY pCertArray)
{
    if (pCertArray == NULL)
    {
        goto cleanup;
    }

    if (pCertArray->dwCount > 0) {
        unsigned int iEntry  = 0;
        // free string in each container
        for (; iEntry < pCertArray->dwCount; iEntry++) {
            VMCARpcFreeMemory( pCertArray->certificates[iEntry].pCert);
        }
        VMCARpcFreeMemory(pCertArray->certificates);
        VMCARpcFreeMemory(pCertArray);
    }
cleanup:
    return;
}



DWORD
VMCACreateDirectory(PSTR pszDirectoryName, BOOL bRestrictAccess)
{
    DWORD dwError = 0;
    PSTR pszDir = NULL;
#ifdef _WIN32
    PSTR pszTemp = NULL;
    int err = 0;
#endif

    if (!pszDirectoryName || !*pszDirectoryName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

#ifndef _WIN32
     mkdir(pszDirectoryName, 700);
     goto error;
#else
    dwError = VMCAAllocateStringA(pszDirectoryName, &pszDir);
    BAIL_ON_VMCA_ERROR(dwError);

    pszTemp = pszDir;
    if (strlen(pszDir) > 3 && pszDir[1] == ':' && pszDir[2] == '\\')
    {
        pszTemp += 3;
    }
    while ( (pszTemp = strchr(pszTemp, '\\')) )
    {
        *pszTemp = '\0';
        err = _mkdir(pszDir);
        if(err == -1 && errno != EEXIST)
        {
            dwError = ERROR_PATH_NOT_FOUND;
            BAIL_ON_VMCA_ERROR(dwError);
        }
        *pszTemp++ = '/';
    }

    if (bRestrictAccess)
    {
        dwError = VMCARestrictDirectoryAccess(pszDir);
        BAIL_ON_VMCA_ERROR(dwError);
    }
#endif

error:

    VMCA_SAFE_FREE_STRINGA(pszDir);

    return dwError;
}

DWORD
VMCACreateDataDirectory(VOID)
{
    DWORD dwError = 0;
    PSTR pszDir = NULL;
    PSTR pszTmpDir = NULL;
    struct stat buf = {0};
    size_t iDirLen = 0;

    dwError = VMCAGetDataDirectory(&pszDir);
    BAIL_ON_VMCA_ERROR(dwError);

    if (stat(pszDir, &buf) != 0)
    {
       iDirLen = strlen(pszDir);
       if (pszDir[iDirLen-1] != VMCA_PATH_SEPARATOR_CHAR)
       {
           dwError = VMCAAllocateMemory(iDirLen+2, (PVOID *)&pszTmpDir);
           BAIL_ON_VMCA_ERROR(dwError);

           strcpy(pszTmpDir, pszDir);
           pszTmpDir[iDirLen] = VMCA_PATH_SEPARATOR_CHAR;
           pszTmpDir[iDirLen+1] = '\0';
           VMCA_SAFE_FREE_STRINGA(pszDir);
           pszDir = pszTmpDir;
       }
       dwError = VMCACreateDirectory(pszDir, TRUE);
       BAIL_ON_VMCA_ERROR(dwError);
    }
#ifdef _WIN32
    else
    {
        dwError = VMCARestrictDirectoryAccess(pszDir);
        BAIL_ON_VMCA_ERROR(dwError);
    }
#endif

error:

    VMCA_SAFE_FREE_STRINGA(pszDir);

    return dwError;
}

DWORD
VMCASetKeyPerm(PSTR pszPrivateKeyFileName)
{
    DWORD dwError = 0;
#ifndef _WIN32
    dwError = chmod(pszPrivateKeyFileName, S_IRUSR | S_IWUSR);
#endif
    return dwError;

}


DWORD
VMCAGetCertificateCount(
    unsigned int dwStatus,
    unsigned int *dwNumCertificates
)
{
    DWORD dwError = 0;
    DWORD dwCount = 0;
    PVMCA_DB_CONTEXT pContext = NULL;
    BOOLEAN bLocked = FALSE;

//
// TODO : Add Parameter Validation Code.
//
    VMCA_LOCK_MUTEX_SHARED(&gVMCAServerGlobals.svcMutex, bLocked);

    // Don't do this, even if we have no Root Cert,
    // The client program should be able to read
    // the number of certificates in the Data base.
    // In most common Scenerios, this will be 0.
    //
    // dwError = VMCASrvValidateCA();
    // BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbCreateContext(&pContext);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError =  VmcaDbGetTotalCertificateCount(pContext, &dwCount, dwStatus);
    BAIL_ON_VMCA_ERROR(dwError);
    *dwNumCertificates = dwCount;

error:

    VMCA_LOCK_MUTEX_UNLOCK(&gVMCAServerGlobals.svcMutex, bLocked);

    if (pContext != NULL) {
        VmcaDbReleaseContext(pContext);
    pContext = NULL;
    }

    return dwError;

}

DWORD
VMCAUpdateCRL(
    PSTR pszCertificate,
    UINT32 uReason
)
{

    DWORD dwError  =  0;
    PSTR pszCRLFile = NULL;
    time_t tmLastUpdate = 0;
    time_t tmNextUpdate = 0;
    X509_CRL *pCrl = NULL;
    X509 *pCert = NULL;
    DWORD dwCrlNum = 0;
    PVMCA_X509_CA pCA = NULL;

    dwError = VMCASrvGetCA(&pCA);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetCRLNamePath(&pszCRLFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAOpenCRLPrivate(pCA, pszCRLFile, &pCrl);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAPEMToX509(pszCertificate, &pCert);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAddCertToCRL(pCrl, pCert, 1);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASortCRL(pCrl);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetNextCrlNumber(pCrl, &dwCrlNum);
    BAIL_ON_VMCA_ERROR(dwError);

    tmLastUpdate = time(NULL);
    dwError = VMCAGetNextUpdateTime(pCrl, &tmNextUpdate);

    if (dwError == VMCA_CRL_NULL_TIME ) {
        tmNextUpdate = tmLastUpdate +
        ( VMCAGetDefaultValidityPeriod() * 24 * 60 * 60);
        dwError = 0;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAUpdateTimeStamps (pCrl, tmLastUpdate,
                                    tmNextUpdate, dwCrlNum);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACrlSign(pCrl, pCA);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACRLToFile(pCrl, pszCRLFile);
    BAIL_ON_VMCA_ERROR(dwError);

error:

    if (pCert != NULL)
    {
        X509_free(pCert);
    }
    VMCA_SAFE_FREE_STRINGA(pszCRLFile);
    if(pCrl != NULL)
    {
       VMCACrlFree(pCrl);
    }
    if (pCA)
    {
       VMCAReleaseCA(pCA);
    }
   return dwError;
}


DWORD
VMCAReadDataFromFile(
    PSTR pszFileName,
    DWORD dwOffset,
    DWORD dwSize,
    PSTR data,
    PDWORD dwBytesRead)
{
    DWORD dwError = 0;
    FILE *pFile = NULL;
    int bytes_read = 0;

    dwError = VMCAOpenFilePath(pszFileName, "r", &pFile);
    BAIL_ON_VMCA_ERROR(dwError);

    if ( pFile == NULL) {
        dwError = VMCA_FILE_IO_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = fseek(pFile, dwOffset, SEEK_SET);
    BAIL_ON_VMCA_ERROR(dwError);

    bytes_read = fread(data, 1, dwSize, pFile);
    if ( !feof(pFile) && ( bytes_read != dwSize)){
        dwError = VMCA_FILE_IO_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    *dwBytesRead = bytes_read;
error :
    if(pFile != NULL){
        fclose(pFile);
    }
    return dwError;
}

unsigned int
VMCAGetCRL(
    unsigned int dwFileOffset,
    unsigned int dwSize,
    VMCA_FILE_BUFFER **ppCRLData
    )
{
    DWORD dwError = 0;
    PVMCA_FILE_BUFFER pCRLData = NULL;
    DWORD dwCount = 0;
    PSTR pszCRLFile = NULL;
    unsigned char buff[FILE_CHUNK] = { 0 };
    BOOLEAN bLocked = FALSE;

    VMCA_LOCK_MUTEX_SHARED(&gVMCAServerGlobals.svcMutex, bLocked);

    dwError = VMCASrvValidateCA();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetCRLNamePath(&pszCRLFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateMemory(
            sizeof(VMCA_FILE_BUFFER),
            (PVOID*) &pCRLData
            );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAReadDataFromFile(
        pszCRLFile,
        dwFileOffset,
        dwSize,
        (char*)&buff,
        &dwCount);
    BAIL_ON_VMCA_ERROR(dwError);
    if (dwCount > 0)
    {
        dwError = VMCAAllocateMemory(
                dwCount * sizeof(unsigned char) + 1,
                (PVOID*) &(pCRLData->buffer)
                );
        BAIL_ON_VMCA_ERROR(dwError);
        memcpy((PVOID) pCRLData->buffer, buff,(size_t) dwCount);
    }

    pCRLData->dwCount = dwCount;
    *ppCRLData = pCRLData;

cleanup :

    VMCA_LOCK_MUTEX_UNLOCK(&gVMCAServerGlobals.svcMutex, bLocked);

    VMCA_SAFE_FREE_STRINGA(pszCRLFile);
    return dwError;

error:
    if(pCRLData != NULL)
    {
        if(pCRLData->buffer != NULL)
        {
            VMCAFreeMemory((PVOID) pCRLData->buffer);
        }
        VMCAFreeMemory((PVOID)pCRLData);
    }
    if(ppCRLData)
    {
        *ppCRLData = NULL;
    }
    goto cleanup;
}


unsigned int
VMCAGenerateCRL ()
{

    DWORD dwCertCount = 0;
    DWORD dwError = 0;
    DWORD dwCount = 0;
    PVMCA_DB_CONTEXT pDbContext = NULL;
    PVMCA_DB_CERTIFICATE_ENTRY pCertEntryArray = NULL;
    PSTR pszCrlTempNamePath = NULL;
    time_t tmNextUpdate = 0;
    time_t tmLastUpdate = 0;
    X509_CRL *pCrl = NULL;
    X509 *pCert = NULL;
    DWORD dwCrlNum = 0;
    PVMCA_X509_CA pCA = NULL;
    BOOLEAN bLocked = FALSE;
    DWORD x = 0;

    VMCA_LOCK_MUTEX_SHARED(&gVMCAServerGlobals.svcMutex, bLocked);

    dwError = VMCASrvValidateCA();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASrvGetCA(&pCA);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbCreateContext(&pDbContext);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError =  VmcaDbGetTotalCertificateCount(
                        pDbContext,
                        &dwCertCount,
                        VMCA_CERTIFICATE_REVOKED);
    BAIL_ON_VMCA_ERROR(dwError);

    if (dwCertCount)
    {
            dwError = VmcaDbQueryCertificatesPaged(
                        pDbContext,
                        0,
                        dwCertCount,
                        VMCA_DB_CERTIFICATE_STATUS_REVOKED,
                        &pCertEntryArray,
                        &dwCount);
            BAIL_ON_VMCA_ERROR(dwError);
    }

     // Database operations are done
    VMCA_LOCK_MUTEX_UNLOCK(&gVMCAServerGlobals.svcMutex, bLocked);

    dwError = VMCAGetTempCRLNamePath(&pszCrlTempNamePath);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAOpenCRLPrivate(pCA, pszCrlTempNamePath, &pCrl);
    BAIL_ON_VMCA_ERROR(dwError);

    for (x = 0; x < dwCount; x++) {
        dwError = VMCAPEMToX509((PSTR)pCertEntryArray[x].pCertBlob,
            &pCert);
        BAIL_ON_VMCA_ERROR(dwError);

        dwError = VMCAAddCertToCRL(pCrl, pCert , 1);
        BAIL_ON_VMCA_ERROR(dwError);

        X509_free(pCert);
        pCert = NULL;
    }

    dwError = VMCAGetNextCrlNumber(pCrl, &dwCrlNum);
    BAIL_ON_VMCA_ERROR(dwError);

    tmLastUpdate = time(NULL);
    dwError = VMCAGetNextUpdateTime(pCrl, &tmNextUpdate);
    if (dwError == VMCA_CRL_NULL_TIME ) {
        tmNextUpdate = tmLastUpdate +
        ( VMCAGetDefaultValidityPeriod() * 24 * 60 * 60);
        dwError = 0;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAUpdateTimeStamps (pCrl, tmLastUpdate,
                                tmNextUpdate, dwCrlNum);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCASortCRL(pCrl);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACrlSign(pCrl, pCA);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACRLToFile(pCrl, pszCrlTempNamePath);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACopyTempCRLtoCRL();
    BAIL_ON_VMCA_ERROR(dwError);

error :

    VMCA_LOCK_MUTEX_UNLOCK(&gVMCAServerGlobals.svcMutex, bLocked);

    if (pCrl)
    {
        VMCACrlFree(pCrl);
    }
    if(pCert != NULL)
    {
        X509_free(pCert);
    }
    VMCA_SAFE_FREE_STRINGA(pszCrlTempNamePath);
    if (pCertEntryArray)
    {
        VmcaDbFreeCertEntryArray(pCertEntryArray, dwCount);
    }
    if (pDbContext)
    {
        VmcaDbReleaseContext(pDbContext);
    }
    if (pCA)
    {
        VMCAReleaseCA(pCA);
    }
    return dwError;
}

DWORD
VMCACopyTempCRLtoCRL()
{
    DWORD dwError = 0;
    PSTR pszTmpFile = NULL;
    PSTR pszCRLFile = NULL;
    BOOLEAN bLocked = FALSE;
    FILE* pIn = NULL;
    FILE* pOut = NULL;

    dwError = VMCAGetCRLNamePath(&pszCRLFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetTempCRLNamePath(&pszTmpFile);
    BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOCK_MUTEX_EXCLUSIVE(&gVMCAServerGlobals.svcMutex, bLocked);

    if ((pIn = fopen(pszTmpFile, "rb")) == NULL)
    {
#ifndef _WIN32
                dwError = LwErrnoToWin32Error(errno);
#else
                dwError = GetLastError();
#endif

        VMCA_LOG_ERROR("Failed to open file [%s] for reading", pszTmpFile);

        BAIL_ON_VMCA_ERROR(dwError);
    }
    if ((pOut = fopen(pszCRLFile, "wb")) == NULL)
    {
#ifndef _WIN32
                dwError = LwErrnoToWin32Error(errno);
#else
                dwError = GetLastError();
#endif

        VMCA_LOG_ERROR("Failed to open file [%s] for writing", pszCRLFile);

        BAIL_ON_VMCA_ERROR(dwError);
    }

    while (!feof(pIn))
    {
        CHAR   szBuf[4096];
        size_t nRead = 0;
        
        nRead = fread(&szBuf[0], sizeof(szBuf[0]), sizeof(szBuf), pIn);

        if (nRead > 0)
        {
            size_t  nWritten = fwrite(&szBuf[0], sizeof(szBuf[0]), nRead, pOut);

            if (nWritten != nRead)
            {
#ifndef _WIN32
                dwError = LwErrnoToWin32Error(errno);
#else
                dwError = GetLastError();
#endif
                BAIL_ON_VMCA_ERROR(dwError);
            }
        }
    }

cleanup:

    VMCA_LOCK_MUTEX_UNLOCK(&gVMCAServerGlobals.svcMutex, bLocked);

    if (pIn)
    {
        fclose(pIn);
    }
    if (pOut)
    {
        fclose(pOut);
    }
    VMCA_SAFE_FREE_STRINGA(pszCRLFile);
    VMCA_SAFE_FREE_STRINGA(pszTmpFile);

    return dwError;

error:

    // TODO : Should we delete a partially written file?

    VMCA_LOG_ERROR("Failed to copy temporary CRL [Error code: %u ]", dwError);

    dwError = VMCA_FILE_IO_ERROR;

    goto cleanup;
}

unsigned int
VMCAReGenCRL()
{

    DWORD dwError = 0;
    PSTR pszTmpFile = NULL;
    DWORD dwExists = 0;

    dwError = VMCAGetTempCRLNamePath(&pszTmpFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACheckFileExists(pszTmpFile,&dwExists);
    if(dwExists == FILE_EXISTS) {

        dwError = remove(pszTmpFile);

        if ( dwError == -1) {
            dwError = VMCA_FILE_REMOVE_ERROR;
        }
        BAIL_ON_VMCA_ERROR(dwError);
    }
    dwError = VMCAGenerateCRL();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACopyTempCRLtoCRL();
    BAIL_ON_VMCA_ERROR(dwError);

error :

    VMCA_SAFE_FREE_STRINGA(pszTmpFile);
    return dwError;
}

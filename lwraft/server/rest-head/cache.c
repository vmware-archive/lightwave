/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

DWORD
VmDirRESTCacheInit(
    PVDIR_REST_HEAD_CACHE*  ppRestCache
    )
{
    DWORD   dwError = 0;
    PVDIR_REST_HEAD_CACHE   pRestCache = NULL;

    if (!ppRestCache)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_REST_HEAD_CACHE), (PVOID*)&pRestCache);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateRWLock(&pRestCache->pRWLock);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTCacheRefresh(pRestCache);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppRestCache = pRestCache;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VmDirFreeRESTCache(pRestCache);
    goto cleanup;
}

DWORD
VmDirRESTCacheRefresh(
    PVDIR_REST_HEAD_CACHE   pRestCache
    )
{
    DWORD   dwError = 0;
    DWORD   dwAFDError = 0;
    DWORD   dwOIDCError = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszDCName = NULL;
    PSTR    pszDomainName = NULL;
    PSTR    pszOIDCSigningCertificatePEM = NULL;
    POIDC_SERVER_METADATA   pOidcMetadata = NULL;

    if (!pRestCache)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwAFDError = gpVdirVmAfdApi->pfnGetDCName(NULL, &pszDCName);
    dwError = dwAFDError ? VMDIR_ERROR_AFD_UNAVAILABLE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwAFDError = gpVdirVmAfdApi->pfnGetDomainName(NULL, &pszDomainName);
    dwError = dwAFDError ? VMDIR_ERROR_AFD_UNAVAILABLE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwOIDCError = OidcServerMetadataAcquire(
            &pOidcMetadata,
            pszDCName,
            VMDIR_REST_OIDC_PORT,
            pszDomainName,
            NULL /* pszTlsCAPath: NULL means skip TLS validation, pass LIGHTWAVE_TLS_CA_PATH to turn on */);
    dwError = dwOIDCError ? VMDIR_ERROR_OIDC_UNAVAILABLE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            OidcServerMetadataGetSigningCertificatePEM(pOidcMetadata),
            &pszOIDCSigningCertificatePEM);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RWLOCK_WRITELOCK(bInLock, pRestCache->pRWLock, 0);

    VMDIR_SAFE_FREE_MEMORY(pRestCache->pszDCName);
    pRestCache->pszDCName = pszDCName;

    VMDIR_SAFE_FREE_MEMORY(pRestCache->pszDomainName);
    pRestCache->pszDomainName = pszDomainName;

    VMDIR_SAFE_FREE_MEMORY(pRestCache->pszOIDCSigningCertificatePEM);
    pRestCache->pszOIDCSigningCertificatePEM = pszOIDCSigningCertificatePEM;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, pRestCache->pRWLock);
    OidcServerMetadataDelete(pOidcMetadata);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d) AFD error (%d)  OIDC error (%d)",
            __FUNCTION__,
            dwError,
            dwAFDError,
            dwOIDCError);

    VMDIR_SAFE_FREE_MEMORY(pszDCName);
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszOIDCSigningCertificatePEM);
    goto cleanup;
}

DWORD
VmDirRESTCacheGetDCName(
    PVDIR_REST_HEAD_CACHE   pRestCache,
    PSTR*                   ppszDCName
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszDCName = NULL;

    if (!pRestCache || !ppszDCName)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_READLOCK(bInLock, gpVdirRestCache->pRWLock, 0);

    dwError = VmDirAllocateStringA(pRestCache->pszDCName, &pszDCName);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszDCName = pszDCName;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gpVdirRestCache->pRWLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pszDCName);
    goto cleanup;
}

DWORD
VmDirRESTCacheGetDomainName(
    PVDIR_REST_HEAD_CACHE   pRestCache,
    PSTR*                   ppszDomainName
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszDomainName = NULL;

    if (!pRestCache || !ppszDomainName)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_READLOCK(bInLock, gpVdirRestCache->pRWLock, 0);

    dwError = VmDirAllocateStringA(pRestCache->pszDomainName, &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszDomainName = pszDomainName;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gpVdirRestCache->pRWLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    goto cleanup;
}

DWORD
VmDirRESTCacheGetOIDCSigningCertPEM(
    PVDIR_REST_HEAD_CACHE   pRestCache,
    PSTR*                   ppszOIDCSigningCertPEM
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszOIDCSigningCertificatePEM = NULL;

    if (!pRestCache || !ppszOIDCSigningCertPEM)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_READLOCK(bInLock, gpVdirRestCache->pRWLock, 0);

    dwError = VmDirAllocateStringA(
            pRestCache->pszOIDCSigningCertificatePEM,
            &pszOIDCSigningCertificatePEM);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszOIDCSigningCertPEM = pszOIDCSigningCertificatePEM;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gpVdirRestCache->pRWLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pszOIDCSigningCertificatePEM);
    goto cleanup;
}

VOID
VmDirFreeRESTCache(
    PVDIR_REST_HEAD_CACHE   pRestCache
    )
{
    BOOLEAN bInLock = FALSE;

    if (pRestCache)
    {
        VMDIR_RWLOCK_WRITELOCK(bInLock, pRestCache->pRWLock, 0);

        VMDIR_SAFE_FREE_MEMORY(pRestCache->pszDCName);
        VMDIR_SAFE_FREE_MEMORY(pRestCache->pszDomainName);
        VMDIR_SAFE_FREE_MEMORY(pRestCache->pszOIDCSigningCertificatePEM);

        VMDIR_RWLOCK_UNLOCK(bInLock, pRestCache->pRWLock);

        VMDIR_SAFE_FREE_RWLOCK(pRestCache->pRWLock);
        VMDIR_SAFE_FREE_MEMORY(pRestCache);
    }
}

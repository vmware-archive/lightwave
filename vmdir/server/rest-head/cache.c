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

    dwError = LwRtlCreateHashMap(
            &pRestCache->pOIDCSigningCertPEM,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
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
    PVDIR_REST_HEAD_CACHE pRestCache,
    PCSTR                 pszDomainName
    )
{
    DWORD   dwError = 0;
    DWORD   dwOIDCError = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszKey = NULL;
    PSTR    pszOIDCSigningCertPEM = NULL;
    POIDC_SERVER_METADATA   pOidcMetadata = NULL;

    if (!pRestCache)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwOIDCError = OidcServerMetadataAcquire(
            &pOidcMetadata,
            VDIR_SAFE_STRING(gVmdirServerGlobals.bvServerObjName.lberbv_val),
            VMDIR_REST_OIDC_PORT,
            pszDomainName,
            LIGHTWAVE_TLS_CA_PATH);
    dwError = VmDirOidcToVmdirError(dwOIDCError);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            OidcServerMetadataGetSigningCertificatePEM(pOidcMetadata),
            &pszOIDCSigningCertPEM);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszDomainName, &pszKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RWLOCK_WRITELOCK(bInLock, pRestCache->pRWLock, 0);

    dwError = LwRtlHashMapInsert(
            pRestCache->pOIDCSigningCertPEM,
            pszKey,
            pszOIDCSigningCertPEM,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, pRestCache->pRWLock);
    OidcServerMetadataDelete(pOidcMetadata);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pszOIDCSigningCertPEM);
    VMDIR_SAFE_FREE_MEMORY(pszKey);
    goto cleanup;
}

DWORD
VmDirRESTCacheGetOIDCSigningCertPEM(
    PVDIR_REST_HEAD_CACHE   pRestCache,
    PCSTR                   pszDomainName,
    PSTR*                   ppszOIDCSigningCertPEM
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszOIDCSigningCertPEM = NULL;
    PSTR    pszVal = NULL;

    if (!pRestCache || !ppszOIDCSigningCertPEM)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_READLOCK(bInLock, gpVdirRestCache->pRWLock, 0);

    if (LwRtlHashMapFindKey(pRestCache->pOIDCSigningCertPEM, (PVOID*)&pszVal, pszDomainName) ||
        IsNullOrEmptyString(pszVal))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_ENTRY_NOT_FOUND);
    }

    dwError = VmDirAllocateStringA(
            pszVal, &pszOIDCSigningCertPEM);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszOIDCSigningCertPEM = pszOIDCSigningCertPEM;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gpVdirRestCache->pRWLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pszOIDCSigningCertPEM);
    goto cleanup;
}

static
VOID
_SigningCertPEMFree(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VMDIR_SAFE_FREE_MEMORY(pPair->pKey);
    VMDIR_SAFE_FREE_MEMORY(pPair->pValue);
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

        LwRtlHashMapClear(
                pRestCache->pOIDCSigningCertPEM,
                _SigningCertPEMFree,
                NULL);
        LwRtlFreeHashMap(&pRestCache->pOIDCSigningCertPEM);

        VMDIR_RWLOCK_UNLOCK(bInLock, pRestCache->pRWLock);

        VMDIR_SAFE_FREE_RWLOCK(pRestCache->pRWLock);
        VMDIR_SAFE_FREE_MEMORY(pRestCache);
    }
}

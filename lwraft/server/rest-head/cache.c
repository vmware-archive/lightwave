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
    BOOLEAN bInLock = FALSE;
    PSTR    pszDCName = NULL;
    PSTR    pszDomainName = NULL;
    PSTR    pszOIDCSigningCertPEM = NULL;
    PSID    pBuiltInAdminsGroupSid = NULL;

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

    // OIDC signing certificate PEM
    dwError = VmDirRESTGetLightwaveOIDCSigningCertPEM(
            pszDCName, pszDomainName, &pszOIDCSigningCertPEM);
    BAIL_ON_VMDIR_ERROR(dwError);

    // built-in administrators group sid
    dwError = VmDirRESTGetLightwaveBuiltInAdminsGroupSid(
            pszDCName, pszDomainName, &pBuiltInAdminsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RWLOCK_WRITELOCK(bInLock, pRestCache->pRWLock, 0);

    VMDIR_SAFE_FREE_MEMORY(pRestCache->pszOIDCSigningCertPEM);
    pRestCache->pszOIDCSigningCertPEM = pszOIDCSigningCertPEM;

    VMDIR_SAFE_FREE_MEMORY(pRestCache->pBuiltInAdminsGroupSid);
    pRestCache->pBuiltInAdminsGroupSid = pBuiltInAdminsGroupSid;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, pRestCache->pRWLock);
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszDCName);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d) AFD error (%d)",
            __FUNCTION__,
            dwError,
            dwAFDError);

    VMDIR_SAFE_FREE_MEMORY(pBuiltInAdminsGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pszOIDCSigningCertPEM);
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
    PSTR    pszOIDCSigningCertPEM = NULL;

    if (!pRestCache || !ppszOIDCSigningCertPEM)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_READLOCK(bInLock, gpVdirRestCache->pRWLock, 0);

    if (IsNullOrEmptyString(pRestCache->pszOIDCSigningCertPEM))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNAVAILABLE);
    }

    dwError = VmDirAllocateStringA(
            pRestCache->pszOIDCSigningCertPEM, &pszOIDCSigningCertPEM);
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

DWORD
VmDirRESTCacheGetBuiltInAdminsGroupSid(
    PVDIR_REST_HEAD_CACHE   pRestCache,
    PSID*                   ppBuiltInAdminsGroupSid
    )
{
    DWORD   dwError = 0;
    ULONG   ulSidLen = 0;
    BOOLEAN bInLock = FALSE;
    PSID    pSid = NULL;

    if (!pRestCache || !ppBuiltInAdminsGroupSid)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_READLOCK(bInLock, gpVdirRestCache->pRWLock, 0);

    if (!RtlValidSid(pRestCache->pBuiltInAdminsGroupSid))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNAVAILABLE);
    }

    ulSidLen = RtlLengthSid(pRestCache->pBuiltInAdminsGroupSid);

    dwError = VmDirAllocateMemory(ulSidLen, (PVOID*)&pSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RtlCopySid(ulSidLen, pSid, pRestCache->pBuiltInAdminsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppBuiltInAdminsGroupSid = pSid;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gpVdirRestCache->pRWLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pSid);
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

        VMDIR_SAFE_FREE_MEMORY(pRestCache->pBuiltInAdminsGroupSid);
        VMDIR_SAFE_FREE_MEMORY(pRestCache->pszOIDCSigningCertPEM);

        VMDIR_RWLOCK_UNLOCK(bInLock, pRestCache->pRWLock);

        VMDIR_SAFE_FREE_RWLOCK(pRestCache->pRWLock);
        VMDIR_SAFE_FREE_MEMORY(pRestCache);
    }
}

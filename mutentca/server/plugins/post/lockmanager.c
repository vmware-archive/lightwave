/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

static
DWORD
_LwCALockDNImpl(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszUUID,
    ULONG               leaseEnd,
    PCSTR               pcszDN
    );

static
DWORD
_LwCAGetCurrentTime(
    ULONG   *pCurrTime
    );

DWORD
LwCALockDN(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszDN,
    PSTR                *ppszUuid
    )
{
    DWORD               dwError = 0;
    PSTR                pszUuid = NULL;
    ULONG               currTime = 0;
    DWORD               dwAttempt = 0;
    struct timespec     ts = {0};

    if (IsNullOrEmptyString(pcszDN) || !ppszUuid || !pHandle)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAUuidGenerate(&pszUuid);
    BAIL_ON_LWCA_ERROR(dwError);

    ts.tv_nsec = LWCA_LOCK_SLEEP_NS;

    while (dwAttempt < LWCA_LOCK_MAX_ATTEMPT)
    {
        dwError = _LwCAGetCurrentTime(&currTime);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _LwCALockDNImpl(pHandle, pszUuid, currTime, pcszDN);
        if (dwError == LWCA_LOCK_APPLY_FAILED)
        {
            dwAttempt += 1;
            nanosleep(&ts, NULL);
            continue;
        }
        BAIL_ON_LWCA_ERROR(dwError);

        break;
    }
    if (dwAttempt >= LWCA_LOCK_MAX_ATTEMPT)
    {
        dwError = LWCA_LOCK_APPLY_FAILED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppszUuid = pszUuid;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszUuid);
    if (ppszUuid)
    {
        *ppszUuid = NULL;
    }
    goto cleanup;
}

DWORD
LwCAUnlockDN(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszDN,
    PCSTR               pcszUUID
    )
{
    DWORD       dwError = 0;
    PSTR        pszCond = NULL;
    PSTR        pszReqBody = NULL;
    PSTR        pszResponse = NULL;
    long        statusCode = 0;

    if (IsNullOrEmptyString(pcszDN) || IsNullOrEmptyString(pcszUUID) || !pHandle)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringPrintfA(&pszCond,
                                        "%s=%s",
                                        LWCA_POST_ATTR_LOCK_OWNER,
                                        pcszUUID
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateLockRequestBody(LWCA_LOCK_UNOWNED,
                                          LWCA_LOCK_UNOWNED_EXPIRE_TIME,
                                          &pszReqBody
                                          );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestExecutePatchImpl(pHandle,
                                       pcszDN,
                                       pszReqBody,
                                       pszCond,
                                       &pszResponse,
                                       &statusCode
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    LWCA_SAFE_FREE_STRINGA(pszCond);
    LWCA_SAFE_FREE_STRINGA(pszReqBody);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCALockDNImpl(
    PLWCA_POST_HANDLE   pHandle,
    PCSTR               pcszUUID,
    ULONG               leaseEnd,
    PCSTR               pcszDN
    )
{
    DWORD       dwError = 0;
    ULONG       currTime = 0;
    ULONG       lockExpire = 0;
    PSTR        pszCond = NULL;
    PSTR        pszReqBody = NULL;
    PSTR        pszResponse = NULL;
    long        statusCode = 0;

    if (IsNullOrEmptyString(pcszDN) || IsNullOrEmptyString(pcszUUID) || !pHandle)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAGetCurrentTime(&currTime);
    BAIL_ON_LWCA_ERROR(dwError);

    /**
     * LWCA_LOCK_MAX_DRIFT is used to compensate for the drift in time between
     * nodes, to avoid a node that has a positive drift from prematurely
     * acquiring the lock before it has actually expired. The following cases
     * are possible:
     *  1. systemDrift <= -LWCA_LOCK_MAX_DRIFT
     *  2. -LWCA_LOCK_MAX_DRIFT < systemDrift <= 0
     *  3. 0 < systemDrift <= LWCA_LOCK_MAX_DRIFT
     *  4. LWCA_LOCK_MAX_DRIFT < systemDrift
     *
     *  For case 1-3, the node could wait an extra time to acquire an expired
     *  lock or could get it immediately. Case 4 is when the node could
     *  pre-maturely acquire an expired lock.
     *
     *  When locks are requested, the expirationTime is set from the reference
     *  of the requesting node. The clock of the POST server is not used.
     */
    lockExpire = leaseEnd + LWCA_LOCK_MAX_DRIFT;

    if (currTime > lockExpire)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringPrintfA(&pszCond,
                                        LWCA_LOCK_CONDITION,
                                        pcszUUID,
                                        leaseEnd
                                        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGenerateLockRequestBody(pcszUUID,
                                          lockExpire,
                                          &pszReqBody
                                          );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestExecutePatchImpl(pHandle,
                                       pcszDN,
                                       pszReqBody,
                                       pszCond,
                                       &pszResponse,
                                       &statusCode
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    if (statusCode == LWCA_HTTP_LOCK_HELD)
    {
        dwError = LWCA_LOCK_APPLY_FAILED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszResponse);
    LWCA_SAFE_FREE_STRINGA(pszCond);
    LWCA_SAFE_FREE_STRINGA(pszReqBody);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCAGetCurrentTime(
    ULONG   *pCurrTime
    )
{
    DWORD   dwError = 0;
    time_t  currTime = 0;

    if (!pCurrTime)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    currTime = time(NULL);
    if (currTime == (time_t) - 1)
    {
        dwError = LWCA_INVALID_TIME_SPECIFIED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *pCurrTime = (ULONG)currTime;

cleanup:
    return dwError;

error:
    goto cleanup;
}


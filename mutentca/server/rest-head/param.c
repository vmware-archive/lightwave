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

DWORD
LwCARestGetStrParam(
    PLWCA_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    PSTR*                   ppszVal,
    BOOLEAN                 bRequired
    )
{
    DWORD   dwError = 0;
    PSTR    pszVal  = NULL;

    if (!pRestOp || IsNullOrEmptyString(pszKey) || !ppszVal)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (LwRtlHashMapFindKey(pRestOp->pParamMap, (PVOID*)&pszVal, pszKey) ||
        IsNullOrEmptyString(pszVal))
    {
        dwError = bRequired ? LWCA_ERROR_INVALID_REQUEST : 0;
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        dwError = LwCAAllocateStringA(pszVal, ppszVal);
        BAIL_ON_LWCA_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d) (pszKey=%s)",
            __FUNCTION__,
            dwError,
            LWCA_SAFE_STRING(pszKey));

    goto cleanup;
}

DWORD
LwCARestGetIntParam(
    PLWCA_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    int*                    piVal,
    BOOLEAN                 bRequired
    )
{
    DWORD   dwError = 0;
    PSTR    pszVal = NULL;

    if (!pRestOp || IsNullOrEmptyString(pszKey) || !piVal)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (LwRtlHashMapFindKey(pRestOp->pParamMap, (PVOID*)&pszVal, pszKey) ||
        IsNullOrEmptyString(pszVal))
    {
        dwError = bRequired ? LWCA_ERROR_INVALID_REQUEST : 0;
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        *piVal = LwCAStringToInt(pszVal);
    }

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d) (pszKey=%s)",
            __FUNCTION__,
            dwError,
            LWCA_SAFE_STRING(pszKey));

    goto cleanup;
}

DWORD
LwCARestGetBoolParam(
    PLWCA_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    BOOLEAN*                pbVal,
    BOOLEAN                 bRequired
    )
{
    DWORD   dwError = 0;
    PSTR    pszVal = NULL;

    if (!pRestOp || IsNullOrEmptyString(pszKey) || !pbVal)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (LwRtlHashMapFindKey(pRestOp->pParamMap, (PVOID*)&pszVal, pszKey) ||
        IsNullOrEmptyString(pszVal))
    {
        dwError = bRequired ? LWCA_ERROR_INVALID_REQUEST : 0;
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else if (LwCAStringCompareA(pszVal, "true", FALSE) == 0)
    {
        *pbVal = TRUE;
    }
    else if (LwCAStringCompareA(pszVal, "false", FALSE) == 0)
    {
        *pbVal = FALSE;
    }
    else
    {
        dwError = LWCA_ERROR_INVALID_REQUEST;
        BAIL_ON_LWCA_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d) (pszKey=%s)",
            __FUNCTION__,
            dwError,
            LWCA_SAFE_STRING(pszKey));

    goto cleanup;
}

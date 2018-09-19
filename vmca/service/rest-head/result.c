/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

DWORD
VMCARestResultCreate(
    PVMCA_REST_RESULT*  ppRestRslt
    )
{
    DWORD               dwError     = 0;
    PVMCA_REST_RESULT   pRestRslt   = NULL;

    if (!ppRestRslt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
            sizeof(VMCA_REST_RESULT), (PVOID*)&pRestRslt);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pRestRslt->pDataMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMCA_ERROR(dwError);

    pRestRslt->bErrSet = FALSE;
    pRestRslt->dwBodyLen = 0;

    *ppRestRslt = pRestRslt;

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMCAFreeRESTResult(pRestRslt);
    goto cleanup;
}

DWORD
VMCARestResultSetError(
    PVMCA_REST_RESULT   pRestRslt,
    int                 errCode,
    PSTR                pszErrMsg
    )
{
    DWORD dwError = 0;

    if (!pRestRslt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!pRestRslt->bErrSet && errCode)
    {
        VMCA_SAFE_FREE_MEMORY(pRestRslt->pszErrMsg);
        dwError = VMCAAllocateStringA(
                VMCA_SAFE_STRING(pszErrMsg), &pRestRslt->pszErrMsg);
        BAIL_ON_VMCA_ERROR(dwError);

        pRestRslt->errCode = errCode;
        pRestRslt->bErrSet = TRUE;
    }

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VMCARestResultUnsetError(
    PVMCA_REST_RESULT   pRestRslt
    )
{
    DWORD dwError = 0;

    if (!pRestRslt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    VMCA_SAFE_FREE_MEMORY(pRestRslt->pszErrMsg);
    pRestRslt->pszErrMsg = NULL;
    pRestRslt->errCode = 0;
    pRestRslt->bErrSet = FALSE;

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VMCARestResultSetStrData(
    PVMCA_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    PSTR                pszVal
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pszVal))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCARestResultSetObjData(pRestRslt, pszKey, json_string(pszVal));
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VMCARestResultSetIntData(
    PVMCA_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    int                 iVal
    )
{
    DWORD dwError = 0;

    dwError = VMCARestResultSetObjData(pRestRslt, pszKey, json_integer(iVal));
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VMCARestResultSetBooleanData(
    PVMCA_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    BOOLEAN             bVal
    )
{
    DWORD dwError = 0;

    dwError = VMCARestResultSetStrData(
                  pRestRslt,
                  pszKey,
                  bVal ? "true" : "false");
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VMCARestResultSetObjData(
    PVMCA_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    json_t*             pjVal
    )
{
    DWORD   dwError  = 0;
    PSTR    pszKeyCp = NULL;

    if (!pRestRslt || IsNullOrEmptyString(pszKey) || !pjVal)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateStringA(pszKey, &pszKeyCp);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = LwRtlHashMapInsert(pRestRslt->pDataMap, pszKeyCp, pjVal, NULL);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMCA_SAFE_FREE_MEMORY(pszKeyCp);
    goto cleanup;
}

DWORD
VMCARestResultGenerateResponseBody(
    PVMCA_REST_RESULT   pRestRslt,
    PVMCA_REST_RESOURCE pResource
    )
{
    DWORD               dwError     = 0;
    json_t*             pjBody      = NULL;
    json_t*             pjErrCode   = NULL;
    json_t*             pjErrMsg    = NULL;
    json_t*             pjData      = NULL;
    LW_HASHMAP_ITER     iter        = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR     pair        = {NULL, NULL};

    if (!pRestRslt || !pResource)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pResource->rscType == VMCA_REST_RSC_UNKNOWN)
    {
        // don't produce response body if resource is unknown
        goto cleanup;
    }

    if (VMCAStringCompareA(pResource->pszContentType, "application/json", FALSE) == 0)
    {
        pjBody = json_object();

        if (pRestRslt->errCode)
        {
            pjErrCode = json_integer(pRestRslt->errCode);
            dwError = json_object_set_new(
                    pjBody, pResource->pszErrCodeKey, pjErrCode);
            BAIL_ON_VMCA_ERROR(dwError);

            pjErrMsg = json_string(VMCA_SAFE_STRING(pRestRslt->pszErrMsg));
            dwError = json_object_set_new(
                    pjBody, pResource->pszErrMsgKey, pjErrMsg);
            BAIL_ON_VMCA_ERROR(dwError);
        }

        while (LwRtlHashMapIterate(pRestRslt->pDataMap, &iter, &pair))
        {
            pjData = (json_t*)pair.pValue;
            dwError = json_object_set(pjBody, (PSTR)pair.pKey, pjData);
            BAIL_ON_VMCA_ERROR(dwError);
        }

        if (json_object_size(pjBody))
        {
            pRestRslt->pszBody = json_dumps(pjBody, JSON_COMPACT);
            pRestRslt->dwBodyLen = VMCAStringLenA(pRestRslt->pszBody);
        }
    }

cleanup:
    if (pjBody)
    {
        json_decref(pjBody);
    }
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

static
VOID
_DataMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VMCA_SAFE_FREE_MEMORY(pPair->pKey);
    if (pPair->pValue)
    {
        json_t* pjData = (json_t*)pPair->pValue;
        json_decref(pjData);
    }
}

VOID
VMCASetRestResult(
    PVMCA_REST_OPERATION    pRestOp,
    DWORD                   dwError,
    PSTR                    pszErrMsg
    )
{
    PVMCA_REST_RESOURCE pResource = NULL;
    PVMCA_REST_RESULT   pRestRslt = NULL;

    if (pRestOp)
    {
        pResource = ((PVMCA_REST_OPERATION)pRestOp)->pResource;
        pRestRslt = ((PVMCA_REST_OPERATION)pRestOp)->pResult;
        pResource->pfnSetResult(pRestRslt, dwError, pszErrMsg);
    }
}

VOID
VMCAFreeRESTResult(
    PVMCA_REST_RESULT   pRestRslt
    )
{
    if (pRestRslt)
    {
        VMCA_SAFE_FREE_MEMORY(pRestRslt->pszErrMsg);
        LwRtlHashMapClear(pRestRslt->pDataMap, _DataMapPairFree, NULL);
        LwRtlFreeHashMap(&pRestRslt->pDataMap);
        VMCA_SAFE_FREE_MEMORY(pRestRslt->pszBody);
        VMCA_SAFE_FREE_MEMORY(pRestRslt);
    }
}

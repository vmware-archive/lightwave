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
VmDirRESTResultCreate(
    PVDIR_REST_RESULT*  ppRestRslt
    )
{
    DWORD   dwError = 0;
    PVDIR_REST_RESULT   pRestRslt = NULL;

    if (!ppRestRslt)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_REST_RESULT), (PVOID*)&pRestRslt);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pRestRslt->pDataMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    pRestRslt->bErrSet = FALSE;

    *ppRestRslt = pRestRslt;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeRESTResult(pRestRslt);
    goto cleanup;
}

DWORD
VmDirRESTResultSetError(
    PVDIR_REST_RESULT   pRestRslt,
    int                 errCode,
    PSTR                pszErrMsg
    )
{
    DWORD   dwError = 0;

    if (!pRestRslt)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pRestRslt->bErrSet && errCode)
    {
        VMDIR_SAFE_FREE_MEMORY(pRestRslt->pszErrMsg);
        dwError = VmDirAllocateStringA(
                VDIR_SAFE_STRING(pszErrMsg), &pRestRslt->pszErrMsg);
        BAIL_ON_VMDIR_ERROR(dwError);

        pRestRslt->errCode = errCode;
        pRestRslt->bErrSet = TRUE;
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirRESTResultSetStrData(
    PVDIR_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    PSTR                pszVal
    )
{
    DWORD   dwError = 0;

    if (IsNullOrEmptyString(pszVal))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirRESTResultSetObjData(pRestRslt, pszKey, json_string(pszVal));
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

DWORD
VmDirRESTResultSetIntData(
    PVDIR_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    int                 iVal
    )
{
    DWORD   dwError = 0;

    dwError = VmDirRESTResultSetObjData(pRestRslt, pszKey, json_integer(iVal));
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

DWORD
VmDirRESTResultSetObjData(
    PVDIR_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    json_t*             pjVal
    )
{
    DWORD   dwError = 0;
    PSTR    pszKeyCp = NULL;

    if (!pRestRslt || IsNullOrEmptyString(pszKey) || !pjVal)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pszKey, &pszKeyCp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapInsert(pRestRslt->pDataMap, pszKeyCp, pjVal, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pszKeyCp);
    goto cleanup;
}

DWORD
VmDirRESTResultToResponseBody(
    PVDIR_REST_RESULT   pRestRslt,
    PVDIR_REST_RESOURCE pResource,
    PSTR*               ppszBody
    )
{
    DWORD   dwError = 0;
    json_t* pjBody = NULL;
    json_t* pjErrCode = NULL;
    json_t* pjErrMsg = NULL;
    json_t* pjData = NULL;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PSTR    pszBody = NULL;

    if (!pRestRslt || !pResource || !ppszBody)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pResource->rscType == VDIR_REST_RSC_UNKNOWN)
    {
        // don't produce response body if resource is unknown
        goto cleanup;
    }

    pjBody = json_object();

    if (pRestRslt->errCode)
    {
        pjErrCode = json_integer(pRestRslt->errCode);
        dwError = json_object_set_new(
                pjBody, pResource->pszErrCodeKey, pjErrCode);
        BAIL_ON_VMDIR_ERROR(dwError);

        pjErrMsg = json_string(VDIR_SAFE_STRING(pRestRslt->pszErrMsg));
        dwError = json_object_set_new(
                pjBody, pResource->pszErrMsgKey, pjErrMsg);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pRestRslt->pDataMap, &iter, &pair))
    {
        pjData = (json_t*)pair.pValue;
        dwError = json_object_set(pjBody, (PSTR)pair.pKey, pjData);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (json_object_size(pjBody))
    {
        pszBody = json_dumps(pjBody, JSON_INDENT(4));
        *ppszBody = pszBody;
    }

cleanup:
    if (pjBody)
    {
        json_decref(pjBody);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pszBody);
    goto cleanup;
}

static
VOID
_DataMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VMDIR_SAFE_FREE_MEMORY(pPair->pKey);
    if (pPair->pValue)
    {
        json_t* pjData = (json_t*)pPair->pValue;
        json_decref(pjData);
    }
}

VOID
VmDirFreeRESTResult(
    PVDIR_REST_RESULT   pRestRslt
    )
{
    if (pRestRslt)
    {
        VMDIR_SAFE_FREE_MEMORY(pRestRslt->pszErrMsg);
        LwRtlHashMapClear(pRestRslt->pDataMap, _DataMapPairFree, NULL);
        LwRtlFreeHashMap(&pRestRslt->pDataMap);
        VMDIR_SAFE_FREE_MEMORY(pRestRslt);
    }
}

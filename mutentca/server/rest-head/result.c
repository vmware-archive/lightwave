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
LwCARestResultCreate(
    PLWCA_REST_RESULT*  ppRestRslt
    )
{
    DWORD               dwError     = 0;
    PLWCA_REST_RESULT   pRestRslt   = NULL;

    if (!ppRestRslt)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_REST_RESULT), (PVOID*)&pRestRslt);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
                &pRestRslt->pDataMap,
                LwRtlHashDigestPstrCaseless,
                LwRtlHashEqualPstrCaseless,
                NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    pRestRslt->bErrSet = FALSE;
    pRestRslt->dwBodyLen = 0;

    *ppRestRslt = pRestRslt;

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    LwCAFreeRESTResult(pRestRslt);
    goto cleanup;
}

DWORD
LwCARestResultSetError(
    PLWCA_REST_RESULT   pRestRslt,
    int                 errCode,
    PCSTR               pcszErrDetail
    )
{
    DWORD dwError = 0;

    if (!pRestRslt)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!pRestRslt->bErrSet && errCode)
    {
        LWCA_SAFE_FREE_MEMORY(pRestRslt->pszErrDetail);
        dwError = LwCAAllocateStringA(LWCA_SAFE_STRING(pcszErrDetail), &pRestRslt->pszErrDetail);
        BAIL_ON_LWCA_ERROR(dwError);

        pRestRslt->errCode = errCode;
        pRestRslt->bErrSet = TRUE;
    }

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
LwCARestResultUnsetError(
    PLWCA_REST_RESULT   pRestRslt
    )
{
    DWORD dwError = 0;

    if (!pRestRslt)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_SAFE_FREE_MEMORY(pRestRslt->pszErrDetail);
    pRestRslt->pszErrDetail = NULL;
    pRestRslt->errCode = 0;
    pRestRslt->bErrSet = FALSE;

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
LwCARestResultSetStrData(
    PLWCA_REST_RESULT   pRestRslt,
    PCSTR               pcszKey,
    PCSTR               pcszVal
    )
{
    DWORD dwError = 0;

    if (!pRestRslt || IsNullOrEmptyString(pcszKey) || IsNullOrEmptyString(pcszVal))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCARestResultSetObjData(pRestRslt, pcszKey, json_string(pcszVal));
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
LwCARestResultSetIntData(
    PLWCA_REST_RESULT   pRestRslt,
    PCSTR               pcszKey,
    int                 iVal
    )
{
    DWORD dwError = 0;

    if (!pRestRslt || IsNullOrEmptyString(pcszKey))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCARestResultSetObjData(pRestRslt, pcszKey, json_integer(iVal));
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
LwCARestResultSetBooleanData(
    PLWCA_REST_RESULT   pRestRslt,
    PCSTR               pcszKey,
    BOOLEAN             bVal
    )
{
    DWORD dwError = 0;

    if (!pRestRslt || IsNullOrEmptyString(pcszKey))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCARestResultSetStrData(
                  pRestRslt,
                  pcszKey,
                  bVal ? "true" : "false");
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
LwCARestResultSetStrArrayData(
    PLWCA_REST_RESULT   pRestRslt,
    PCSTR               pcszKey,
    PLWCA_STRING_ARRAY  pVal
    )
{
    DWORD               dwError     = 0;
    int                 iJsonErr    = 0;
    DWORD               dwIdx       = 0;
    PLWCA_JSON_OBJECT   pjArray     = NULL;

    if (!pRestRslt || IsNullOrEmptyString(pcszKey) || !pVal)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pjArray = json_array();
    if (!pjArray)
    {
        dwError = LWCA_JSON_ERROR;
        BAIL_ON_JSON_ERROR_WITH_MSG(dwError, "Failed to create json array result data");
    }

    for (; dwIdx < pVal->dwCount; ++dwIdx)
    {
        iJsonErr = json_array_append_new(pjArray, json_string(pVal->ppData[dwIdx]));
        if (iJsonErr)
        {
            dwError = LWCA_JSON_ERROR;
            BAIL_ON_JSON_ERROR_WITH_MSG(dwError, "Failed to append to json array result data");
        }
    }

    dwError = LwCARestResultSetObjData(pRestRslt, pcszKey, pjArray);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
LwCARestResultSetCertArrayData(
    PLWCA_REST_RESULT       pRestRslt,
    PCSTR                   pcszKey,
    PLWCA_CERTIFICATE_ARRAY pVal
    )
{
    DWORD               dwError     = 0;
    int                 iJsonErr    = 0;
    DWORD               dwIdx       = 0;
    PLWCA_JSON_OBJECT   pjArray     = NULL;

    if (!pRestRslt || IsNullOrEmptyString(pcszKey) || !pVal)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pjArray = json_array();
    if (!pjArray)
    {
        dwError = LWCA_JSON_ERROR;
        BAIL_ON_JSON_ERROR_WITH_MSG(dwError, "Failed to create json array result data");
    }

    for (; dwIdx < pVal->dwCount; ++dwIdx)
    {
        iJsonErr = json_array_append_new(pjArray, json_string(pVal->ppCertificates[dwIdx]));
        if (iJsonErr)
        {
            dwError = LWCA_JSON_ERROR;
            BAIL_ON_JSON_ERROR_WITH_MSG(dwError, "Failed to append to json array result data");
        }
    }

    dwError = LwCARestResultSetObjData(pRestRslt, pcszKey, pjArray);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
LwCARestResultSetObjData(
    PLWCA_REST_RESULT   pRestRslt,
    PCSTR               pcszKey,
    PLWCA_JSON_OBJECT   pjVal
    )
{
    DWORD   dwError  = 0;
    PSTR    pszKeyCp = NULL;

    if (!pRestRslt || IsNullOrEmptyString(pcszKey) || !pjVal)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(pcszKey, &pszKeyCp);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwRtlHashMapInsert(pRestRslt->pDataMap, pszKeyCp, pjVal, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    LWCA_SAFE_FREE_MEMORY(pszKeyCp);
    goto cleanup;
}

DWORD
LwCARestResultGenerateResponseBody(
    PLWCA_REST_RESULT   pRestRslt,
    PLWCA_REST_RESOURCE pResource,
    PLWCA_HTTP_ERROR    pHttpError
    )
{
    DWORD               dwError         = 0;
    int                 iJsonErr        = 0;
    PLWCA_JSON_OBJECT   pjBody          = NULL;
    PLWCA_JSON_OBJECT   pjReqId         = NULL;
    PLWCA_JSON_OBJECT   pjStrErrCode    = NULL;
    PLWCA_JSON_OBJECT   pjErrCode       = NULL;
    PLWCA_JSON_OBJECT   pjErrMsg        = NULL;
    PLWCA_JSON_OBJECT   pjErrDetail     = NULL;
    PLWCA_JSON_OBJECT   pjData          = NULL;
    LW_HASHMAP_ITER     iter            = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR     pair            = {NULL, NULL};

    if (!pRestRslt || !pResource)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pResource->rscType == LWCA_REST_RSC_UNKNOWN)
    {
        // don't produce response body if resource is unknown
        goto cleanup;
    }

    if (LwCAStringCompareA(pResource->pszContentType, "application/json", FALSE) == 0)
    {
        pjBody = json_object();
        if (!pjBody)
        {
            dwError = LWCA_JSON_ERROR;
            BAIL_ON_JSON_ERROR_WITH_MSG(dwError, "Failed to create json object for response");
        }

        if (pRestRslt->errCode)
        {
            pjReqId = json_string(LWCA_SAFE_STRING(pRestRslt->pszRequestId));
            dwError = json_object_set_new(pjBody, "requestId", pjReqId);
            BAIL_ON_JSON_ERROR_WITH_MSG(dwError, "Failed to set new json object 'requestId'");

            pjStrErrCode = json_string(pHttpError->pszHttpStatus);
            iJsonErr = json_object_set_new(pjBody, "code", pjStrErrCode);
            if (iJsonErr)
            {
                dwError = LWCA_JSON_ERROR;
                BAIL_ON_JSON_ERROR_WITH_MSG(dwError, "Failed to set new json object 'code'");
            }

            pjErrCode = json_integer(pHttpError->httpStatus);
            iJsonErr = json_object_set_new(pjBody, "errorCode", pjErrCode);
            if (iJsonErr)
            {
                dwError = LWCA_JSON_ERROR;
                BAIL_ON_JSON_ERROR_WITH_MSG(dwError, "Failed to set new json object 'errorCode'");
            }

            pjErrMsg = json_string(pHttpError->pszHttpReason);
            iJsonErr = json_object_set_new(pjBody, "message", pjErrMsg);
            if (iJsonErr)
            {
                dwError = LWCA_JSON_ERROR;
                BAIL_ON_JSON_ERROR_WITH_MSG(dwError, "Failed to set new json object 'message'");
            }

            pjErrDetail = json_string(LWCA_SAFE_STRING(pRestRslt->pszErrDetail));
            iJsonErr = json_object_set_new(pjBody, "detail", pjErrDetail);
            if (iJsonErr)
            {
                dwError = LWCA_JSON_ERROR;
                BAIL_ON_JSON_ERROR_WITH_MSG(dwError, "Failed to set new json object 'detail'");
            }
        }

        while (LwRtlHashMapIterate(pRestRslt->pDataMap, &iter, &pair))
        {
            pjData = (PLWCA_JSON_OBJECT)pair.pValue;
            iJsonErr = json_object_set(pjBody, (PSTR)pair.pKey, pjData);
            if (iJsonErr)
            {
                dwError = LWCA_JSON_ERROR;
                BAIL_ON_JSON_ERROR_WITH_MSG(dwError, "Failed to set json object");
            }
        }

        if (json_object_size(pjBody))
        {
            pRestRslt->pszBody = json_dumps(pjBody, JSON_COMPACT);
            pRestRslt->dwBodyLen = LwCAStringLenA(pRestRslt->pszBody);
        }
    }

cleanup:
    LwCAJsonCleanupObject(pjBody);

    return dwError;

error:
    LWCA_LOG_ERROR(
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
    PLWCA_JSON_OBJECT pjData = NULL;

    LWCA_SAFE_FREE_MEMORY(pPair->pKey);
    if (pPair->pValue)
    {
        pjData = (PLWCA_JSON_OBJECT)pPair->pValue;
        LwCAJsonCleanupObject(pjData);
    }
}

VOID
LwCASetRestResult(
    PLWCA_REST_OPERATION    pRestOp,
    DWORD                   dwError
    )
{
    PLWCA_REST_RESOURCE pResource = NULL;
    PLWCA_REST_RESULT   pRestRslt = NULL;

    if (pRestOp)
    {
        pResource = ((PLWCA_REST_OPERATION)pRestOp)->pResource;
        pRestRslt = ((PLWCA_REST_OPERATION)pRestOp)->pResult;
        pResource->pfnSetResult(pRestRslt, pRestOp->pszRequestId, dwError, LwCAGetErrorDescription(dwError));
    }
}

VOID
LwCAFreeRESTResult(
    PLWCA_REST_RESULT   pRestRslt
    )
{
    if (pRestRslt)
    {
        LWCA_SAFE_FREE_STRINGA(pRestRslt->pszRequestId);
        LWCA_SAFE_FREE_MEMORY(pRestRslt->pszErrDetail);
        LwRtlHashMapClear(pRestRslt->pDataMap, _DataMapPairFree, NULL);
        LwRtlFreeHashMap(&pRestRslt->pDataMap);
        LWCA_SAFE_FREE_STRINGA(pRestRslt->pszBody);
        LWCA_SAFE_FREE_MEMORY(pRestRslt);
    }
}

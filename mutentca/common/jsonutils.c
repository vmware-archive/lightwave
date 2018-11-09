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
LwCAJsonLoadObjectFromFile(
    PCSTR                   pcszFilePath,
    PLWCA_JSON_OBJECT       *ppJsonConfig
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PLWCA_JSON_OBJECT       pJsonConfig = NULL;
    size_t                  szJsonFlags = JSON_DECODE_ANY;

    if (IsNullOrEmptyString(pcszFilePath) || !ppJsonConfig)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!(pJsonConfig = json_load_file(pcszFilePath, szJsonFlags, &jsonError)))
    {
        LWCA_LOG_ERROR(
                "[%s,%d] Failed to open config file (%s). Error: %s:%d",
                __FUNCTION__,
                __LINE__,
                pcszFilePath,
                jsonError.text,
                jsonError.line);
        dwError = LWCA_JSON_FILE_LOAD_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppJsonConfig = pJsonConfig;


cleanup:

    return dwError;

error:

    LWCA_SAFE_JSON_DECREF(pJsonConfig);
    if (ppJsonConfig)
    {
        *ppJsonConfig = NULL;
    }

    goto cleanup;
}

DWORD
LwCAJsonLoadObjectFromString(
    PCSTR               pcszJsonStr,
    PLWCA_JSON_OBJECT   *ppJsonConfig
    )
{
    DWORD               dwError = 0;
    json_error_t        jsonError = {0};
    PLWCA_JSON_OBJECT   pJsonConfig = NULL;
    size_t              szJsonFlags = JSON_DECODE_ANY;

    if (IsNullOrEmptyString(pcszJsonStr) || !ppJsonConfig)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pJsonConfig = json_loads(pcszJsonStr, szJsonFlags, &jsonError);
    if (!pJsonConfig)
    {
        LWCA_LOG_ERROR(
            "[%s, %d] Failed to load json string (%s). Error: %s: %d",
            __FUNCTION__,
            __LINE__,
            pcszJsonStr,
            jsonError.text,
            jsonError.line
            );
        dwError = LWCA_JSON_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppJsonConfig = pJsonConfig;

cleanup:
    return dwError;

error:
    LWCA_SAFE_JSON_DECREF(pJsonConfig);
    if (ppJsonConfig)
    {
        *ppJsonConfig = NULL;
    }
    goto cleanup;
}

DWORD
LwCAJsonDumps(
    PLWCA_JSON_OBJECT   pJson,
    size_t              flags,
    PSTR                *ppszDest
    )
{
    DWORD   dwError = 0;
    PSTR    pszDest = NULL;

    if (!pJson || !ppszDest)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pszDest = json_dumps(pJson, flags);
    if (!pszDest)
    {
        dwError = LWCA_JSON_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppszDest = pszDest;

cleanup:
    return dwError;

error:
    if (ppszDest)
    {
        *ppszDest = NULL;
    }
    LWCA_SAFE_FREE_STRINGA(pszDest);
    goto cleanup;
}

/*
 * This function sets the json value (in ppJsonValue) corresponding to the given key of json object
 * bOptional is equivalent to searching for an optional key in the json object
 * If key does not exist and bOptional is set, this will return success with NULL value
 */
DWORD
LwCAJsonGetObjectFromKey(
    PLWCA_JSON_OBJECT       pJson,
    BOOLEAN                 bOptional,
    PCSTR                   pcszKey,
    PLWCA_JSON_OBJECT       *ppJsonValue
    )
{
    DWORD                   dwError = 0;
    PLWCA_JSON_OBJECT       pJsonValue = NULL;

    if (!pJson || IsNullOrEmptyString(pcszKey) || !ppJsonValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pJsonValue = json_object_get(pJson, pcszKey);
    if (!pJsonValue && !bOptional)
    {
        dwError = LWCA_JSON_PARSE_ERROR;
        BAIL_ON_JSON_PARSE_ERROR(dwError);
    }

    *ppJsonValue = pJsonValue;

cleanup:

    return dwError;

error:

    if (ppJsonValue)
    {
        *ppJsonValue = NULL;
    }

    goto cleanup;
}

/*
 * This function sets the string value (in ppcszValue) corresponding to the given the key of json object
 * bOptional is equivalent to searching for an optional key in the json object
 * If key does not exist and bOptional is set, this will return success with NULL value
 */
DWORD
LwCAJsonGetConstStringFromKey(
    PLWCA_JSON_OBJECT       pJson,
    BOOLEAN                 bOptional,
    PCSTR                   pcszKey,
    PCSTR                   *ppcszValue
    )
{
    DWORD                   dwError = 0;
    PLWCA_JSON_OBJECT       pJsonValue = NULL;
    PCSTR                   pcszValue = NULL;

    if (!pJson || IsNullOrEmptyString(pcszKey) || !ppcszValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, bOptional, pcszKey, &pJsonValue);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!pJsonValue && bOptional)
    {
        *ppcszValue = NULL;
        goto cleanup;
    }

    pcszValue = json_string_value(pJsonValue);
    if (IsNullOrEmptyString(pcszValue))
    {
        dwError = LWCA_JSON_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppcszValue = pcszValue;

cleanup:

    return dwError;

error:

    if (ppcszValue)
    {
        *ppcszValue = NULL;
    }

    goto cleanup;
}

/*
 * This function sets the string value (in ppszValue) corresponding to the given the key of json object
 * bOptional is equivalent to searching for an optional key in the json object
 * If key does not exist and bOptional is set, this will return success with NULL value
 */
DWORD
LwCAJsonGetStringFromKey(
    PLWCA_JSON_OBJECT       pJson,
    BOOLEAN                 bOptional,
    PCSTR                   pcszKey,
    PSTR                    *ppszValue
    )
{
    DWORD                   dwError = 0;
    PCSTR                   pcszValue = NULL;
    PSTR                    pszValue = NULL;

    if (!pJson || IsNullOrEmptyString(pcszKey) || !ppszValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetConstStringFromKey(pJson, bOptional, pcszKey, &pcszValue);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pcszValue, &pszValue);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszValue);
    if (ppszValue)
    {
        *ppszValue = NULL;
    }

    goto cleanup;
}

/*
 * This function sets the string array value (in ppszValue) corresponding to the given the key of json object
 * bOptional is equivalent to searching for an optional key in the json object
 * If key does not exist and bOptional is set, this will return success with NULL value
 */
DWORD
LwCAJsonGetStringArrayFromKey(
    PLWCA_JSON_OBJECT       pJson,
    BOOLEAN                 bOptional,
    PCSTR                   pcszKey,
    PLWCA_STRING_ARRAY      *ppStrArrValue
    )
{
    DWORD                   dwError = 0;
    DWORD                   dwIdx = 0;
    PLWCA_JSON_OBJECT       pJsonValue = NULL;
    PLWCA_JSON_OBJECT       pJsonArrayValue = NULL;
    PCSTR                   pcszArrayValue = NULL;
    DWORD                   dwArraySize = 0;
    PLWCA_STRING_ARRAY      pStrArray = NULL;

    if (!pJson || IsNullOrEmptyString(pcszKey) || !ppStrArrValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, bOptional, pcszKey, &pJsonValue);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!pJsonValue && bOptional)
    {
        *ppStrArrValue = NULL;
        goto cleanup;
    }

    dwArraySize = json_array_size(pJsonValue);
    if (!dwArraySize)
    {
        if (bOptional)
        {
            *ppStrArrValue = NULL;
            goto cleanup;
        }

        dwError = LWCA_JSON_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_STRING_ARRAY), (PVOID*)&pStrArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(PSTR) * dwArraySize, (PVOID*)&pStrArray->ppData);
    BAIL_ON_LWCA_ERROR(dwError);

    for (dwIdx = 0; dwIdx < dwArraySize; ++dwIdx)
    {
        dwError = LwCAJsonArrayGetBorrowedRef(pJsonValue, dwIdx, &pJsonArrayValue);
        BAIL_ON_LWCA_ERROR(dwError);

        pcszArrayValue = json_string_value(pJsonArrayValue);
        if (IsNullOrEmptyString(pcszArrayValue))
        {
            dwError = LWCA_JSON_PARSE_ERROR;
            BAIL_ON_LWCA_ERROR(dwError);
        }

        dwError = LwCAAllocateStringA(pcszArrayValue, &pStrArray->ppData[pStrArray->dwCount++]);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppStrArrValue = pStrArray;

cleanup:

    return dwError;

error:

    LwCAFreeStringArray(pStrArray);

    if (ppStrArrValue)
    {
        *ppStrArrValue = NULL;
    }

    goto cleanup;
}

/*
 * This function sets the time value (in ptValue) corresponding to the given the key of json object
 * The json value is expected to be a string in epoch time format
 * bOptional is equivalent to searching for an optional key in the json object
 * If key does not exist and bOptional is set, this will return success with NULL value
 */
DWORD
LwCAJsonGetTimeFromKey(
    PLWCA_JSON_OBJECT       pJson,
    BOOLEAN                 bOptional,
    PCSTR                   pcszKey,
    time_t                  *ptValue
    )
{
    DWORD                   dwError = 0;
    PLWCA_JSON_OBJECT       pJsonValue = NULL;
    PCSTR                   pcszValue = NULL;
    time_t                  time = 0;

    if (!pJson || IsNullOrEmptyString(pcszKey) || !ptValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, bOptional, pcszKey, &pJsonValue);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!pJsonValue && bOptional)
    {
        *ptValue = 0;
        goto cleanup;
    }

    pcszValue = json_string_value(pJsonValue);
    if (IsNullOrEmptyString(pcszValue))
    {
        dwError = LWCA_JSON_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    errno = 0;
    time = strtoul(pcszValue, NULL, 10);
    if (errno)
    {
        dwError = LWCA_JSON_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ptValue = time;

cleanup:

    return dwError;

error:

    if (ptValue)
    {
        *ptValue = 0;
    }

    goto cleanup;
}

/*
 * This function sets the int value (in piValue) corresponding to the given the key of json object
 * bOptional is equivalent to searching for an optional key in the json object
 * If key does not exist and bOptional is set, this will return success with NULL value
 */
DWORD
LwCAJsonGetIntegerFromKey(
    PLWCA_JSON_OBJECT       pJson,
    BOOLEAN                 bOptional,
    PCSTR                   pcszKey,
    int                     *piValue
    )
{
    DWORD                   dwError = 0;
    PLWCA_JSON_OBJECT       pJsonValue = NULL;
    int                     iValue = 0;

    if (!pJson || IsNullOrEmptyString(pcszKey) || !piValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, bOptional, pcszKey, &pJsonValue);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!pJsonValue && bOptional)
    {
        *piValue = 0;
        goto cleanup;
    }

    iValue = json_integer_value(pJsonValue);

    *piValue = iValue;

cleanup:

    return dwError;

error:

    if (piValue)
    {
        *piValue = 0;
    }

    goto cleanup;
}

/*
 * This function sets the uint32_t value (in pdwValue) corresponding to the given the key of json object
 * bOptional is equivalent to searching for an optional key in the json object
 * If key does not exist and bOptional is set, this will return success with NULL value
 */
DWORD
LwCAJsonGetUnsignedIntegerFromKey(
    PLWCA_JSON_OBJECT       pJson,
    BOOLEAN                 bOptional,
    PCSTR                   pcszKey,
    DWORD                   *pdwValue
    )
{
    DWORD                   dwError = 0;
    int                     iValue = 0;

    if (!pJson || IsNullOrEmptyString(pcszKey) || !pdwValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetIntegerFromKey(pJson, bOptional, pcszKey, &iValue);
    BAIL_ON_LWCA_ERROR(dwError);

    if (iValue < 0)
    {
        dwError = LWCA_JSON_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *pdwValue = (DWORD) iValue;

cleanup:

    return dwError;

error:

    if (pdwValue)
    {
        *pdwValue = 0;
    }

    goto cleanup;
}

/*
 * This function sets the boolean value (in pbValue) corresponding to the given the key of json object
 * bOptional is equivalent to searching for an optional key in the json object
 * If key does not exist and bOptional is set, this will return success with NULL value
 */
DWORD
LwCAJsonGetBooleanFromKey(
    PLWCA_JSON_OBJECT       pJson,
    BOOLEAN                 bOptional,
    PCSTR                   pcszKey,
    BOOLEAN                 *pbValue
    )
{
    DWORD                   dwError = 0;
    PLWCA_JSON_OBJECT       pJsonValue = NULL;
    BOOLEAN                 bValue = FALSE;

    if (!pJson || IsNullOrEmptyString(pcszKey) || !pbValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, bOptional, pcszKey, &pJsonValue);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!pJsonValue && bOptional)
    {
        *pbValue = FALSE;
        goto cleanup;
    }

    bValue = json_boolean_value(pJsonValue);

    *pbValue = bValue;

cleanup:

    return dwError;

error:

    if (pbValue)
    {
        *pbValue = FALSE;
    }

    goto cleanup;
}

BOOLEAN
LwCAJsonIsArray(
    PLWCA_JSON_OBJECT       pJson
    )
{
    return json_is_array(pJson);
}

SIZE_T
LwCAJsonArraySize(
    PLWCA_JSON_OBJECT       pJson
    )
{
    return json_array_size(pJson);
}

DWORD
LwCAJsonArrayGetBorrowedRef(
    PLWCA_JSON_OBJECT       pJsonIn,
    SIZE_T                  idx,
    PLWCA_JSON_OBJECT       *ppJsonOut // Borrowed reference, do not free
    )
{
    DWORD dwError = 0;

    if (!pJsonIn || !ppJsonOut)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!LwCAJsonIsArray(pJsonIn) || (idx > (LwCAJsonArraySize(pJsonIn) - 1)))
    {
        dwError = LWCA_JSON_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppJsonOut = json_array_get(pJsonIn, idx);

error:
    return dwError;
}

VOID
LwCAJsonCleanupObject(
    PLWCA_JSON_OBJECT       pJson
    )
{
    if (pJson)
    {
        json_decref(pJson);
    }
}

DWORD
LwCAJsonObjectCreate(
    PLWCA_JSON_OBJECT  *ppJson
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pJson = NULL;

    if (!ppJson)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }
    pJson = json_object();
    if (!pJson)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppJson = pJson;

cleanup:
    return dwError;

error:
    LWCA_SAFE_JSON_DECREF(pJson);
    if (ppJson)
    {
        *ppJson = NULL;
    }
    goto cleanup;
}

DWORD
LwCAJsonArrayCreate(
    PLWCA_JSON_OBJECT  *ppJson
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pJson = NULL;

    if (!ppJson)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }
    pJson = json_array();
    if (!pJson)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppJson = pJson;

cleanup:
    return dwError;

error:
    LWCA_SAFE_JSON_DECREF(pJson);
    if (ppJson)
    {
        *ppJson = NULL;
    }
    goto cleanup;
}

DWORD
LwCAJsonArrayStringCopy(
    PLWCA_JSON_OBJECT   pSrc,
    PLWCA_JSON_OBJECT   *ppDest
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pDest = NULL;
    PLWCA_JSON_OBJECT   pElem = NULL;
    SIZE_T              index = 0;

    dwError = LwCAJsonArrayCreate(&pDest);
    BAIL_ON_LWCA_ERROR(dwError);

    json_array_foreach(pSrc, index, pElem)
    {
        dwError = LwCAJsonAppendStringToArray(pDest, json_string_value(pElem));
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppDest = pDest;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAJsonSetStringToObject(
    PLWCA_JSON_OBJECT   pObj,
    PCSTR               pcszKey,
    PCSTR               pcszValue
    )
{
    DWORD               dwError = 0;
    int                 jsonError = 0;

    if (!pObj ||
        IsNullOrEmptyString(pcszKey) ||
        IsNullOrEmptyString(pcszValue)
        )
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    jsonError = json_object_set_new(pObj, pcszKey, json_string(pcszValue));
    if (jsonError)
    {
        dwError = LWCA_JSON_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAJsonSetJsonToObject(
    PLWCA_JSON_OBJECT       pObj,
    PCSTR                   pcszKey,
    PLWCA_JSON_OBJECT       pJson
    )
{
    DWORD   dwError = 0;
    int     jsonError = 0;

    if (!pJson || IsNullOrEmptyString(pcszKey) || !pObj)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    jsonError = json_object_set(pObj, pcszKey, pJson);
    if (jsonError)
    {
        dwError = LWCA_JSON_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAJsonAppendStringToArray(
    PLWCA_JSON_OBJECT   pArray,
    PCSTR               pcszValue
    )
{
    DWORD               dwError = 0;
    int                 jsonError = 0;

    if (!pArray || IsNullOrEmptyString(pcszValue))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    jsonError = json_array_append_new(pArray, json_string(pcszValue));
    if (jsonError)
    {
        dwError = LWCA_JSON_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAJsonAppendJsonToArray(
    PLWCA_JSON_OBJECT       pArray,
    PLWCA_JSON_OBJECT       pJson
    )
{
    DWORD   dwError = 0;
    int     jsonError = 0;

    if (!pJson || !pArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    jsonError = json_array_append(pArray, pJson);
    if (jsonError)
    {
        dwError = LWCA_JSON_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAJsonArrayGetStringAtIndex(
    PLWCA_JSON_OBJECT   pArray,
    int                 index,
    PSTR                *ppszValue
    )

{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pElem = NULL;
    PSTR                pszValue = NULL;
    PCSTR               pcszJsonStr = NULL;

    if (!pArray || !ppszValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonArrayGetBorrowedRef(pArray, index, &pElem);
    BAIL_ON_LWCA_ERROR(dwError);

    pcszJsonStr = json_string_value(pElem);
    if (!pcszJsonStr)
    {
        dwError = LWCA_JSON_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(pcszJsonStr, &pszValue);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszValue);
    if (ppszValue)
    {
        *ppszValue = NULL;
    }
    goto cleanup;
}

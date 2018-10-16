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

    LwCAJsonCleanupObject(pJsonConfig);
    if (ppJsonConfig)
    {
        *ppJsonConfig = NULL;
    }

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
    PLWCA_JSON_OBJECT       pJsonValue = NULL;
    PCSTR                   pcszValue = NULL;

    if (!pJson || IsNullOrEmptyString(pcszKey) || !ppszValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, bOptional, pcszKey, &pJsonValue);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!pJsonValue && bOptional)
    {
        dwError = 0;
        *ppszValue = NULL;
        goto cleanup;
    }

    pcszValue = json_string_value(pJsonValue);
    if (IsNullOrEmptyString(pcszValue))
    {
        dwError = LWCA_JSON_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(pcszValue, ppszValue);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:

    return dwError;

error:

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
        dwError = 0;
        *ppStrArrValue = NULL;
        goto cleanup;
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_STRING_ARRAY), (PVOID*)&pStrArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwArraySize = json_array_size(pJsonValue);
    if (!dwArraySize)
    {
        dwError = LWCA_JSON_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

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
        dwError = 0;
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
    time = strtoul(pcszValue, NULL, 0);
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
        dwError = 0;
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
        dwError = 0;
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

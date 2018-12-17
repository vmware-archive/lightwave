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
VmJsonResultLoadFromFile(
    PCSTR pszFile,
    PVM_JSON_RESULT pResult
    )
{
    DWORD dwError = 0;
    json_t *pJsonRoot = NULL;
    json_error_t jsonError = {0};

    if (IsNullOrEmptyString(pszFile) || !pResult)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    pJsonRoot = json_load_file(pszFile, 0, &jsonError);
    if (!pJsonRoot)
    {
        pResult->nJsonErrorLine = jsonError.line;

        dwError = VmAllocateStringPrintf(
                      &pResult->pszJsonErrorText,
                      jsonError.text);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        dwError = VM_COMMON_ERROR_JSON_LOAD_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    pResult->pJsonRoot = pJsonRoot;

cleanup:
    return dwError;

error:
    if (pJsonRoot)
    {
        json_decref(pJsonRoot);
    }
    goto cleanup;
}

static
DWORD
_VmJsonObjectPopulateSimpleCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    );

DWORD
VmJsonResultLoadString(
    PCSTR pszJson,
    PVM_JSON_RESULT pResult
    )
{
    DWORD dwError = 0;
    json_t *pJsonRoot = NULL;
    json_error_t jsonError = {0};

    pJsonRoot = json_loads(pszJson, 0, &jsonError);
    if (!pJsonRoot)
    {
        pResult->nJsonErrorLine = jsonError.line;

        dwError = VmAllocateStringPrintf(
                      &pResult->pszJsonErrorText,
                      jsonError.text);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        dwError = VM_COMMON_ERROR_JSON_LOAD_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    pResult->pJsonRoot = pJsonRoot;

cleanup:
    return dwError;

error:
    if (pJsonRoot)
    {
        json_decref(pJsonRoot);
    }
    goto cleanup;
}

DWORD
VmJsonResultInit(
    PVM_JSON_RESULT *ppResult
    )
{
    DWORD dwError = 0;
    PVM_JSON_RESULT pResult = NULL;

    if (!ppResult)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmAllocateMemory(sizeof(VM_JSON_RESULT), (PVOID *)&pResult);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    *ppResult = pResult;

cleanup:
    return dwError;

error:
    VmJsonResultFreeHandle(pResult);
    goto cleanup;
}

DWORD
VmJsonResultGetRootPosition(
    PVM_JSON_RESULT pResult,
    PVM_JSON_POSITION *ppPosition
    )
{
    DWORD dwError = 0;

    if (!pResult || !ppPosition)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    *ppPosition = pResult->pJsonRoot;

error:
    return dwError;
}

static
DWORD
_VmJsonTranslateType(
    int nJsonType,
    VM_JSON_RESULT_TYPE *pType
    )
{
    DWORD dwError = 0;
    VM_JSON_RESULT_TYPE nType = JSON_RESULT_INVALID;

    switch(nJsonType)
    {
        case JSON_OBJECT:  nType = JSON_RESULT_OBJECT; break;
        case JSON_ARRAY:   nType = JSON_RESULT_ARRAY; break;
        case JSON_STRING:  nType = JSON_RESULT_STRING; break;
        case JSON_INTEGER: nType = JSON_RESULT_INTEGER; break;
        case JSON_REAL:    nType = JSON_RESULT_REAL; break;
        case JSON_TRUE:    nType = JSON_RESULT_BOOLEAN; break;
        case JSON_FALSE:   nType = JSON_RESULT_BOOLEAN; break;
        case JSON_NULL:    nType = JSON_RESULT_NULL; break;
        default:           nType = JSON_RESULT_INVALID;
    }
    if (nType == JSON_RESULT_INVALID)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }
    *pType = nType;

error:
    return dwError;
}

static
DWORD
_VmJsonPopulateResultValue(
    PVM_JSON_POSITION pPosition,
    PVM_JSON_RESULT_VALUE pResultVal
    )
{
    DWORD dwError = 0;
    int nJsonType = json_typeof((json_t *)pPosition);

    dwError = _VmJsonTranslateType(nJsonType, &pResultVal->nType);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    switch(pResultVal->nType)
    {
        case JSON_RESULT_ARRAY:
            pResultVal->value.pArray = pPosition;
            break;
        case JSON_RESULT_OBJECT:
        case JSON_RESULT_NULL:
            pResultVal->value.pObject = pPosition;
            break;
        case JSON_RESULT_STRING:
            pResultVal->value.pszValue = json_string_value(pPosition);
            break;
        case JSON_RESULT_INTEGER:
            pResultVal->value.nValue = json_integer_value(pPosition);
            break;
        case JSON_RESULT_REAL:
            pResultVal->value.dValue = json_real_value(pPosition);
            break;
        case JSON_RESULT_BOOLEAN:
            pResultVal->value.bValue = json_boolean_value((json_t *)pPosition);
            break;
        default:
            dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
            BAIL_ON_VM_COMMON_ERROR(dwError);
    }

error:
    return dwError;
}

/*
 * note: result values used in callback are reused.
 * so do not save result values to build state.
 * implementations can build state in callbacks by
 * saving position handles.
*/
DWORD
VmJsonResultIterateObjectAt(
    PVM_JSON_POSITION pPosition,
    PVOID pUserData,
    PFN_JSON_RESULT_OBJECT_CB pfnCB
    )
{
    DWORD dwError = 0;
    PCSTR pszKey = NULL;
    json_t *pValue = NULL;
    VM_JSON_RESULT_VALUE resultVal = {0};

    if (!pPosition || !pfnCB)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    json_object_foreach(pPosition, pszKey, pValue)
    {
        dwError = _VmJsonPopulateResultValue(pValue, &resultVal);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        dwError = pfnCB(pUserData, pszKey, &resultVal);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmJsonResultIterateArrayAt(
    PVM_JSON_POSITION pPosition,
    PVOID pUserData,
    PFN_JSON_RESULT_ARRAY_CB pfnCB
    )
{
    DWORD dwError = 0;
    size_t nIndex = 0;
    size_t nSize = 0;
    json_t *pValue = NULL;

    if (!pPosition ||
        !pfnCB ||
        json_typeof((json_t *)pPosition) != JSON_ARRAY)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    nSize = json_array_size(pPosition);

    json_array_foreach(pPosition, nIndex, pValue)
    {
        dwError = pfnCB(pUserData, nSize, nIndex, pValue);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmJsonResultIterateAndGetValueFromArrayAt(
    PVM_JSON_POSITION pPosition,
    PVOID pUserData,
    PFN_JSON_RESULT_ARRAYVALUE_CB pfnCB
    )
{
    DWORD dwError = 0;
    size_t nIndex = 0;
    size_t nSize = 0;
    json_t *pValue = NULL;
    VM_JSON_RESULT_VALUE resultVal = {0};

    if (!pPosition ||
        !pfnCB ||
        json_typeof((json_t *)pPosition) != JSON_ARRAY)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    nSize = json_array_size(pPosition);

    json_array_foreach(pPosition, nIndex, pValue)
    {
        dwError = _VmJsonPopulateResultValue(pValue, &resultVal);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        dwError = pfnCB(pUserData, nSize, nIndex, &resultVal);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * Map simple result objects from json
 * pszJson is a valid json string.eg: {"greeting":"hello","recipient":"world"}
 * PSTR pszGreeting = NULL;
 * PSTR pszRecipient = NULL;
 * pMap is an array of struct entries of type VM_JSON_OBJECT_MAP. eg:
 * VM_JSON_OBJECT_MAP map[] =
 * {
 *     {"greeting",  JSON_RESULT_STRING, {&pszGreeting}},
 *     {"recipient", JSON_RESULT_STRING, {&pszRecipient}}
 * };
 *
*/
DWORD
VmJsonResultMapObject(
    PCSTR pszJson,
    PVM_JSON_OBJECT_MAP pMap
    )
{
    DWORD dwError = 0;
    PVM_JSON_RESULT pResult = NULL;
    PVM_JSON_POSITION pPositionRoot = NULL;

    if (IsNullOrEmptyString(pszJson) || !pMap)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmJsonResultInit(&pResult);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmJsonResultLoadString(pszJson, pResult);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmJsonResultGetRootPosition(pResult, &pPositionRoot);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmJsonResultIterateObjectAt(
                  pPositionRoot,
                  pMap,
                  _VmJsonObjectPopulateSimpleCB);
    BAIL_ON_VM_COMMON_ERROR(dwError);
error:
    VmJsonResultFreeHandle(pResult);
    return dwError;
}

static
DWORD
_VmJsonObjectPopulateSimpleCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    )
{
    DWORD dwError = 0;
    PVM_JSON_OBJECT_MAP pObjMap = NULL;
    PSTR pszValue = NULL;

    if (!pUserData || !pszKey || !pValue)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    for (pObjMap = (PVM_JSON_OBJECT_MAP)pUserData;
         pObjMap->pszName;
         ++pObjMap)
    {
        if (VmStringCompareA(pObjMap->pszName, pszKey, TRUE) != 0)
        {
            continue;
        }
        if (pValue->nType != pObjMap->type)
        {
            dwError = VM_COMMON_ERROR_JSON_MAP_BAD_TYPE;
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }
        switch(pObjMap->type)
        {
            case JSON_RESULT_BOOLEAN:
                *pObjMap->value.pbValue = pValue->value.bValue;
            break;
            case JSON_RESULT_STRING:
            {
                dwError = VmAllocateStringA(
                              pValue->value.pszValue,
                              &pszValue);
                BAIL_ON_VM_COMMON_ERROR(dwError);
                *pObjMap->value.ppszValue = pszValue;
            }
            break;
            case JSON_RESULT_INTEGER:
                *pObjMap->value.pnValue = pValue->value.nValue;
            break;
            case JSON_RESULT_REAL:
                *pObjMap->value.pdValue = pValue->value.dValue;
            break;
            case JSON_RESULT_OBJECT:
                dwError = VmJsonResultIterateObjectAt(
                              pValue->value.pObject,
                              pObjMap->value.pObjectValue,
                              _VmJsonObjectPopulateSimpleCB);
                BAIL_ON_VM_COMMON_ERROR(dwError);
            break;
            default:
            break;
        }
    }

cleanup:
    return dwError;

error:
    VmFreeMemory(pszValue);
    goto cleanup;
}

VOID
VmJsonResultFreeHandle(
    PVM_JSON_RESULT pResult
    )
{
    if (pResult)
    {
        if (pResult->pJsonRoot)
        {
            json_decref(pResult->pJsonRoot);
        }
        VM_COMMON_SAFE_FREE_MEMORY(pResult->pszJsonErrorText);
        VmFreeMemory(pResult);
    }
}

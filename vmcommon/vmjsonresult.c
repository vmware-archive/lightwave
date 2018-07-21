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
        case JSON_RESULT_OBJECT:
        case JSON_RESULT_ARRAY:
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

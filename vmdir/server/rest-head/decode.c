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
VmDirRESTDecodeEntry(
    json_t*         pjInput,
    PVDIR_ENTRY*    ppEntry
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0, j = 0;
    json_t* pjEntry = NULL;
    json_t* pjDN = NULL;
    json_t* pjAttrs = NULL;
    json_t* pjAttr = NULL;
    json_t* pjType = NULL;
    json_t* pjVals = NULL;
    json_t* pjVal = NULL;
    PCSTR   pszDN = NULL;
    PCSTR   pszType = NULL;
    PCSTR   pszVal = NULL;
    PVDIR_ENTRY     pEntry = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;

    if (!ppEntry)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pjEntry = pjInput;
    if (!pjEntry || !json_is_object(pjEntry))
    {
        dwError = VMDIR_ERROR_INVALID_REQUEST;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    pjDN = json_object_get(pjEntry, "dn");
    if (!pjDN || !json_is_string(pjDN))
    {
        dwError = VMDIR_ERROR_INVALID_REQUEST;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pszDN = json_string_value(pjDN);

    dwError = VmDirStringToBervalContent(pszDN, &pEntry->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pjAttrs = json_object_get(pjEntry, "attributes");
    if (!pjAttrs || !json_is_array(pjAttrs))
    {
        dwError = VMDIR_ERROR_INVALID_REQUEST;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i = 0; i < json_array_size(pjAttrs); i++)
    {
        dwError = VmDirAllocateMemory(sizeof(VDIR_ATTRIBUTE), (PVOID*)&pAttr);
        BAIL_ON_VMDIR_ERROR(dwError);

        pjAttr = json_array_get(pjAttrs, i);
        if (!pjAttr || !json_is_object(pjAttr))
        {
            dwError = VMDIR_ERROR_INVALID_REQUEST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pjType = json_object_get(pjAttr, "type");
        if (!pjType || !json_is_string(pjType))
        {
            dwError = VMDIR_ERROR_INVALID_REQUEST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        pszType = json_string_value(pjType);

        dwError = VmDirStringToBervalContent(pszType, &pAttr->type);
        BAIL_ON_VMDIR_ERROR(dwError);

        pjVals = json_object_get(pjAttr, "value");
        if (!pjVals || !json_is_array(pjVals))
        {
            dwError = VMDIR_ERROR_INVALID_REQUEST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        pAttr->numVals = (DWORD)json_array_size(pjVals);

        dwError = VmDirAllocateMemory(
                sizeof(VDIR_BERVALUE) * (pAttr->numVals + 1),
                (PVOID*)&pAttr->vals);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (j = 0; j < pAttr->numVals; j++)
        {
            pjVal = json_array_get(pjVals, j);
            if (!pjVal || !json_is_string(pjVal))
            {
                dwError = VMDIR_ERROR_INVALID_REQUEST;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            pszVal = json_string_value(pjVal);

            dwError = VmDirStringToBervalContent(pszVal, &pAttr->vals[j]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pAttr->next = pEntry->attrs;
        pEntry->attrs = pAttr;
        pAttr = NULL;
    }

    *ppEntry = pEntry;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeAttribute(pAttr);
    VmDirFreeEntry(pEntry);
    goto cleanup;
}

DWORD
VmDirRESTDecodeMods(
    json_t*             pjInput,
    PVDIR_MODIFICATION* ppMods,
    DWORD*              pdwNumMods
    )
{
    DWORD   dwError = 0;
    DWORD   dwNumMods = 0;
    DWORD   i = 0, j = 0;
    json_t* pjMods = NULL;
    json_t* pjMod = NULL;
    json_t* pjOp = NULL;
    json_t* pjAttr = NULL;
    json_t* pjType = NULL;
    json_t* pjVals = NULL;
    json_t* pjVal = NULL;
    PCSTR   pszOp = NULL;
    PCSTR   pszType = NULL;
    PCSTR   pszVal = NULL;
    PVDIR_MODIFICATION  pMod = NULL;
    PVDIR_MODIFICATION  pMods = NULL;

    if (!ppMods || !pdwNumMods)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pjMods = pjInput;
    if (!pjMods || !json_is_array(pjMods))
    {
        dwError = VMDIR_ERROR_INVALID_REQUEST;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    dwNumMods = (DWORD)json_array_size(pjMods);

    for (i = 0; i < dwNumMods; i++)
    {
        dwError = VmDirAllocateMemory(sizeof(VDIR_MODIFICATION), (PVOID*)&pMod);
        BAIL_ON_VMDIR_ERROR(dwError);

        pjMod = json_array_get(pjMods, i);
        if (!pjMod || !json_is_object(pjMod))
        {
            dwError = VMDIR_ERROR_INVALID_REQUEST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pjOp = json_object_get(pjMod, "operation");
        if (!pjOp || !json_is_string(pjOp))
        {
            dwError = VMDIR_ERROR_INVALID_REQUEST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        pszOp = json_string_value(pjOp);

        if (VmDirStringCompareA("add", pszOp, FALSE) == 0)
        {
            pMod->operation = MOD_OP_ADD;
        }
        else if (VmDirStringCompareA("delete", pszOp, FALSE) == 0)
        {
            pMod->operation = MOD_OP_DELETE;
        }
        else if (VmDirStringCompareA("replace", pszOp, FALSE) == 0)
        {
            pMod->operation = MOD_OP_REPLACE;
        }
        else
        {
            dwError = VMDIR_ERROR_INVALID_REQUEST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pjAttr = json_object_get(pjMod, "attribute");
        if (!pjAttr || !json_is_object(pjAttr))
        {
            dwError = VMDIR_ERROR_INVALID_REQUEST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pjType = json_object_get(pjAttr, "type");
        if (!pjType || !json_is_string(pjType))
        {
            dwError = VMDIR_ERROR_INVALID_REQUEST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        pszType = json_string_value(pjType);

        dwError = VmDirStringToBervalContent(pszType, &pMod->attr.type);
        BAIL_ON_VMDIR_ERROR(dwError);

        pjVals = json_object_get(pjAttr, "value");
        if (!pjVals || !json_is_array(pjVals))
        {
            dwError = VMDIR_ERROR_INVALID_REQUEST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        pMod->attr.numVals = (DWORD)json_array_size(pjVals);

        dwError = VmDirAllocateMemory(
                sizeof(VDIR_BERVALUE) * (pMod->attr.numVals + 1),
                (PVOID*)&pMod->attr.vals);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (j = 0; j < pMod->attr.numVals; j++)
        {
            pjVal = json_array_get(pjVals, j);
            if (!pjVal || !json_is_string(pjVal))
            {
                dwError = VMDIR_ERROR_INVALID_REQUEST;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            pszVal = json_string_value(pjVal);

            dwError = VmDirStringToBervalContent(pszVal, &pMod->attr.vals[j]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pMod->next = pMods;
        pMods = pMod;
        pMod = NULL;
    }

    *ppMods = pMods;
    *pdwNumMods = dwNumMods;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirModificationFree(pMod);
    for (pMod = pMods; pMod; )
    {
        PVDIR_MODIFICATION pNext = pMod->next;
        VmDirModificationFree(pMod);
        pMod = pNext;
    }
    goto cleanup;
}

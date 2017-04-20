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

static
DWORD
_VmDirRESTGetDNFromPayload(
    json_t*         pjEntry,
    PSTR*           ppszOutDN
    );

DWORD
VmDirRESTDecodeEntry(
    PVDIR_REST_OPERATION    pRestOp,
    PVDIR_ENTRY*            ppEntry
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0, j = 0;
    json_t* pjEntry = NULL;
    json_t* pjAttrs = NULL;
    json_t* pjAttr = NULL;
    json_t* pjType = NULL;
    json_t* pjVals = NULL;
    json_t* pjVal = NULL;
    PCSTR   pszType = NULL;
    PCSTR   pszVal = NULL;
    PSTR    pszLocalDN = NULL;
    PVDIR_ENTRY     pEntry = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;

    if (!ppEntry)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pjEntry = pRestOp->pjInput;
    if (!pjEntry || !json_is_object(pjEntry))
    {
        dwError = VMDIR_ERROR_INVALID_REQUEST;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    switch (pRestOp->pResource->rscType)
    {
        case VDIR_REST_RSC_LDAP:
                dwError = _VmDirRESTGetDNFromPayload(pjEntry, &pszLocalDN);
                BAIL_ON_VMDIR_ERROR(dwError);
                break;

        case VDIR_REST_RSC_OBJECT:
                dwError = VmDirRESTEndpointToDN(pRestOp, &pszLocalDN);
                BAIL_ON_VMDIR_ERROR(dwError);
                break;

        default:  BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_REQUEST);
    }

    dwError = VmDirStringToBervalContent(pszLocalDN, &pEntry->dn);
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
    VMDIR_SAFE_FREE_MEMORY(pszLocalDN);

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

static
DWORD
_VmDirRESTGetDNFromPayload(
    json_t*         pjEntry,
    PSTR*           ppszOutDN
    )
{
    DWORD   dwError=0;
    json_t* pjDN = NULL;
    PCSTR   pszDN = NULL;
    PSTR    pszLocalDN = NULL;
    PCSTR   pszFieldName = VMDIR_REST_DN_STR;

    pjDN = json_object_get(pjEntry, pszFieldName);
    if (!pjDN || !json_is_string(pjDN))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_REQUEST);
    }
    pszDN = json_string_value(pjDN);

    if (IsNullOrEmptyString(pszDN))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_REQUEST);
    }

    dwError = VmDirAllocateStringA(pszDN, &pszLocalDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszOutDN = pszLocalDN;
    pszLocalDN = NULL;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pszLocalDN);

    goto cleanup;
}

DWORD
VmDirRESTEndpointToDN(
    PVDIR_REST_OPERATION    pRestOp,
    PSTR*                   ppOutDN
    )
{
    DWORD   dwError = 0;
    PCSTR   pszObjectPath = NULL;
    PSTR    pszLocalDN = NULL;

    pszObjectPath = VmDirRESTGetRscEndpoint(pRestOp->pResource->rscType);
    assert(pszObjectPath);

    dwError = memcmp(pszObjectPath, pRestOp->pszEndpoint, strlen(pszObjectPath));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTObjPathToDN(pRestOp->pszEndpoint+strlen(pszObjectPath)+1, &pszLocalDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppOutDN = pszLocalDN;
    pszLocalDN = NULL;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszLocalDN);

    goto cleanup;
}

DWORD
VmDirRESTObjPathToDN(
    PCSTR   pszObjPath,
    PSTR*   ppszOutDN
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalDN = NULL;
    PVMDIR_STRING_LIST  pRDNList = NULL;
    size_t  dwDNLen = 0;
    size_t  dwIdx = 0;
    DWORD   dwCnt = 0;

    dwError = VmDirStringToTokenList(pszObjPath, VMDIR_URL_PATH_DELIMITER_STR, &pRDNList);
    BAIL_ON_VMDIR_ERROR(dwError);

    // all RDNs are "cn=XXX,"
    // So NumRDN * (4) + 1  system domain DN
    dwDNLen = VmDirStringLenA(pszObjPath) + (pRDNList->dwCount * 4) + 1 +
              (VmDirStringLenA(gVmdirServerGlobals.systemDomainDN.lberbv_val));

    dwError = VmDirAllocateMemory(dwDNLen, (PVOID*)&pszLocalDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = pRDNList->dwCount; dwCnt > 0; dwCnt--)
    {
        size_t dwRDNLen = 0;

        if (pRDNList->pStringList[dwCnt-1][0] == '\0')
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_REQUEST);
        }

        pszLocalDN[dwIdx++] = 'c';
        pszLocalDN[dwIdx++] = 'n';
        pszLocalDN[dwIdx++] = '=';

        dwRDNLen = VmDirStringLenA(pRDNList->pStringList[dwCnt-1]);
        dwError = VmDirCopyMemory( pszLocalDN+dwIdx, dwDNLen-dwIdx, pRDNList->pStringList[dwCnt-1], dwRDNLen);
        BAIL_ON_VMDIR_ERROR(dwError);
        dwIdx += dwRDNLen;

        pszLocalDN[dwIdx++] = ',';
    }

    dwError = VmDirCopyMemory(pszLocalDN+dwIdx, dwDNLen-dwIdx,
                    (VOID*)gVmdirServerGlobals.systemDomainDN.lberbv_val,
                    VmDirStringLenA(gVmdirServerGlobals.systemDomainDN.lberbv_val));
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszOutDN = pszLocalDN; pszLocalDN = NULL;

cleanup:
    VmDirStringListFree(pRDNList);

    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszLocalDN);

    goto cleanup;
}

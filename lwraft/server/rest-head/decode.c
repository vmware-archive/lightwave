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
VmDirRESTDecodeAttributeNoAlloc(
    json_t*         pjInput,
    PVDIR_ATTRIBUTE pAttr
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    json_t* pjAttr = NULL;
    json_t* pjType = NULL;
    json_t* pjVals = NULL;
    json_t* pjVal = NULL;
    PCSTR   pszType = NULL;
    PCSTR   pszVal = NULL;
    PSTR    pszDecoded = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    size_t  valLen = 0;
    int     len = 0;

    if (!pAttr)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pjAttr = pjInput;
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

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaAttrNameToDescriptor(
            pSchemaCtx, pszType, &pAttr->pATDesc);
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

    for (i = 0; i < pAttr->numVals; i++)
    {
        pjVal = json_array_get(pjVals, i);
        if (!pjVal || !json_is_string(pjVal))
        {
            dwError = VMDIR_ERROR_INVALID_REQUEST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        pszVal = json_string_value(pjVal);

        // check if value needs to be decoded
        if (VmDirSchemaAttrIsOctetString(pAttr->pATDesc))
        {
            VMDIR_SAFE_FREE_STRINGA(pszDecoded);

            valLen = VmDirStringLenA(pszVal);
            dwError = VmDirAllocateMemory(valLen + 1, (PVOID*)&pszDecoded);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = sasl_decode64(pszVal, valLen, pszDecoded, valLen, &len);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirStringToBervalContent(pszVal, &pAttr->vals[i]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDecoded);
    VmDirSchemaCtxRelease(pSchemaCtx);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VmDirRESTDecodeAttribute(
    json_t*             pjInput,
    PVDIR_ATTRIBUTE*    ppAttr
    )
{
    DWORD   dwError = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;

    if (!ppAttr)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_ATTRIBUTE), (PVOID*)&pAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTDecodeAttributeNoAlloc(pjInput, pAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppAttr = pAttr;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VmDirFreeAttribute(pAttr);
    goto cleanup;
}

DWORD
VmDirRESTDecodeEntry(
    json_t*         pjInput,
    PVDIR_ENTRY*    ppEntry
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    json_t* pjEntry = NULL;
    json_t* pjDN = NULL;
    json_t* pjAttrs = NULL;
    json_t* pjAttr = NULL;
    PCSTR   pszDN = NULL;
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
        pjAttr = json_array_get(pjAttrs, i);
        dwError = VmDirRESTDecodeAttribute(pjAttr, &pAttr);
        BAIL_ON_VMDIR_ERROR(dwError);

        pAttr->next = pEntry->attrs;
        pEntry->attrs = pAttr;
        pAttr = NULL;
    }

    *ppEntry = pEntry;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VmDirFreeAttribute(pAttr);
    VmDirFreeEntry(pEntry);
    goto cleanup;
}

DWORD
VmDirRESTDecodeEntryMods(
    json_t*             pjInput,
    PVDIR_MODIFICATION* ppMods,
    DWORD*              pdwNumMods
    )
{
    DWORD   dwError = 0;
    DWORD   dwNumMods = 0;
    DWORD   i = 0, j = 0;
    DWORD   dwTotalValues = 0;
    json_t* pjMods = NULL;
    json_t* pjMod = NULL;
    json_t* pjOp = NULL;
    json_t* pjAttr = NULL;
    PCSTR   pszOp = NULL;
    PCSTR   pszLogValue = NULL;
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

        dwError = VmDirRESTDecodeAttributeNoAlloc(pjAttr, &pMod->attr);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (j = 0; j < pMod->attr.numVals; j++)
        {
            pszLogValue = (0 == VmDirStringCompareA(VDIR_SAFE_STRING(pMod->attr.type.lberbv.bv_val), ATTR_USER_PASSWORD, FALSE)) ?
                "XXX" : VDIR_SAFE_STRING(pMod->attr.vals[j].lberbv.bv_val);

            // log the mod operation
            if (j < MAX_NUM_MOD_CONTENT_LOG) // cap the number of logs per attribute
            {
                VMDIR_LOG_INFO(
                        VMDIR_LOG_MASK_ALL,
                        "MOD %d,%s,%s: (%.*s)",
                        ++dwTotalValues,
                        pszOp,
                        VDIR_SAFE_STRING(pMod->attr.type.lberbv.bv_val),
                        VMDIR_MIN(pMod->attr.vals[j].lberbv.bv_len, VMDIR_MAX_LOG_OUTPUT_LEN), // cap the len of value being logged
                        pszLogValue);
            }
            else if (j == MAX_NUM_MOD_CONTENT_LOG)
            {
                VMDIR_LOG_INFO(
                        VMDIR_LOG_MASK_ALL,
                        "MOD %d,%s,%s: .... Total MOD %d",
                        ++dwTotalValues,
                        pszOp,
                        VDIR_SAFE_STRING(pMod->attr.type.lberbv.bv_val),
                        pMod->attr.numVals);
                break;
            }
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
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VmDirModificationFree(pMod);
    for (pMod = pMods; pMod; )
    {
        PVDIR_MODIFICATION pNext = pMod->next;
        VmDirModificationFree(pMod);
        pMod = pNext;
    }
    goto cleanup;
}

DWORD
VmDirRESTDecodeObjectPathToDN(
    PCSTR   pszObjPath,
    PCSTR   pszTenant,
    PSTR*   ppszDN
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0, j = 0;
    size_t  RDNLen = 0;
    size_t  localDNLen = 0;
    PCSTR   pszRDN = NULL;
    PSTR    pszLocalDN = NULL;
    PSTR    pszTenantDN = NULL;
    PSTR    pszDN = NULL;
    PVMDIR_STRING_LIST  pRDNList = NULL;

    if (!ppszDN ||
        (IsNullOrEmptyString(pszObjPath) &&
         IsNullOrEmptyString(pszTenant)))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszTenant))
    {
        dwError = VmDirDomainNameToDN(pszTenant, &pszTenantDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszObjPath))
    {
        dwError = VmDirStringToTokenList(
                pszObjPath, VMDIR_URL_PATH_DELIMITER_STR, &pRDNList);
        BAIL_ON_VMDIR_ERROR(dwError);

        localDNLen = VmDirStringLenA(pszObjPath) + (pRDNList->dwCount * 3) + 2;

        dwError = VmDirAllocateMemory(localDNLen, (PVOID*)&pszLocalDN);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (i = pRDNList->dwCount; i > 0; i--)
        {
            pszRDN = pRDNList->pStringList[i-1];
            RDNLen = VmDirStringLenA(pszRDN);

            dwError = VmDirCopyMemory(pszLocalDN+j, localDNLen-j, "cn=", 3);
            BAIL_ON_VMDIR_ERROR(dwError);
            j += 3;

            dwError = VmDirCopyMemory(pszLocalDN+j, localDNLen-j, pszRDN, RDNLen);
            BAIL_ON_VMDIR_ERROR(dwError);
            j += RDNLen;

            pszLocalDN[j++] = ',';
        }

        pszLocalDN[--j] = IsNullOrEmptyString(pszTenantDN) ? '\0' : ',';
    }

    dwError = VmDirAllocateStringPrintf(
            &pszDN,
            "%s%s",
            VDIR_SAFE_STRING(pszLocalDN),
            VDIR_SAFE_STRING(pszTenantDN));
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszDN = pszDN;

cleanup:
    VmDirStringListFree(pRDNList);
    VMDIR_SAFE_FREE_MEMORY(pszLocalDN);
    VMDIR_SAFE_FREE_MEMORY(pszTenantDN);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pszDN);
    goto cleanup;
}

DWORD
VmDirRESTDecodeObjectFilter(
    PVDIR_FILTER    pFilter,
    PCSTR           pszTenant
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0, j = 0;
    PSTR    pszFilType = NULL;
    PSTR    pszFilVal = NULL;
    PSTR    pszDecoded = NULL;
    PVDIR_FILTER    f = NULL;
    VDIR_BERVALUE   bvTmp = {0};

    if (IsNullOrEmptyString(pszTenant))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pFilter)
    {
        goto cleanup;
    }

    switch (pFilter->choice)
    {
    case LDAP_FILTER_AND:
    case LDAP_FILTER_OR:

        for (f = pFilter->filtComp.complex; f; f = f->next )
        {
            dwError = VmDirRESTDecodeObjectFilter(f, pszTenant);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        break;

    case LDAP_FILTER_NOT:

        dwError = VmDirRESTDecodeObjectFilter(
                pFilter->filtComp.complex, pszTenant);
        BAIL_ON_VMDIR_ERROR(dwError);
        break;

    case LDAP_FILTER_EQUALITY:
    case LDAP_FILTER_GE:
    case LDAP_FILTER_LE:

        pszFilType = pFilter->filtComp.ava.type.lberbv.bv_val;
        pszFilVal = pFilter->filtComp.ava.value.lberbv.bv_val;

        if (VmDirSchemaAttrIsDN(pFilter->filtComp.ava.pATDesc))
        {
            dwError = VmDirRESTDecodeObjectPathToDN(
                    pszFilVal, pszTenant, &pszDecoded);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringToBervalContent(
                    pszDecoded, &pFilter->filtComp.ava.value);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        break;

    case LDAP_FILTER_SUBSTRINGS:

        pszFilType = pFilter->filtComp.subStrings.type.lberbv.bv_val;

        /*
         * TODO Only final works currently, fix initial and any
         */
        if (VmDirSchemaAttrIsDN(pFilter->filtComp.subStrings.pATDesc))
        {
            // switch initial and final
            dwError = VmDirBervalContentDup(
                    &pFilter->filtComp.subStrings.initial, &bvTmp);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirBervalContentDup(
                    &pFilter->filtComp.subStrings.final,
                    &pFilter->filtComp.subStrings.initial);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirBervalContentDup(
                    &bvTmp, &pFilter->filtComp.subStrings.final);
            BAIL_ON_VMDIR_ERROR(dwError);

            // reverse any array
            for (i = 0; i < pFilter->filtComp.subStrings.anySize / 2; i++)
            {
                j = pFilter->filtComp.subStrings.anySize - i - 1;

                dwError = VmDirBervalContentDup(
                        &pFilter->filtComp.subStrings.any[i], &bvTmp);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirBervalContentDup(
                        &pFilter->filtComp.subStrings.any[j],
                        &pFilter->filtComp.subStrings.any[i]);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirBervalContentDup(
                        &bvTmp, &pFilter->filtComp.subStrings.any[j]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            // decode initial
            if (pFilter->filtComp.subStrings.initial.lberbv.bv_len)
            {
                pszFilVal = pFilter->filtComp.subStrings.initial.lberbv.bv_val;

                dwError = VmDirRESTDecodeObjectPathToDN(
                        pszFilVal, NULL, &pszDecoded);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirStringToBervalContent(
                        pszDecoded, &pFilter->filtComp.subStrings.initial);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            // decode final
            if (pFilter->filtComp.subStrings.final.lberbv.bv_len)
            {
                pszFilVal = pFilter->filtComp.subStrings.final.lberbv.bv_val;

                VMDIR_SAFE_FREE_STRINGA(pszDecoded);
                dwError = VmDirRESTDecodeObjectPathToDN(
                        pszFilVal, pszTenant, &pszDecoded);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirStringToBervalContent(
                        pszDecoded, &pFilter->filtComp.subStrings.final);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            // decode any array
            for (i = 0; i < pFilter->filtComp.subStrings.anySize; i++)
            {
                pszFilVal = pFilter->filtComp.subStrings.any[i].lberbv.bv_val;

                VMDIR_SAFE_FREE_STRINGA(pszDecoded);
                dwError = VmDirRESTDecodeObjectPathToDN(
                        pszFilVal, NULL, &pszDecoded);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirStringToBervalContent(
                        pszDecoded, &pFilter->filtComp.subStrings.any[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        break;

    default:
        break;
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDecoded);
    VmDirFreeBervalContent(&bvTmp);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VmDirRESTDecodeObject(
    json_t*         pjInput,
    PCSTR           pszObjPath,
    PCSTR           pszTenant,
    PVDIR_ENTRY*    ppObj
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0, j = 0;
    json_t* pjObj = NULL;
    json_t* pjAttrs = NULL;
    json_t* pjAttr = NULL;
    PSTR    pszDN = NULL;
    PSTR    pszCN = NULL;
    BOOLEAN bAddDefaultCN = TRUE;
    PVDIR_ENTRY     pObj = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;

    if (IsNullOrEmptyString(pszObjPath) || IsNullOrEmptyString(pszTenant) || !ppObj)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pjObj = pjInput;
    if (!pjObj || !json_is_object(pjObj))
    {
        dwError = VMDIR_ERROR_INVALID_REQUEST;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pObj);
    BAIL_ON_VMDIR_ERROR(dwError);

    pObj->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    dwError = VmDirRESTDecodeObjectPathToDN(pszObjPath, pszTenant, &pszDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirDnLastRDNToCn(pszDN, &pszCN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszDN, &pObj->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pjAttrs = json_object_get(pjObj, "attributes");
    if (!pjAttrs || !json_is_array(pjAttrs))
    {
        dwError = VMDIR_ERROR_INVALID_REQUEST;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i = 0; i < json_array_size(pjAttrs); i++)
    {
        pjAttr = json_array_get(pjAttrs, i);
        dwError = VmDirRESTDecodeAttribute(pjAttr, &pAttr);
        BAIL_ON_VMDIR_ERROR(dwError);

        // decode if attr syntax is DN
        if (VmDirSchemaAttrIsDN(pAttr->pATDesc))
        {
            for (j = 0; j < pAttr->numVals; j++)
            {
                VMDIR_SAFE_FREE_MEMORY(pszDN);

                dwError = VmDirRESTDecodeObjectPathToDN(
                        pAttr->vals[j].lberbv.bv_val, pszTenant, &pszDN);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirStringToBervalContent(pszDN, &pAttr->vals[j]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        // check if explicit CN list contains objectpath-derived CN
        else if (VmDirStringCompareA(
                pAttr->type.lberbv.bv_val, ATTR_CN, FALSE) == 0)
        {
            for (j = 0; j < pAttr->numVals; j++)
            {
                if (VmDirStringCompareA(
                        pAttr->vals[j].lberbv.bv_val, pszCN, FALSE) == 0)
                {
                    bAddDefaultCN = FALSE;
                    break;
                }
            }
        }

        pAttr->next = pObj->attrs;
        pObj->attrs = pAttr;
        pAttr = NULL;
    }

    if (bAddDefaultCN)
    {
        dwError = VmDirSchemaCtxAcquire(&pObj->pSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirEntryAddSingleValueStrAttribute(pObj, ATTR_CN, pszCN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppObj = pObj;

cleanup:
    if (pObj)
    {
        VmDirSchemaCtxRelease(pObj->pSchemaCtx);
        pObj->pSchemaCtx = NULL;
    }
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    VMDIR_SAFE_FREE_MEMORY(pszCN);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VmDirFreeAttribute(pAttr);
    VmDirFreeEntry(pObj);
    goto cleanup;
}

DWORD
VmDirRESTDecodeObjectMods(
    json_t*             pjInput,
    PCSTR               pszTenant,
    PVDIR_MODIFICATION* ppMods,
    DWORD*              pdwNumMods
    )
{
    DWORD   dwError = 0;
    DWORD   dwNumMods = 0;
    DWORD   i = 0;
    PSTR    pszDN = NULL;
    PVDIR_MODIFICATION  pMods = NULL;
    PVDIR_MODIFICATION  pMod = NULL;

    if (IsNullOrEmptyString(pszTenant) || !ppMods || !pdwNumMods)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirRESTDecodeEntryMods(pjInput, &pMods, &dwNumMods);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pMod = pMods; pMod; )
    {
        PVDIR_MODIFICATION pNext = pMod->next;
        PVDIR_ATTRIBUTE pAttr = &pMod->attr;

        // decode if attr syntax is DN
        if (VmDirSchemaAttrIsDN(pAttr->pATDesc))
        {
            for (i = 0; i < pAttr->numVals; i++)
            {
                VMDIR_SAFE_FREE_MEMORY(pszDN);

                dwError = VmDirRESTDecodeObjectPathToDN(
                        pAttr->vals[i].lberbv.bv_val, pszTenant, &pszDN);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirStringToBervalContent(pszDN, &pAttr->vals[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        pMod = pNext;
    }

    *ppMods = pMods;
    *pdwNumMods = dwNumMods;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    for (pMod = pMods; pMod; )
    {
        PVDIR_MODIFICATION pNext = pMod->next;
        VmDirModificationFree(pMod);
        pMod = pNext;
    }
    goto cleanup;
}

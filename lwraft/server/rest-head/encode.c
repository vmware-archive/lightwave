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
VmDirRESTEncodeAttribute(
    PVDIR_ATTRIBUTE pAttr,
    json_t**        ppjOutput
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    json_t* pjVals = NULL;
    json_t* pjAttr = NULL;
    PSTR    pszEncodedVal = NULL;
    int     len = 0;

    if (!pAttr || !ppjOutput)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pjAttr = json_object();
    pjVals = json_array();

    dwError = json_object_set_new(
            pjAttr, "type", json_string(pAttr->type.lberbv.bv_val));
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pAttr->numVals; i++)
    {
        // check if value needs to be encoded
        if (VmDirSchemaAttrIsOctetString(pAttr->pATDesc))
        {
            VMDIR_SAFE_FREE_STRINGA(pszEncodedVal);

            dwError = VmDirAllocateMemory(
                    pAttr->vals[i].lberbv.bv_len * 2 + 1,
                    (PVOID*)&pszEncodedVal);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = sasl_encode64(
                    pAttr->vals[i].lberbv.bv_val,
                    pAttr->vals[i].lberbv.bv_len,
                    pszEncodedVal,
                    pAttr->vals[i].lberbv.bv_len * 2 + 1,
                    &len);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = json_array_append_new(
                    pjVals, json_string(pszEncodedVal));
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = json_array_append_new(
                    pjVals, json_string(pAttr->vals[i].lberbv.bv_val));
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = json_object_set_new(pjAttr, "value", pjVals);
    BAIL_ON_VMDIR_ERROR(dwError);
    pjVals = NULL;

    *ppjOutput = pjAttr;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszEncodedVal);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    if (pjVals)
    {
        json_decref(pjVals);
    }
    if (pjAttr)
    {
        json_decref(pjAttr);
    }
    goto cleanup;
}

DWORD
VmDirRESTDNToObjectpath(
    PCSTR                   pszDN,
    PSTR*                   ppOutObjectPath
    )
{
    DWORD   dwError = 0;
    PSTR    pszObjectPath = NULL;
    PSTR    pszLocalDN = NULL;
    PVMDIR_STRING_LIST  pRDNList = NULL;
    DWORD   dwCnt = 0;
    DWORD   dwLen = 0;
    DWORD   dwIdx = 0;

int iSystemDomainRDNCnt = 2;  // TODO

    dwError = VmDirDNToRDNList(pszDN, 1, &pRDNList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt+iSystemDomainRDNCnt < pRDNList->dwCount; dwCnt++)
    {
        dwLen += VmDirStringLenA(pRDNList->pStringList[dwCnt]);
    }
    dwLen += pRDNList->dwCount;

    dwError = VmDirAllocateMemory(dwLen, (PVOID*)&pszObjectPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = pRDNList->dwCount - iSystemDomainRDNCnt, dwIdx = 0;
         dwCnt > 0;
         dwCnt--)
    {
        DWORD dwRDNLen = VmDirStringLenA(pRDNList->pStringList[dwCnt-1]);

        pszObjectPath[dwIdx++] = '/';
        dwError = VmDirCopyMemory( pszObjectPath+dwIdx, dwLen-dwIdx, pRDNList->pStringList[dwCnt-1], dwRDNLen);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwIdx += dwRDNLen;
    }

    *ppOutObjectPath = pszObjectPath;
    pszObjectPath = NULL;

cleanup:
    VmDirStringListFree(pRDNList);

    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszLocalDN);

    goto cleanup;
}

DWORD
VmDirRESTEncodeEntry(
    PVDIR_REST_OPERATION    pRestOp,
    PVDIR_ENTRY             pEntry,
    PVDIR_BERVALUE          pbvAttrs,
    json_t**                ppjOutput
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0, j = 0;
    BOOLEAN bReturn = FALSE;
    PSTR    pszObjctPath = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;
    PVDIR_ATTRIBUTE pAttrs[3] = {0};
    json_t*         pjAttr = NULL;
    json_t*         pjAttrs = NULL;
    json_t*         pjEntry = NULL;

    if (!pEntry || !ppjOutput)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pjEntry = json_object();
    pjAttrs = json_array();

    switch (pRestOp->pResource->rscType)
    {
        case VDIR_REST_RSC_LDAP:
                dwError = json_object_set_new(
                            pjEntry, VMDIR_REST_DN_STR, json_string(pEntry->dn.lberbv.bv_val));
                BAIL_ON_VMDIR_ERROR(dwError);
                break;

        case VDIR_REST_RSC_OBJECT:
                dwError = VmDirRESTDNToObjectpath(pEntry->dn.lberbv.bv_val, &pszObjctPath);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = json_object_set_new(
                                            pjEntry, VMDIR_REST_OBJECTPATH_STR, json_string(pszObjctPath));
                BAIL_ON_VMDIR_ERROR(dwError);
                break;

        default: BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_REQUEST);
    }

    // TODO special char?
    pAttrs[0] = pEntry->attrs;
    pAttrs[1] = pEntry->pComputedAttrs;

    for (i = 0; pAttrs[i]; i++)
    {
        for (pAttr = pAttrs[i]; pAttr; pAttr = pAttr->next)
        {
            bReturn = FALSE;

            for (j = 0; pbvAttrs && pbvAttrs[j].lberbv.bv_val; j++)
            {
                if (VmDirStringCompareA(
                        pAttr->type.lberbv.bv_val,
                        pbvAttrs[j].lberbv.bv_val,
                        FALSE) == 0)
                {
                    bReturn = TRUE;
                    break;
                }
            }

            if ( bReturn ||
                 (!pbvAttrs && pAttr->pATDesc->usage == VDIR_LDAP_USER_APPLICATIONS_ATTRIBUTE)
               )
            {
                dwError = VmDirRESTEncodeAttribute(pAttr, &pjAttr);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = json_array_append_new(pjAttrs, pjAttr);
                BAIL_ON_VMDIR_ERROR(dwError);
                pjAttr = NULL;
            }
        }
    }

    dwError = json_object_set_new(pjEntry, "attributes", pjAttrs);
    BAIL_ON_VMDIR_ERROR(dwError);
    pjAttrs = NULL;

    *ppjOutput = pjEntry;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszObjctPath);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    if (pjAttr)
    {
        json_decref(pjAttr);
    }
    if (pjAttrs)
    {
        json_decref(pjAttrs);
    }
    if (pjEntry)
    {
        json_decref(pjEntry);
    }
    goto cleanup;
}

DWORD
VmDirRESTEncodeEntryArray(
    PVDIR_REST_OPERATION    pRestOp,
    PVDIR_ENTRY_ARRAY       pEntryArray,
    PVDIR_BERVALUE          pbvAttrs,
    json_t**                ppjOutput
    )
{
    DWORD   dwError = 0;
    size_t  i = 0;
    PVDIR_ENTRY pEntry = NULL;
    json_t*     pjEntry = NULL;
    json_t*     pjEntryArray = NULL;

    if (!pEntryArray || !ppjOutput)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pjEntryArray = json_array();

    for (i = 0; i < pEntryArray->iSize; i++)
    {
        pEntry = &pEntryArray->pEntry[i];

        dwError = VmDirRESTEncodeEntry(pRestOp, pEntry, pbvAttrs, &pjEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = json_array_append_new(pjEntryArray, pjEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
        pjEntry = NULL;
    }

    *ppjOutput = pjEntryArray;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    if (pjEntry)
    {
        json_decref(pjEntry);
    }
    if (pjEntryArray)
    {
        json_decref(pjEntryArray);
    }
    goto cleanup;
}

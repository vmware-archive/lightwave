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
VmDirRESTEncodeEntry(
    PVDIR_ENTRY     pEntry,
    PVDIR_BERVALUE  pbvAttrs,
    json_t**        ppjOutput
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0, j = 0;
    BOOLEAN bReturn = FALSE;
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

    dwError = json_object_set_new(
            pjEntry, "dn", json_string(pEntry->dn.lberbv.bv_val));
    BAIL_ON_VMDIR_ERROR(dwError);

    // TODO special char?
    pAttrs[0] = pEntry->attrs;
    pAttrs[1] = pEntry->pComputedAttrs;

    for (i = 0; pAttrs[i]; i++)
    {
        for (pAttr = pAttrs[i]; pAttr; pAttr = pAttr->next)
        {
            bReturn = pbvAttrs == NULL;

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

            if (bReturn)
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
    PVDIR_ENTRY_ARRAY   pEntryArray,
    PVDIR_BERVALUE      pbvAttrs,
    json_t**            ppjOutput
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

        dwError = VmDirRESTEncodeEntry(pEntry, pbvAttrs, &pjEntry);
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

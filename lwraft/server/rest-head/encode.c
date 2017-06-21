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
    PSTR    pszEncoded = NULL;
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
            VMDIR_SAFE_FREE_STRINGA(pszEncoded);

            dwError = VmDirAllocateMemory(
                    pAttr->vals[i].lberbv.bv_len * 2 + 1,
                    (PVOID*)&pszEncoded);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = sasl_encode64(
                    pAttr->vals[i].lberbv.bv_val,
                    pAttr->vals[i].lberbv.bv_len,
                    pszEncoded,
                    pAttr->vals[i].lberbv.bv_len * 2 + 1,
                    &len);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = json_array_append_new(
                    pjVals, json_string(pszEncoded));
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
    VMDIR_SAFE_FREE_STRINGA(pszEncoded);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

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
    BOOLEAN bAsterisk = FALSE;
    BOOLEAN bPlusSign = FALSE;
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

    for (i = 0; pbvAttrs && pbvAttrs[i].lberbv.bv_val; i++)
    {
        if (VmDirStringCompareA("*", pbvAttrs[i].lberbv.bv_val, TRUE) == 0)
        {
            bAsterisk = TRUE;
        }
        else if (VmDirStringCompareA("+", pbvAttrs[i].lberbv.bv_val, TRUE) == 0)
        {
            bPlusSign = TRUE;
        }
    }

    pAttrs[0] = pEntry->attrs;
    pAttrs[1] = pEntry->pComputedAttrs;

    for (i = 0; pAttrs[i]; i++)
    {
        for (pAttr = pAttrs[i]; pAttr; pAttr = pAttr->next)
        {
            bReturn = FALSE;

            if ((bAsterisk || !pbvAttrs) &&
                    pAttr->pATDesc->usage ==
                            VDIR_LDAP_USER_APPLICATIONS_ATTRIBUTE)
            {
                bReturn = TRUE;
            }
            else if (bPlusSign &&
                    pAttr->pATDesc->usage ==
                            VDIR_LDAP_DIRECTORY_OPERATION_ATTRIBUTE)
            {
                bReturn = TRUE;
            }
            else if (pbvAttrs)
            {
                for (j = 0; pbvAttrs[j].lberbv.bv_val; j++)
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
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

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
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

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

DWORD
VmDirRESTEncodeDNToObjectPath(
    PCSTR   pszDN,
    PCSTR   pszTenant,
    PSTR*   ppszObjPath
    )
{
    DWORD   dwError = 0;
    DWORD   dwLocalRDNs = 0;
    DWORD   i = 0, j = 0;
    PSTR    pszTenantDN = NULL;
    PSTR    pszObjPath = NULL;
    PCSTR   pszRDN = NULL;
    size_t  objPathLen = 0;
    size_t  RDNLen = 0;
    PVMDIR_STRING_LIST  pRDNList = NULL;
    PVMDIR_STRING_LIST  pTenantRDNList = NULL;

    if (IsNullOrEmptyString(pszDN) || IsNullOrEmptyString(pszTenant) || !ppszObjPath)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSrvCreateDomainDN(pszTenant, &pszTenantDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // DN must be under the target tenant
    if (!VmDirStringEndsWith(pszDN, pszTenantDN, FALSE))
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirDNToRDNList(pszDN, 0, &pRDNList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirDNToRDNList(pszTenantDN, 0, &pTenantRDNList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwLocalRDNs = pRDNList->dwCount - pTenantRDNList->dwCount;

    objPathLen = VmDirStringLenA(pszDN)
               - VmDirStringLenA(pszTenantDN)
               - (dwLocalRDNs * 3) + 2;

    dwError = VmDirAllocateMemory(objPathLen, (PVOID*)&pszObjPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszObjPath[0] = '/';
    for (i = dwLocalRDNs; i > 0; i--)
    {
        pszRDN = pRDNList->pStringList[i-1];

        // - each RDN must be cn
        // - each RDN must not contain '/'
        if (!VmDirStringStartsWith(pszRDN, "cn=", FALSE) ||
            VmDirStringChrA(pszRDN, '/'))
        {
            dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pszRDN += 3;
        RDNLen = VmDirStringLenA(pszRDN);

        pszObjPath[j++] = '/';

        dwError = VmDirCopyMemory(pszObjPath+j, objPathLen-j, pszRDN, RDNLen);
        BAIL_ON_VMDIR_ERROR(dwError);
        j += RDNLen;
    }

    *ppszObjPath = pszObjPath;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszTenantDN);
    VmDirStringListFree(pTenantRDNList);
    VmDirStringListFree(pRDNList);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pszObjPath);
    goto cleanup;
}

DWORD
VmDirRESTEncodeObjectAttribute(
    PVDIR_ATTRIBUTE pObjAttr,
    PCSTR           pszTenant,
    json_t**        ppjOutput
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszObjPath = NULL;
    json_t* pjObjAttr = NULL;

    if (!pObjAttr || IsNullOrEmptyString(pszTenant) || !ppjOutput)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirSchemaAttrIsDN(pObjAttr->pATDesc))
    {
        for (i = 0; i < pObjAttr->numVals; i++)
        {
            VMDIR_SAFE_FREE_STRINGA(pszObjPath);

            dwError = VmDirRESTEncodeDNToObjectPath(
                    pObjAttr->vals[i].lberbv.bv_val, pszTenant, &pszObjPath);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringToBervalContent(
                    pszObjPath, &pObjAttr->vals[i]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmDirRESTEncodeAttribute(pObjAttr, &pjObjAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppjOutput = pjObjAttr;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszObjPath);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    if (pjObjAttr)
    {
        json_decref(pjObjAttr);
    }
    goto cleanup;
}

DWORD
VmDirRESTEncodeObject(
    PVDIR_ENTRY     pObj,
    PVDIR_BERVALUE  pbvAttrs,
    PCSTR           pszTenant,
    json_t**        ppjOutput
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0, j = 0;
    BOOLEAN bReturn = FALSE;
    BOOLEAN bAsterisk = FALSE;
    BOOLEAN bPlusSign = FALSE;
    PSTR    pszObjPath = NULL;
    PVDIR_ATTRIBUTE pObjAttr = NULL;
    PVDIR_ATTRIBUTE pObjAttrs[3] = {0};
    json_t*         pjObjAttr = NULL;
    json_t*         pjObjAttrs = NULL;
    json_t*         pjObj = NULL;

    if (!pObj || IsNullOrEmptyString(pszTenant) || !ppjOutput)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pjObj = json_object();
    pjObjAttrs = json_array();

    dwError = VmDirRESTEncodeDNToObjectPath(
            pObj->dn.lberbv.bv_val, pszTenant, &pszObjPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = json_object_set_new(
            pjObj, "objectpath", json_string(pszObjPath));
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; pbvAttrs && pbvAttrs[i].lberbv.bv_val; i++)
    {
        if (VmDirStringCompareA("*", pbvAttrs[i].lberbv.bv_val, TRUE) == 0)
        {
            bAsterisk = TRUE;
        }
        else if (VmDirStringCompareA("+", pbvAttrs[i].lberbv.bv_val, TRUE) == 0)
        {
            bPlusSign = TRUE;
        }
    }

    pObjAttrs[0] = pObj->attrs;
    pObjAttrs[1] = pObj->pComputedAttrs;

    for (i = 0; pObjAttrs[i]; i++)
    {
        for (pObjAttr = pObjAttrs[i]; pObjAttr; pObjAttr = pObjAttr->next)
        {
            bReturn = FALSE;

            if ((bAsterisk || !pbvAttrs) &&
                    pObjAttr->pATDesc->usage ==
                            VDIR_LDAP_USER_APPLICATIONS_ATTRIBUTE)
            {
                bReturn = TRUE;
            }
            else if (bPlusSign &&
                    pObjAttr->pATDesc->usage ==
                            VDIR_LDAP_DIRECTORY_OPERATION_ATTRIBUTE)
            {
                bReturn = TRUE;
            }
            else if (pbvAttrs)
            {
                for (j = 0; pbvAttrs[j].lberbv.bv_val; j++)
                {
                    if (VmDirStringCompareA(
                            pObjAttr->type.lberbv.bv_val,
                            pbvAttrs[j].lberbv.bv_val,
                            FALSE) == 0)
                    {
                        bReturn = TRUE;
                        break;
                    }
                }
            }

            if (bReturn)
            {
                dwError = VmDirRESTEncodeObjectAttribute(
                        pObjAttr, pszTenant, &pjObjAttr);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = json_array_append_new(pjObjAttrs, pjObjAttr);
                BAIL_ON_VMDIR_ERROR(dwError);
                pjObjAttr = NULL;
            }
        }
    }

    dwError = json_object_set_new(pjObj, "attributes", pjObjAttrs);
    BAIL_ON_VMDIR_ERROR(dwError);
    pjObjAttrs = NULL;

    *ppjOutput = pjObj;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszObjPath);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    if (pjObjAttr)
    {
        json_decref(pjObjAttr);
    }
    if (pjObjAttrs)
    {
        json_decref(pjObjAttrs);
    }
    if (pjObj)
    {
        json_decref(pjObj);
    }
    goto cleanup;
}

DWORD
VmDirRESTEncodeObjectArray(
    PVDIR_ENTRY_ARRAY   pObjArray,
    PVDIR_BERVALUE      pbvAttrs,
    PCSTR               pszTenant,
    json_t**            ppjOutput,
    size_t*             pSkipped
    )
{
    DWORD   dwError = 0;
    size_t  skipped = 0;
    size_t  i = 0;
    PVDIR_ENTRY pObj = NULL;
    json_t*     pjObj = NULL;
    json_t*     pjObjArray = NULL;

    if (!pObjArray || IsNullOrEmptyString(pszTenant) || !ppjOutput)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pjObjArray = json_array();

    for (i = 0; i < pObjArray->iSize; i++)
    {
        pObj = &pObjArray->pEntry[i];

        dwError = VmDirRESTEncodeObject(pObj, pbvAttrs, pszTenant, &pjObj);
        if (dwError == VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION)
        {
            // skip objects that violate constraint
            dwError = 0;
            skipped++;
            continue;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = json_array_append_new(pjObjArray, pjObj);
        BAIL_ON_VMDIR_ERROR(dwError);
        pjObj = NULL;
    }

    if (pSkipped)
    {
        *pSkipped = skipped;
    }

    *ppjOutput = pjObjArray;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    if (pjObj)
    {
        json_decref(pjObj);
    }
    if (pjObjArray)
    {
        json_decref(pjObjArray);
    }
    goto cleanup;
}

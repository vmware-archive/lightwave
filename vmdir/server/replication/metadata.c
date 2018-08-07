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

static
DWORD
_VmDirReplPopulateAttrNewMetaData(
    PVDIR_ENTRY    pEntry,
    USN            localUsn,
    PLW_HASHMAP    pMetaDataMap
    );
/*
 * 1) Find the attribute that holds attribute meta data.
 * 2) Attributes for usnCreated/usnChanged are updated with current local USN
 * 3) If we are doing a modify/delete, attribute meta data is checked to see supplier/consumer wins.
 *        - If supplier attribute won, update its meta data with current local USN.
 *        - If consumer wins don't write corresponding attribute.
 *        - Special case: supplier lost for UsnChanged, replace the metaData with consumer's metaData.
 * 4) If no attribute metaData exists, create it.
 */
DWORD
VmDirReplSetAttrNewMetaData(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PLW_HASHMAP*        ppMetaDataMap
    )
{
    DWORD               dwError = LDAP_SUCCESS;
    PVDIR_ATTRIBUTE     pCurrAttr = NULL;
    PVDIR_ATTRIBUTE     pPrevAttr = NULL;
    PVDIR_ATTRIBUTE     pAttrAttrMetaData = NULL;
    PLW_HASHMAP         pMetaDataMap = NULL;

    if (!pOperation || !pEntry || !ppMetaDataMap)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    for (pPrevAttr = NULL, pCurrAttr = pEntry->attrs;
         pCurrAttr;
         pPrevAttr = pCurrAttr, pCurrAttr = pCurrAttr->next)
    {
        if (VmDirStringCompareA(pCurrAttr->type.lberbv.bv_val, ATTR_ATTR_META_DATA, FALSE) == 0)
        {
            if (pPrevAttr == NULL)
            {
                pEntry->attrs = pCurrAttr->next;
            }
            else
            {
                pPrevAttr->next = pCurrAttr->next;
            }

            pAttrAttrMetaData = pCurrAttr;

            dwError = VmDirAttributeMetaDataToHashMap(pAttrAttrMetaData, &pMetaDataMap);
            BAIL_ON_VMDIR_ERROR(dwError);

            *ppMetaDataMap = pMetaDataMap;
            continue;
        }

        if (VmDirStringCompareA(pCurrAttr->type.lberbv.bv_val, ATTR_USN_CREATED, FALSE) == 0 ||
            VmDirStringCompareA(pCurrAttr->type.lberbv.bv_val, ATTR_USN_CHANGED, FALSE) == 0)
        {
            char      pszLocalUsn[VMDIR_MAX_USN_STR_LEN] = {'\0'};
            size_t    localUsnStrlen = 0;

            dwError = VmDirStringNPrintFA(
                    pszLocalUsn,
                    sizeof(pszLocalUsn),
                    sizeof(pszLocalUsn) - 1,
                    "%"PRId64,
                    pOperation->pWriteQueueEle->usn);
            BAIL_ON_VMDIR_ERROR(dwError);

            localUsnStrlen = VmDirStringLenA(pszLocalUsn);

            VmDirFreeBervalContent(&pCurrAttr->vals[0]);

            dwError = VmDirAllocateAndCopyMemory(pszLocalUsn, localUsnStrlen, (PVOID*)&pCurrAttr->vals[0].lberbv.bv_val);
            BAIL_ON_VMDIR_ERROR(dwError);

            pCurrAttr->vals[0].lberbv.bv_len = localUsnStrlen;
            pCurrAttr->vals[0].bOwnBvVal = TRUE;
            continue;
        }
    }

    if (pAttrAttrMetaData == NULL)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: attrMetaData attribute not present in Entry: %s",
                __FUNCTION__,
                pEntry->dn.lberbv.bv_val);
        BAIL_WITH_VMDIR_ERROR(dwError, LDAP_OPERATIONS_ERROR);
    }

    if (pOperation->reqCode == LDAP_REQ_MODIFY)
    {
        dwError = VmDirReplResolveConflicts(
                pOperation,
                pEntry,
                pMetaDataMap);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirReplPopulateAttrNewMetaData(
            pEntry,
            pOperation->pWriteQueueEle->usn,
            pMetaDataMap);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeAttribute(pAttrAttrMetaData);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirReplPopulateAttrNewMetaData(
    PVDIR_ENTRY    pEntry,
    USN            localUsn,
    PLW_HASHMAP    pMetaDataMap
    )
{
    PSTR                        pszAttrType = NULL;
    DWORD                       dwError = 0;
    PVDIR_ATTRIBUTE             pCurrAttr = NULL;
    LW_HASHMAP_PAIR             pair = {NULL, NULL};
    PVMDIR_ATTRIBUTE_METADATA   pSupplierMetaData = NULL;

    if (!pEntry || !pMetaDataMap)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    for (pCurrAttr = pEntry->attrs; pCurrAttr; pCurrAttr = pCurrAttr->next)
    {
        dwError = LwRtlHashMapRemove(pMetaDataMap, pCurrAttr->type.lberbv.bv_val, &pair);

        pszAttrType = (PSTR) pair.pKey;
        pSupplierMetaData = (PVMDIR_ATTRIBUTE_METADATA) pair.pValue;

        /*
         * Supplier won: Attribute type found with value: pSupplierMetaData
         * Supplier lost: Attribute type found with value: NULL
         */
        if (dwError == 0 && IS_VMDIR_REPL_ATTR_CONFLICT(pSupplierMetaData) == FALSE)
        {
            dwError = VmDirMetaDataSetLocalUsn(pSupplierMetaData, localUsn);
            BAIL_ON_VMDIR_ERROR(dwError);

            VmDirFreeMetaData(pCurrAttr->pMetaData);
            pCurrAttr->pMetaData = pSupplierMetaData;
            pair.pValue = NULL;
        }
        else if (dwError == LW_STATUS_NOT_FOUND) //local/non-replicated attribute
        {
            char  origTimeStamp[VMDIR_ORIG_TIME_STR_LEN] = {'\0'};

            dwError = VmDirGenOriginatingTimeStr(origTimeStamp);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirMetaDataCreate(
                    localUsn,
                    (UINT64) 1,
                    gVmdirServerGlobals.invocationId.lberbv.bv_val,
                    origTimeStamp,
                    localUsn,
                    &pCurrAttr->pMetaData);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirFreeMetaDataMapPair(&pair, NULL);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    VmDirFreeMetaDataMapPair(&pair, NULL);
    goto cleanup;
}

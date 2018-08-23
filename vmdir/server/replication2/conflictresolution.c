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
BOOLEAN
_VmDirIsBenignReplConflict(
    PVDIR_ENTRY     pEntry,
    PVDIR_ATTRIBUTE pAttr
    );

static
BOOLEAN
_VmDirReplAttrConflictCheck(
    PVMDIR_ATTRIBUTE_METADATA     pSupplierMetaData,
    PVMDIR_ATTRIBUTE_METADATA     pConsumerMetaData
    );

/* Detect and resolve attribute level conflicts.
 *
 * Read consumer attributes' meta data corresponding to given supplier attributes' meta data,
 * "compare" them, and "mark" the losing supplier attribute meta data.
 *
 * To resolve conflicts between "simultaneous" modifications to same attribute on 2 replicas,
 * following fields (in that order/priority) in the attribute meta data are used:
 * 1) version #
 * 2) server ID
 *
 * Logic:
 *
 * - If supplier attribute version # is > consumer attribute version #, there is "no conflict",
 *   and supplier WINS,
 *      => supplier attribute mod should be applied to this consumer.
 * - If supplier attribute version # is < consumer attribute version #, there is a conflict,
 *   and supplier LOSES,
 *      => supplier attribute mod should NOT be applied to this consumer
 * - If supplier attribute version # is = consumer attribute version #, there is is conflict,
 *   and conflict is resolved
 *   by lexicographical comparison of supplier and consumer server IDs. I.e. the attribute
 *   change on the server with higher serverID WINs.
 *
 *
 *   This function reads (from local DB) consumer attribute meta data, **CORRESPONDING** to
 *   the supplier attributes meta data  and frees the contents of losing supplier meta data
 *
 *   A losing supplier attribute meta data will hlooks like: "<attr name>:"
 *   A winning supplier attribute meta data will have valid contents were as losing supplier
 *   attribute metadata will be NULL.
 */

int
VmDirReplResolveConflicts(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PLW_HASHMAP         pMetaDataMap
    )
{
    int                          retVal = LDAP_SUCCESS;
    int                          dbRetVal = 0;
    PSTR                         pszAttrType = NULL;
    DWORD                        dwConflictCnt = 0;
    LW_HASHMAP_ITER              iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR              pair = {NULL, NULL};
    PVDIR_ATTRIBUTE              pConsumerAttr = NULL;
    PVMDIR_ATTRIBUTE_METADATA    pSupplierMetaData = NULL;
    PVMDIR_REPLICATION_METRICS   pReplMetrics = NULL;

    if (!pOperation || !pEntry || !pMetaDataMap)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_INVALID_PARAMETER);
    }

    while (LwRtlHashMapIterate(pMetaDataMap, &iter, &pair))
    {
        pszAttrType = (PSTR) pair.pKey;
        pSupplierMetaData = (PVMDIR_ATTRIBUTE_METADATA) pair.pValue;

        if (VmDirStringCompareA(pszAttrType, ATTR_OBJECT_GUID, FALSE) == 0)
        {
            continue;
        }

        VmDirFreeAttribute(pConsumerAttr);
        pConsumerAttr = NULL;

        retVal = VmDirAttributeAllocate(pszAttrType, 0, pOperation->pSchemaCtx, &pConsumerAttr);
        BAIL_ON_LDAP_ERROR(
                retVal,
                LDAP_OPERATIONS_ERROR,
                (pOperation->ldapResult.pszErrMsg),
                "VmDirAttributeAllocate failed", VDIR_SAFE_STRING(pOperation->pBEErrorMsg));

        dbRetVal = pOperation->pBEIF->pfnBEGetAttrMetaData(
                pOperation->pBECtx, pConsumerAttr, pEntry->eId);

        if (dbRetVal)
        {
            switch (dbRetVal)
            {
                case ERROR_BACKEND_ATTR_META_DATA_NOTFOUND: //When a new attribute is being added
                    // => Supplier attribute meta data WINS against consumer attribute meta data
                    break;

                default:
                     BAIL_ON_LDAP_ERROR(
                             retVal,
                             LDAP_OPERATIONS_ERROR,
                             (pOperation->ldapResult.pszErrMsg),
                             "pfnBEGetAttrMetaData failed - (%d)(%s)",
                             dbRetVal,
                             VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
            }
        }
        else
        {
            BOOLEAN             bSupplierWon = FALSE;
            BOOLEAN             bIsSameAttrValue = FALSE;
            PSZ_METADATA_BUF    pszSupplierMetaData = {'\0'};
            PSZ_METADATA_BUF    pszConsumerMetaData = {'\0'};

            bSupplierWon = _VmDirReplAttrConflictCheck(
                    pSupplierMetaData, pConsumerAttr->pMetaData);

            bIsSameAttrValue = _VmDirIsBenignReplConflict(
                    pEntry, pConsumerAttr);

            //Ignore error - used only for logging
            VmDirMetaDataSerialize(pSupplierMetaData, &pszSupplierMetaData[0]);
            VmDirMetaDataSerialize(pConsumerAttr->pMetaData, &pszConsumerMetaData[0]);

            if (bSupplierWon == FALSE)
            {
                if (VmDirStringCompareA(pszAttrType, ATTR_USN_CHANGED, FALSE) == 0)
                {
                    // Need to keep usnChanged to advance localUSN for this replication change.
                    retVal = VmDirMetaDataCopyContent(pConsumerAttr->pMetaData, pSupplierMetaData);
                    BAIL_ON_VMDIR_ERROR(retVal);
                }
                else
                {
                    VMDIR_FREE_REPL_ATTR_IN_CONFLICT(pSupplierMetaData);
                }

                dwConflictCnt++;

                if (!bIsSameAttrValue)
                {
                    VMDIR_LOG_WARNING(
                            VMDIR_LOG_MASK_ALL,
                            "%s: supplier version loses."
                            " pszAttrType: %s supplier attr meta: %s, consumer attr meta: %s ",
                            __FUNCTION__,
                            pEntry->dn.lberbv.bv_val,
                            pszAttrType,
                            pszSupplierMetaData,
                            pszConsumerMetaData);
                }
            }
            else
            {
                VMDIR_LOG_VERBOSE(
                        VMDIR_LOG_MASK_ALL,
                        "%s: supplier version wins."
                        " pszAttrType: %s supplier attr meta: %s, consumer attr meta: %s ",
                        __FUNCTION__,
                        pEntry->dn.lberbv.bv_val,
                        pszAttrType,
                        pszSupplierMetaData,
                        pszConsumerMetaData);
            }
        }
    }

    if (dwConflictCnt > 0)
    {
        if (VmDirReplMetricsCacheFind(pOperation->pszPartner, &pReplMetrics) == 0)
        {
            VmMetricsCounterAdd(pReplMetrics->pCountConflictResolved, dwConflictCnt);
        }
    }

cleanup:
    VmDirFreeAttribute(pConsumerAttr);
    return retVal;

ldaperror:
error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", retVal);
    goto cleanup;
}

/*
 * Determine whether the supplier's attr-value-meta-data wins by checking it against local
 * attr-meta-data and local attr-value-meta-data.
 * It first compares the <version><invocation-id> of that in local attr-meta-data which was
 * applied either in the previous transaction or the previous modification in the current
 * transactions. Then if the <version><invocation-id> matches, it looks up the local server
 * to see if the same attr-value-meta-data exist: if supplier's attr-value-meta-data has a
 * newer timestamp then it wins and inScope set to TRUE.
 */
DWORD
VmDirReplResolveValueMetaDataConflicts(
    PVDIR_OPERATION                    pModOp,
    PVDIR_ATTRIBUTE                    pAttr,
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pSupplierValueMetaData,
    ENTRYID                            entryId,
    PBOOLEAN                           pInScope
    )
{
    DWORD                              dwError = 0;
    VDIR_BERVALUE                      bervSupplierValueMetaData = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE                      bervConsumerValueMetaData = VDIR_BERVALUE_INIT;
    DEQUE                              valueMetaDataQueue = {0};
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pConsumerValueMetaData = NULL;

    if (!pModOp || !pAttr || !pSupplierValueMetaData || !pInScope)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    *pInScope = TRUE;
    dwError = pModOp->pBEIF->pfnBEGetAttrMetaData(pModOp->pBECtx, pAttr, entryId);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Consumer <version><originating-server-id> in metaValueData
    //not match supplier's <version<<originating-server-id> in metaData
    //this value-meta-data out of scope
    if (pSupplierValueMetaData->version != pAttr->pMetaData->version ||
        VmDirStringCompareA(
            pSupplierValueMetaData->pszOrigInvoId, pAttr->pMetaData->pszOrigInvoId, TRUE) != 0)
    {
        *pInScope = FALSE;
        goto cleanup;
    }

    if (VmDirLogGetMask() & LDAP_DEBUG_REPL)
    {
        //Ignore error, used only for logging
        VmDirValueMetaDataSerialize(pSupplierValueMetaData, &bervSupplierValueMetaData);
        VmDirValueMetaDataSerialize(pConsumerValueMetaData, &bervConsumerValueMetaData);
    }

    dwError = pModOp->pBEIF->pfnBEGetAttrValueMetaData(
            pModOp->pBECtx, entryId, pAttr->pATDesc->usAttrID, &valueMetaDataQueue);
    BAIL_ON_VMDIR_ERROR(dwError);

    while(!dequeIsEmpty(&valueMetaDataQueue))
    {
        VMDIR_SAFE_FREE_VALUE_METADATA(pConsumerValueMetaData);

        dequePopLeft(&valueMetaDataQueue, (PVOID*)&pConsumerValueMetaData);

        if (pConsumerValueMetaData->dwValSize != pSupplierValueMetaData->dwValSize ||
            VmDirCompareMemory(
                    pConsumerValueMetaData->pszValue,
                    pSupplierValueMetaData->pszValue,
                    pConsumerValueMetaData->dwValSize) != 0)
        {
            continue;
        }

        if (VmDirStringCompareA(
                    pConsumerValueMetaData->pszValChgOrigTime,
                    pSupplierValueMetaData->pszValChgOrigTime,
                    TRUE) > 0)
        {
            *pInScope = FALSE;

            VMDIR_LOG_DEBUG(
                    LDAP_DEBUG_REPL,
                    "%s: supplier attr-value-meta lose: %s consumer: %s",
                    __FUNCTION__,
                    VDIR_SAFE_STRING(bervSupplierValueMetaData.lberbv_val),
                    VDIR_SAFE_STRING(bervConsumerValueMetaData.lberbv_val));
        }
    }

    if (*pInScope)
    {
        VMDIR_LOG_DEBUG(
                LDAP_DEBUG_REPL,
                "%s: supplier attr-value-meta won: %s",
                __FUNCTION__,
                VDIR_SAFE_STRING(bervSupplierValueMetaData.lberbv_val));
    }

cleanup:
    VmDirFreeBervalContent(&bervSupplierValueMetaData);
    VmDirFreeBervalContent(&bervConsumerValueMetaData);
    VMDIR_SAFE_FREE_VALUE_METADATA(pConsumerValueMetaData);
    VmDirFreeAttrValueMetaDataContent(&valueMetaDataQueue);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

static
BOOLEAN
_VmDirReplAttrConflictCheck(
    PVMDIR_ATTRIBUTE_METADATA     pSupplierMetaData,
    PVMDIR_ATTRIBUTE_METADATA     pConsumerMetaData
    )
{
    BOOLEAN    bSupplierWon = FALSE;

    if ((pSupplierMetaData->version > pConsumerMetaData->version) ||
        (pSupplierMetaData->version == pConsumerMetaData->version &&
         VmDirStringNCompareA(
             pSupplierMetaData->pszOrigInvoId,
             pConsumerMetaData->pszOrigInvoId,
             VMDIR_GUID_STR_LEN,
             FALSE) > 0))
    {
        bSupplierWon = TRUE;
    }

    return bSupplierWon;
}

static
BOOLEAN
_VmDirIsBenignReplConflict(
    PVDIR_ENTRY     pEntry,
    PVDIR_ATTRIBUTE pAttr
    )
{
    int            i = 0;
    CHAR           excludeAttrs[] = {ATTR_USN_CHANGED};
    DWORD          dwError = 0;
    BOOLEAN        bIsBenign = FALSE;
    VDIR_ENTRY     consumerEntry = {0};

    if (pEntry->eId)
    {
        if (!consumerEntry.dn.lberbv_val)
        {
            PVDIR_BACKEND_INTERFACE pBE = NULL;
            pBE = VmDirBackendSelect(NULL);
            assert(pBE);

            dwError = pBE->pfnBESimpleIdToEntry(pEntry->eId, &consumerEntry);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        for (i=0; i< VMDIR_ARRAY_SIZE(excludeAttrs); i++)
        {
            if (VmDirStringCompareA(pAttr->type.lberbv_val, &excludeAttrs[i], FALSE) == 0)
            {
                bIsBenign = TRUE;
                goto cleanup;
            }
        }

        bIsBenign = VmDirIsSameConsumerSupplierEntryAttr(pAttr, pEntry, &consumerEntry);
    }

cleanup:
    VmDirFreeEntryContent(&consumerEntry);
    return bIsBenign;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

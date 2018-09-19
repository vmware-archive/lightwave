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

/*
 * Detach attribute value meta data from the entry's attributes,
 * and set ppAttrAttrValueMetaData to the attribute value meta attribute
 * so that it will be handled seperated.
 */
DWORD
VmDirValueMetaDataDetachFromEntry(
    PVDIR_ENTRY    pEntry,
    PDEQUE         pValueMetaDataQueue
    )
{
    DWORD                              dwError = 0;
    DWORD                              dwCnt = 0;
    PVDIR_ATTRIBUTE                    currAttr = NULL;
    PVDIR_ATTRIBUTE                    prevAttr = NULL;
    PVDIR_ATTRIBUTE                    pAttrAttrValueMetaData = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    if (!pEntry || !pValueMetaDataQueue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    for (prevAttr = NULL, currAttr = pEntry->attrs;
         currAttr;
         prevAttr = currAttr, currAttr = currAttr->next)
    {
        if (VmDirStringCompareA(
                    currAttr->type.lberbv.bv_val, ATTR_ATTR_VALUE_META_DATA, FALSE) == 0)
        {
            if (prevAttr == NULL)
            {
                pEntry->attrs = currAttr->next;
            }
            else
            {
                prevAttr->next = currAttr->next;
            }

            pAttrAttrValueMetaData = currAttr;
            break;
        }
    }

    for (dwCnt = 0;
         pAttrAttrValueMetaData && pAttrAttrValueMetaData->vals[dwCnt].lberbv.bv_val != NULL;
         dwCnt++)
    {
        dwError = VmDirValueMetaDataDeserialize(
                pAttrAttrValueMetaData->vals[dwCnt].lberbv.bv_val, &pValueMetaData);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = dequePush(pValueMetaDataQueue, pValueMetaData);
        BAIL_ON_VMDIR_ERROR(dwError);

        pValueMetaData = NULL;
    }

cleanup:
    VmDirFreeAttribute(pAttrAttrValueMetaData);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    VmDirFreeValueMetaData(pValueMetaData);
    VmDirFreeAttrValueMetaDataDequeueContent(pValueMetaDataQueue);
    goto cleanup;
}

/*
 * Attach and alter attribute value meta data to that attribute in pEntry
 * so that they can be inserted into the backend index when the
 * entry is added to backend.
 */
DWORD
VmDirValueMetaDataUpdateLocalUsn(
    PVDIR_ENTRY    pEntry,
    USN            localUsn,
    PDEQUE         pValueMetaDataQueue
    )
{
    DWORD                              dwError = 0;
    PVDIR_ATTRIBUTE                    pAttr = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    if (!pValueMetaDataQueue || !pEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    while (!dequeIsEmpty(pValueMetaDataQueue))
    {
        dequePopLeft(pValueMetaDataQueue, (PVOID*)&pValueMetaData);

        pAttr = VmDirEntryFindAttribute(pValueMetaData->pszAttrType, pEntry);

        /*
         * TODO follow-up to understand whether it is a valid case to
         * have value metadata without attribute.
         */
        if (pAttr)
        {
            pValueMetaData->localUsn = localUsn;

            dwError = dequePush(&pAttr->valueMetaDataToAdd, pValueMetaData);
            BAIL_ON_VMDIR_ERROR(dwError);

            pValueMetaData = NULL;
        }
        else
        {
            VMDIR_LOG_ERROR(
                    VMDIR_LOG_MASK_ALL,
                    "%s: value metadata present but no corresponding attribute, attr: %s",
                    __FUNCTION__,
                    pValueMetaData->pszAttrType);
        }

        VMDIR_SAFE_FREE_VALUE_METADATA(pValueMetaData);
    }

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_VALUE_METADATA(pValueMetaData);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

/*
 * If any mod is a MOD_OP_REPLACE on a multi-value attribute,
 * delete that attribute's attr-value-meta-data
 */
DWORD
VmDirValueMetaDataDeleteOldForReplace(
    PVDIR_OPERATION       pModOp,
    PVDIR_MODIFICATION    pMods,
    ENTRYID               entryId
    )
{
    DWORD                 dwError = 0;
    PVDIR_MODIFICATION    pTempMod = NULL;
    DEQUE                 valueMetaDataToDelete = {0};

    if (!pModOp)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    for(pTempMod = pMods; pTempMod; pTempMod = pTempMod->next )
    {
         if (pTempMod->operation != MOD_OP_REPLACE || pTempMod->attr.pATDesc->bSingleValue)
         {
             continue;
         }

         dwError = pModOp->pBEIF->pfnBEGetAttrValueMetaData(
                 pModOp->pBECtx, entryId, pTempMod->attr.pATDesc->usAttrID, &valueMetaDataToDelete);
         BAIL_ON_VMDIR_ERROR(dwError);

         if (dequeIsEmpty(&valueMetaDataToDelete))
         {
             continue;
         }

         dwError = pModOp->pBEIF->pfnBEUpdateAttrValueMetaData(
                 pModOp->pBECtx,
                 entryId,
                 pTempMod->attr.pATDesc->usAttrID,
                 BE_INDEX_OP_TYPE_DELETE,
                 &valueMetaDataToDelete);
         BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VmDirFreeAttrValueMetaDataDequeueContent(&valueMetaDataToDelete);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

/*
 * First determine if each attribute value meta data in pAttrAttrValueMetaData
 * win those in local database (if they exist locally); then create mod for
 * adding/deleting the attribute value for those with winning attribute value meta data
 * from supplier.
 */
DWORD
VmDirReplSetAttrNewValueMetaData(
    PDEQUE              pValueMetaDataQueue,
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    ENTRYID             entryId,
    USN                 localUsn,
    PVDIR_OPERATION     pModOp
    )
{
    DWORD                              dwError = 0;
    DWORD                              dwOpCode = 0;
    VDIR_BERVALUE                      value = VDIR_BERVALUE_INIT;
    BOOLEAN                            bInScope = FALSE;
    ModifyReq                         *pModifyReq = NULL;
    PVDIR_MODIFICATION                 pMod = NULL;
    PVDIR_MODIFICATION                 pCurrMod = NULL;
    PVDIR_MODIFICATION                 pPrevMod = NULL;
    PVDIR_ATTRIBUTE                    pAttr = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    if (!pValueMetaDataQueue || !pSchemaCtx || !pModOp)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pModifyReq = &(pModOp->request.modifyReq);

    while (!dequeIsEmpty(pValueMetaDataQueue))
    {
        dequePopLeft(pValueMetaDataQueue, (PVOID*)&pValueMetaData);

        bInScope = FALSE;

        VmDirFreeAttribute(pAttr);
        pAttr = NULL;
        dwError = VmDirAttributeAllocate(pValueMetaData->pszAttrType, 1, pSchemaCtx, &pAttr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirReplResolveValueMetaDataConflicts(
                pModOp, pAttr, pValueMetaData, entryId, &bInScope);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (bInScope == FALSE)
        {
            VMDIR_SAFE_FREE_VALUE_METADATA(pValueMetaData);
            continue;
        }

        pValueMetaData->localUsn = localUsn;

        dwOpCode = pValueMetaData->dwOpCode;

        VmDirFreeBervalContent(&value);
        dwError = VmDirAllocateAndCopyMemory(
                pValueMetaData->pszValue, pValueMetaData->dwValSize, (PVOID*)&value.lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);
        value.lberbv_len = pValueMetaData->dwValSize;
        value.bOwnBvVal = TRUE;

        dwError = dequePush(&pAttr->valueMetaDataToAdd, pValueMetaData);
        BAIL_ON_VMDIR_ERROR(dwError);
        pValueMetaData = NULL;

        dwError = pModOp->pBEIF->pfnBEUpdateAttrValueMetaData(
                pModOp->pBECtx,
                entryId,
                pAttr->pATDesc->usAttrID,
                BE_INDEX_OP_TYPE_UPDATE,
                &pAttr->valueMetaDataToAdd);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateMemory(sizeof(VDIR_MODIFICATION), (PVOID *)&pCurrMod);
        BAIL_ON_VMDIR_ERROR(dwError);

        pCurrMod->operation = dwOpCode;

        dwError = VmDirAttributeInitialize(
                pAttr->type.lberbv.bv_val, 1, pSchemaCtx, &pCurrMod->attr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateAndCopyMemory(
                value.lberbv_val, value.lberbv_len, (PVOID*)&pCurrMod->attr.vals[0].lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        pCurrMod->attr.vals[0].bOwnBvVal = TRUE;
        pCurrMod->attr.vals[0].lberbv.bv_len = value.lberbv_len;

        for (pMod = pModifyReq->mods; pMod; pPrevMod = pMod, pMod = pMod->next)
        {
            if (pMod->attr.pATDesc->usAttrID == pCurrMod->attr.pATDesc->usAttrID &&
                pMod->operation == pCurrMod->operation)
            {
                break;
            }
        }

        if (pMod == NULL)
        {
            if (pPrevMod == NULL)
            {
                pModifyReq->mods = pCurrMod;
            }
            else
            {
                pPrevMod->next = pCurrMod;
            }
            pModifyReq->numMods++;
            pCurrMod = NULL;
        }
        else
        {
           // add/delete attr value on the same attribute exists, merge the new mod into it.
            dwError = VmDirReallocateMemoryWithInit(
                    pMod->attr.vals,
                    (PVOID*)(&(pMod->attr.vals)),
                    (pMod->attr.numVals + 2) * sizeof(VDIR_BERVALUE),
                    (pMod->attr.numVals + 1) * sizeof(VDIR_BERVALUE));
           BAIL_ON_VMDIR_ERROR(dwError);

           dwError = VmDirBervalContentDup(
                   &pCurrMod->attr.vals[0], &pMod->attr.vals[pMod->attr.numVals]);
           BAIL_ON_VMDIR_ERROR(dwError);

           pMod->attr.numVals++;
           memset(&(pMod->attr.vals[pMod->attr.numVals]), 0, sizeof(VDIR_BERVALUE));

           VmDirModificationFree(pCurrMod);
           pCurrMod = NULL;
       }
    }

cleanup:
    VmDirFreeBervalContent(&value);
    VmDirFreeAttribute(pAttr);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    VMDIR_SAFE_FREE_VALUE_METADATA(pValueMetaData);
    VmDirModificationFree(pCurrMod);
    goto cleanup;
}

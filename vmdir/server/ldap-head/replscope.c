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
VmDirIsAttrInScope(
    PVDIR_OPERATION             pOperation,
    PCSTR                       pszAttrType,
    PVMDIR_ATTRIBUTE_METADATA   pAttrMetaData,
    USN                         priorSentUSNCreated,
    PBOOLEAN                    pbInScope
    )
{
    DWORD               dwError = 0;
    PSZ_METADATA_BUF    pszMetaData = {'\0'};

    /*
     * Skip input validation for pszAttrType
     * For deleted attributes: pszAttrType can be NULL
     */
    if (!pOperation ||
        !pAttrMetaData ||
        !pbInScope)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    *pbInScope = FALSE;

    //Ignore error - used only for logging
    VmDirMetaDataSerialize(pAttrMetaData, pszMetaData);

    // Skip the attribute:
    //      - If it is one of those attributes that have "local" scope only.
    //      E.g. sending ATTR_LAST_LOCAL_USN_PROCESSED and ATTR_UP_TO_DATE_VECTOR, causes
    //      continuous back-forth replication of Replication Agreements and Server
    //      entries between various servers.
    if ((pszAttrType &&
         (VmDirStringCompareA(pszAttrType, ATTR_LAST_LOCAL_USN_PROCESSED, FALSE) == 0 ||
          VmDirStringCompareA(pszAttrType, ATTR_UP_TO_DATE_VECTOR, FALSE) == 0 ||
          VmDirStringCompareA(pszAttrType, VDIR_ATTRIBUTE_SEQUENCE_RID, FALSE) == 0)))
    {
        // Reset metaData value so that we don't send local only attribute back.
        *pbInScope = FALSE;
        goto cleanup;
    }
    else if (pszAttrType && (VmDirStringCompareA(pszAttrType, ATTR_USN_CHANGED, FALSE) == 0))
    {
        ; // always send uSNChanged. (PR 1573117)
    }
    else if (pszAttrType && gVmdirServerGlobals.dwDomainFunctionalLevel >= VDIR_DFL_MODDN &&
            (VmDirStringCompareA(pszAttrType, ATTR_OBJECT_GUID, FALSE) == 0))
    {
        ; // always send objectGUID to uniquely identify entries, regardless
          // which node the entry was created. (PR 1730608)
    }
    else
    {
        BOOLEAN usnInScope = FALSE;

        dwError = VmDirIsUsnInScope(
                pOperation,
                pszAttrType,
                pAttrMetaData->pszOrigInvoId,
                pAttrMetaData->origUsn,
                pAttrMetaData->localUsn,
                priorSentUSNCreated,
                &usnInScope);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!usnInScope)
        {
            VMDIR_LOG_VERBOSE(
                    LDAP_DEBUG_REPL_ATTR,
                    "%s: Attribute: %s, metaData: %s, replication scope = FALSE",
                    __FUNCTION__,
                    pszAttrType,
                    pszMetaData);
            // Reset metaData value so that we don't send metaData for this attribute back.
            *pbInScope = FALSE;
            goto cleanup;
        }
    }

    VMDIR_LOG_INFO(
            LDAP_DEBUG_REPL_ATTR,
            "%s: Attribute: %s, metaData: %s, replication scope = TRUE",
            __FUNCTION__,
            pszAttrType,
            pszMetaData);

    *pbInScope = TRUE;

cleanup:
    if (*pbInScope == FALSE)
    {
        VMDIR_FREE_ATTRIBUTE_NOT_IN_SCOPE(pAttrMetaData);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirIsAttrValueInScope(
   PVDIR_OPERATION    pOperation,
   PDEQUE             pAllValueMetaDataQueue,
   PDEQUE             pValueMetaDataToSendQueue
   )
{
    DWORD                              dwError = 0;
    VDIR_BERVALUE                      bervValueMetaData = VDIR_BERVALUE_INIT;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    if (!pOperation ||
        !pAllValueMetaDataQueue ||
        !pValueMetaDataToSendQueue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    while (!dequeIsEmpty(pAllValueMetaDataQueue))
    {
        BOOLEAN bUsnInScope = FALSE;

        VMDIR_SAFE_FREE_VALUE_METADATA(pValueMetaData);
        dequePopLeft(pAllValueMetaDataQueue, (PVOID*)&pValueMetaData);

        dwError = VmDirIsUsnInScope(
                pOperation,
                NULL,
                pValueMetaData->pszValChgOrigInvoId,
                pValueMetaData->valChgOrigUsn,
                pValueMetaData->localUsn,
                0,
                &bUsnInScope);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!bUsnInScope)
        {
            continue;
        }

        if (VmDirLogGetMask() & LDAP_DEBUG_REPL)
        {
            //ignore error
            VmDirValueMetaDataSerialize(pValueMetaData, &bervValueMetaData);
            VMDIR_LOG_INFO(
                    LDAP_DEBUG_REPL,
                    "%s: valueMetaData: %s, usnInScope true",
                    __FUNCTION__,
                    VDIR_SAFE_STRING(bervValueMetaData.lberbv_val));
            VmDirFreeBervalContent(&bervValueMetaData);
        }

        dwError = dequePush(pValueMetaDataToSendQueue, pValueMetaData);
        BAIL_ON_VMDIR_ERROR(dwError);

        pValueMetaData = NULL;
    }

cleanup:
    VMDIR_SAFE_FREE_VALUE_METADATA(pValueMetaData);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

/*
 * Test whether the origUsn is in scope so that attribute, attr-meta-data or
 * attr-value-meta-data be sent back to the replicaiton consumer based on whether
 * the origUsn for that invocationId has been processed already by the consumer
 * TODO: Needed Refractoring
 */
DWORD
VmDirIsUsnInScope(
    PVDIR_OPERATION     pOperation,
    PCSTR               pAttrName,
    PCSTR               pszOrigInvocationId,
    USN                 origUsn,
    USN                 localUSN,
    USN                 priorSentUSNCreated,
    PBOOLEAN            pbIsUsnInScope
    )
{
    DWORD                   dwError = 0;
    PLW_HASHTABLE_NODE      pNode = NULL;
    PSTR                    pszLocalErrorMsg = NULL;
    UptoDateVectorEntry     *pUtdVectorEntry = NULL;
    UptoDateVectorEntry     *pNewUtdVectorEntry = NULL;
    int                     retVal = 0;

    if (!pOperation ||
        !pszOrigInvocationId ||
        !pbIsUsnInScope ||
        !pOperation->syncReqCtrl ||
        !pOperation->syncDoneCtrl)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    *pbIsUsnInScope = FALSE;

    //Originating server for the current state is same as the requesting server
    if (VmDirStringCompareA(
                pszOrigInvocationId,
                pOperation->syncReqCtrl->value.syncReqCtrlVal.reqInvocationId.lberbv.bv_val,
                TRUE) == 0)
    {
        *pbIsUsnInScope = FALSE;
        goto cleanup;
    }

    retVal = LwRtlHashTableFindKey(
            pOperation->syncDoneCtrl->value.syncDoneCtrlVal.htUtdVector, &pNode, pszOrigInvocationId);

    retVal = LwNtStatusToWin32Error(retVal);

    if (retVal != 0 && retVal != ERROR_NOT_FOUND)
    {
        VMDIR_LOG_VERBOSE(
                VMDIR_LOG_MASK_ALL,
                "%s: LwRtlHashTableFindKey failed for origInvocationId: %s",
                __FUNCTION__,
                pszOrigInvocationId);
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "LwRtlHashTableFindKey failed.");
    }

    if (pNode == NULL)
    {
        VDIR_BERVALUE    bvServerId = VDIR_BERVALUE_INIT;

        dwError = VmDirAllocateMemory(sizeof(UptoDateVectorEntry), (PVOID *)&pNewUtdVectorEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        bvServerId.lberbv.bv_val = (PSTR)pszOrigInvocationId;
        bvServerId.lberbv.bv_len = VmDirStringLenA(pszOrigInvocationId);

        dwError = VmDirBervalContentDup(&bvServerId, &pNewUtdVectorEntry->invocationId);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNewUtdVectorEntry->currMaxOrigUsnProcessed = origUsn;

        LwRtlHashTableResizeAndInsert(
                pOperation->syncDoneCtrl->value.syncDoneCtrlVal.htUtdVector,
                &pNewUtdVectorEntry->Node,
                &pNode);

        // assert the key of added node is unique
        assert(pNode == NULL);

        pNewUtdVectorEntry = NULL;
        *pbIsUsnInScope = TRUE;

        goto cleanup;
    }

    pUtdVectorEntry = (UptoDateVectorEntry *)LW_STRUCT_FROM_FIELD(pNode, UptoDateVectorEntry, Node);

    if (origUsn > pUtdVectorEntry->reqLastOrigUsnProcessed )
    {
        // attribute or the valueMetaData item in scope if origUsn valueMetaData is > the current highest
        if (origUsn > pUtdVectorEntry->currMaxOrigUsnProcessed )
        {
            pUtdVectorEntry->currMaxOrigUsnProcessed = origUsn;
        }

        // Note, this handles ADD->MODIFY case but not multiple MODIFYs scenario.
        // However, it is fine as consumer should be able to handle redundant feed from supplier.
        // The key point here is to NOT send ATTR_USN_CREATED, so we can derive correct sync_state in WriteSyncStateControl.
        if (localUSN > priorSentUSNCreated)
        {
            *pbIsUsnInScope = TRUE;

            if (priorSentUSNCreated > 0)
            {
                VMDIR_LOG_INFO(
                        LDAP_DEBUG_REPL,
                        "%s new usn %llu after prior usncreated %llu attr %s",
                        __FUNCTION__,
                        origUsn,
                        priorSentUSNCreated,
                        VDIR_SAFE_STRING(pAttrName));
            }
        }
        else
        {
            VMDIR_LOG_INFO(
                    LDAP_DEBUG_REPL,
                    "%s (add->modify) race condition avoided. skip prior usncreated %llu attr %s",
                    __FUNCTION__,
                    priorSentUSNCreated,
                    VDIR_SAFE_STRING(pAttrName));
        }

        goto cleanup;
    }

    VMDIR_LOG_INFO(
            LDAP_DEBUG_REPL,
            "%s: (not in scope) attr name: %s orig invo: %s utdUsn: %"PRId64" usn: %"PRId64,
            __FUNCTION__,
            VDIR_SAFE_STRING(pAttrName),
            VDIR_SAFE_STRING(pszOrigInvocationId),
            pUtdVectorEntry->reqLastOrigUsnProcessed,
            origUsn);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    if (pNewUtdVectorEntry)
    {
        VmDirFreeBervalContent(&pNewUtdVectorEntry->invocationId);
    }
    VMDIR_SAFE_FREE_MEMORY(pNewUtdVectorEntry);
    goto cleanup;
}

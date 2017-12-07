/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

VOID
VmDirReplicationEncodeEntryForRetry(
    PVDIR_ENTRY pEntry,
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry
    )
{
    DWORD           dwError = 0;
    PVDIR_BERVALUE  pBervEncodedEntry = NULL;

    if (pPageEntry->pBervEncodedEntry == NULL)
    {
        if (pEntry != NULL)
        {
            dwError = VmDirAllocateMemory(sizeof(VDIR_BERVALUE), (PVOID)&pBervEncodedEntry);
            BAIL_ON_VMDIR_ERROR(dwError);

            pBervEncodedEntry->bOwnBvVal = TRUE;

            dwError = VmDirBervalContentDup( &(pEntry->dn), &pPageEntry->reqDn);
            BAIL_ON_VMDIR_ERROR(dwError);

            /*
             * _VmDirEntrySanityCheck:
             *     1. attribute has its descriptor
             *     2. must has ATTR_DN
             *     3. must has ATTR_OBJECT_CLASS
             * bValidateEntry is FALSE to disable _VmDirEntrySanityCheck.
             * because ATTR_DN will not be available in modify/delete case
             */
            dwError = VmDirEncodeEntry(pEntry, pBervEncodedEntry, FALSE);
            BAIL_ON_VMDIR_ERROR(dwError);

        }
        pPageEntry->pBervEncodedEntry = pBervEncodedEntry;
    }

cleanup:
    return;

error:
    VMDIR_LOG_ERROR(
        VMDIR_LOG_MASK_ALL,
        "%s: Error while encoding the entry: error = (%d)",
        __FUNCTION__,
       dwError);
    VmDirFreeBerval(pBervEncodedEntry);
    goto cleanup;
}

DWORD
VmDirReplicationDecodeEntryForRetry(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry,
    PVDIR_ENTRY                     pEntry
    )
{
    DWORD  dwError = 0;
    PSTR   pszEntryValue = NULL;
    size_t len = 0;

    if (pPageEntry->pBervEncodedEntry)
    {
        len = pPageEntry->pBervEncodedEntry->lberbv.bv_len;

        dwError = VmDirAllocateMemory(len + 1, (PVOID *)&pszEntryValue);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirCopyMemory(pszEntryValue, len + 1,
                      pPageEntry->pBervEncodedEntry->lberbv.bv_val, len);
        BAIL_ON_VMDIR_ERROR(dwError);

        pEntry->encodedEntry = pszEntryValue;
        pszEntryValue = NULL; // transfer ownership

        /*
         * LDAP_SYNC_ADD - Decode entry will obtain Dn from entryDN attribute
         * In the case of modify and delete entryDN attribute may not be preset
         * pass the cached DN to decode entry
         */
        dwError = VmDirDecodeEntry(
                      pSchemaCtx,
                      pEntry,
                      (pPageEntry->entryState == LDAP_SYNC_ADD) ? NULL : &pPageEntry->reqDn);
        BAIL_ON_VMDIR_ERROR(dwError);

        /*
         * pEntry->encodedEntry entry will be freed during Unpack and
         * new entry content will be allocated
         *
         * Unpacking is necessary because replication consumer flow, expects the entry to be in
         * ENTRY_STORAGE_FORMAT_NORMAL
         */
        dwError = VmDirEntryUnpack(pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
   }

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszEntryValue);
    VMDIR_LOG_ERROR(
        VMDIR_LOG_MASK_ALL,
        "%s: Error while encoding the entry: error = (%d)",
        __FUNCTION__,
       dwError);
    goto cleanup;
}

DWORD
VmDirReplicationPushFailedEntriesToQueue(
    PVMDIR_REPLICATION_CONTEXT     pContext,
    PVMDIR_REPLICATION_PAGE_ENTRY  pPageEntry
    )
{
    DWORD  dwError = 0;
    PVMDIR_REPLICATION_PAGE_ENTRY  pPageEntryDup = NULL;

    dwError = VmDirReplicationDupPageEntry(pPageEntry, &pPageEntryDup);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = dequePush(pContext->pFailedEntriesQueue, pPageEntryDup);
    BAIL_ON_VMDIR_ERROR(dwError);
    pPageEntryDup = NULL; //transfer ownership

    VMDIR_LOG_INFO(
        VMDIR_LOG_MASK_ALL,
        "%s: Added to queue for later processing, sync_state = (%d) dn = (%s) error = (%d)",
        __FUNCTION__,
        pPageEntry->entryState,
        VDIR_SAFE_STRING(pPageEntry->reqDn.lberbv.bv_val),
        pPageEntry->errVal);

cleanup:
    return dwError;

error:
    VmDirReplicationFreePageEntry(pPageEntryDup);
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error code (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}

DWORD
VmDirReplicationDupPageEntry(
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry,
    PVMDIR_REPLICATION_PAGE_ENTRY*  ppPageEntryDup
    )
{
    DWORD  dwError = 0;
    PVMDIR_REPLICATION_PAGE_ENTRY pTempPageEntry = NULL;

    if (pPageEntry == NULL ||
        pPageEntry->pBervEncodedEntry == NULL ||
        pPageEntry->reqDn.lberbv.bv_val == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(
                  sizeof(VMDIR_REPLICATION_PAGE_ENTRY),
                  (PVOID) &pTempPageEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pPageEntry->dwDnLength != 0)
    {
        dwError = VmDirAllocateStringA(pPageEntry->pszDn, &pTempPageEntry->pszDn);
        BAIL_ON_VMDIR_ERROR(dwError);

        pTempPageEntry->dwDnLength = pPageEntry->dwDnLength;
    }

    pTempPageEntry->entry = NULL;
    pTempPageEntry->entryState = pPageEntry->entryState;
    pTempPageEntry->ulPartnerUSN = pPageEntry->ulPartnerUSN;
    pTempPageEntry->errVal = pPageEntry->errVal;
    pTempPageEntry->pBervEncodedEntry = pPageEntry->pBervEncodedEntry;
    pPageEntry->pBervEncodedEntry = NULL; // transfer ownership

    dwError = VmDirBervalContentDup(&pPageEntry->reqDn, &pTempPageEntry->reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

   *ppPageEntryDup = pTempPageEntry;

cleanup:
    return dwError;

error:
    VmDirReplicationFreePageEntry(pTempPageEntry);
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error code (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}

VOID
VmDirReplicationClearFailedEntriesFromQueue(
    PVMDIR_REPLICATION_CONTEXT     pContext
    )
{
    PVMDIR_REPLICATION_PAGE_ENTRY  pPageEntry = NULL;

    while (dequePop(pContext->pFailedEntriesQueue, (PVOID*)&pPageEntry) == 0)
    {
        if (pPageEntry->errVal == LDAP_ALREADY_EXISTS)
        {
            VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "%s: Entry already exists sync_state = (%d) dn = (%s) error = (%d)",
                 __FUNCTION__,
                 pPageEntry->entryState,
                 VDIR_SAFE_STRING(pPageEntry->reqDn.lberbv.bv_val),
                 pPageEntry->errVal);
        }
        else
        {
            VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: Failure could result in data inconsistency sync_state = (%d) dn = (%s) error = (%d)",
                 __FUNCTION__,
                 pPageEntry->entryState,
                 VDIR_SAFE_STRING(pPageEntry->reqDn.lberbv.bv_val),
                 pPageEntry->errVal);
        }
        VmDirReplicationFreePageEntry(pPageEntry);
        pPageEntry = NULL;
    }
}

VOID
VmDirReplicationFreePageEntry(
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry
    )
{
    if (pPageEntry)
    {
        VmDirFreeBerval(pPageEntry->pBervEncodedEntry);
        VmDirFreeBervalContent(&pPageEntry->reqDn);
        VmDirFreeStringA(pPageEntry->pszDn);
        VMDIR_SAFE_FREE_MEMORY(pPageEntry);
    }
}

VOID
VmDirReapplyFailedEntriesFromQueue(
    PVMDIR_REPLICATION_CONTEXT     pContext
    )
{
    DWORD  dwQueueSize = 0;
    DWORD  dwIter = 0;
    DWORD  entryState = 0;
    int    errVal = 0;
    PVDIR_SCHEMA_CTX   pSchemaCtx = NULL;
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry = NULL;

    pSchemaCtx = pContext->pSchemaCtx;

    do
    {
        dwQueueSize = dequeGetSize(pContext->pFailedEntriesQueue);
        dwIter = 0;

        if (dwQueueSize != 0)
        {
            VMDIR_LOG_INFO(
                LDAP_DEBUG_REPL,
                "%s: Failed entries queue size = (%d) begining of the retry",
                __FUNCTION__,
                dwQueueSize);
        }

        while (dwIter < dwQueueSize)
        {
            // Pop from the queue head
            dequePopLeft(pContext->pFailedEntriesQueue, (PVOID*)&pPageEntry);
            if (pPageEntry && pPageEntry->pBervEncodedEntry)
            {
                VMDIR_LOG_INFO(
                    LDAP_DEBUG_REPL,
                    "%s: retrying entry: %s state: %d",
                    __FUNCTION__,
                    VDIR_SAFE_STRING(pPageEntry->reqDn.lberbv.bv_val),
                    pPageEntry->entryState);

                entryState = pPageEntry->entryState;
                if (entryState == LDAP_SYNC_ADD)
                {
                    errVal = ReplAddEntry(pSchemaCtx, pPageEntry, &pSchemaCtx);
                    pContext->pSchemaCtx = pSchemaCtx;
                }
                else if (entryState == LDAP_SYNC_MODIFY)
                {
                    errVal = ReplModifyEntry(pSchemaCtx, pPageEntry, &pSchemaCtx);
                    pContext->pSchemaCtx = pSchemaCtx;
                }
                else if (entryState == LDAP_SYNC_DELETE)
                {
                    errVal = ReplDeleteEntry(pSchemaCtx, pPageEntry);
                }

                if (errVal == LDAP_SUCCESS)
                {
                    VMDIR_LOG_INFO(
                        VMDIR_LOG_MASK_ALL,
                        "%s: Successfully reapplied sync_state = (%d) dn = (%s)",
                        __FUNCTION__,
                        entryState,
                        VDIR_SAFE_STRING(pPageEntry->reqDn.lberbv.bv_val));

                    VmDirReplicationFreePageEntry(pPageEntry);
                    pPageEntry = NULL;
                }
                else
                {
                    VMDIR_LOG_WARNING(
                        LDAP_DEBUG_REPL,
                        "%s: Failed to reapply sync_state = (%d) dn = (%s) error = (%d)",
                        __FUNCTION__,
                        entryState,
                        VDIR_SAFE_STRING(pPageEntry->reqDn.lberbv.bv_val),
                        errVal);

                    //push to the queue tail
                    dequePush(pContext->pFailedEntriesQueue, pPageEntry);
                    pPageEntry = NULL;
                }
            }
            dwIter++;
        }
     // if queue is non empty and queue size has reduced from the previous queue size then continue
    }while(dequeIsEmpty(pContext->pFailedEntriesQueue) == FALSE &&
           dwQueueSize > dequeGetSize(pContext->pFailedEntriesQueue));

    if (dwQueueSize != 0)
    {
        VMDIR_LOG_INFO(
            LDAP_DEBUG_REPL,
            "%s: Failed entries queue size = (%d) end of the retry",
            __FUNCTION__,
            dequeGetSize(pContext->pFailedEntriesQueue));
    }
}


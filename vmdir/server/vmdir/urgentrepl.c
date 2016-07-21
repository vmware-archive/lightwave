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

DWORD
VmDirdAddToUrgentReplicationServerList(
    PSTR    pszUrgentReplicationServer
    )
{
    BOOLEAN             bInLock = FALSE;
    DWORD               dwError = 0;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    dwError = VmDirdAddToUrgentReplicationServerList_InLock(pszUrgentReplicationServer);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirdAddToUrgentReplicationServerList_InLock(
    PSTR    pszUrgentReplicationServer
    )
{
    DWORD    dwError = 0;
    BOOLEAN  bServerExists = FALSE;
    PVMDIR_URGENT_REPL_SERVER_LIST   pUrgentReplServerList = NULL;
    PVMDIR_URGENT_REPL_SERVER_LIST   pTempUrgentReplServerList = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_URGENT_REPL_SERVER_LIST), (PVOID)&pUrgentReplServerList);
    BAIL_ON_VMDIR_ERROR(dwError);

    pUrgentReplServerList->pInitiatorServerName = pszUrgentReplicationServer;
    pszUrgentReplicationServer = NULL; //ownership transfer
    pUrgentReplServerList->next = NULL;

    if (gVmdirUrgentRepl.pUrgentReplServerList == NULL)
    {
        gVmdirUrgentRepl.pUrgentReplServerList = pUrgentReplServerList;
    }
    else
    {
        pTempUrgentReplServerList = gVmdirUrgentRepl.pUrgentReplServerList;
        while (pTempUrgentReplServerList != NULL)
        {
            if (VmDirStringCompareA(pUrgentReplServerList->pInitiatorServerName,
                                    pTempUrgentReplServerList->pInitiatorServerName,
                                    FALSE) == 0)
            {
                bServerExists = TRUE;
		VMDIR_LOG_DEBUG(LDAP_DEBUG_RPC, "VmDirdAddToUrgentReplicationServerList_InLock server %s is already present", pszUrgentReplicationServer);
                VMDIR_SAFE_FREE_MEMORY(pUrgentReplServerList->pInitiatorServerName);
                VMDIR_SAFE_FREE_MEMORY(pUrgentReplServerList);
                break;
            }
            pTempUrgentReplServerList = pTempUrgentReplServerList->next;
        }

        if (bServerExists == FALSE)
        {
            pUrgentReplServerList->next = gVmdirUrgentRepl.pUrgentReplServerList;
            gVmdirUrgentRepl.pUrgentReplServerList = pUrgentReplServerList;
        }
    }

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszUrgentReplicationServer);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
        "VmDirdAddToUrgentReplicationServerList_InLock failed with status: %d",dwError);
    goto cleanup;
}

PVMDIR_URGENT_REPL_SERVER_LIST
VmDirdGetUrgentReplicationServerList(
    VOID
    )
{
    PVMDIR_URGENT_REPL_SERVER_LIST   pUrgentReplServerList = NULL;
    BOOLEAN     bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    pUrgentReplServerList = VmDirdGetUrgentReplicationServerList_InLock();
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return pUrgentReplServerList;
}

PVMDIR_URGENT_REPL_SERVER_LIST
VmDirdGetUrgentReplicationServerList_InLock(
    VOID
    )
{
    return gVmdirUrgentRepl.pUrgentReplServerList;
}

VOID
VmDirdFreeUrgentReplicationServerList(
    VOID
    )
{
    BOOLEAN     bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    VmDirdFreeUrgentReplicationServerList_InLock();
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return;
}

VOID
VmDirdFreeUrgentReplicationServerList_InLock(
    VOID
    )
{
    PVMDIR_URGENT_REPL_SERVER_LIST   pUrgentReplServerList = NULL;
    PVMDIR_URGENT_REPL_SERVER_LIST   pTempUrgentReplServerList = NULL;

    pUrgentReplServerList = gVmdirUrgentRepl.pUrgentReplServerList;
    while (pUrgentReplServerList != NULL)
    {
        pTempUrgentReplServerList = pUrgentReplServerList;
        pUrgentReplServerList = pUrgentReplServerList->next;
        VMDIR_SAFE_FREE_MEMORY(pTempUrgentReplServerList->pInitiatorServerName);
        VMDIR_SAFE_FREE_MEMORY(pTempUrgentReplServerList);
    }

    gVmdirUrgentRepl.pUrgentReplServerList = NULL;
}

DWORD
VmDirdInitiateUrgentRepl(
    PSTR   pszServerName
    )
{
    DWORD    dwError = 0;
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    dwError = VmDirdAddToUrgentReplicationServerList_InLock(pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirdSetReplNow(TRUE);
    VmDirUrgentReplSignal();

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirdInitiateUrgentRepl: failed with error: %d",dwError);
    goto cleanup;
}

VOID
VmDirSendAllUrgentReplicationResponse(
    VOID
    )
{
    DWORD   dwError = 0;
    PCSTR    pUtdVector = NULL;
    PVMDIR_URGENT_REPL_SERVER_LIST  pUrgentReplServerList = NULL;
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    pUtdVector = VmDirdUrgentReplGetUtdVector_InLock();

    pUrgentReplServerList = VmDirdGetUrgentReplicationServerList_InLock();
    while (pUrgentReplServerList != NULL)
    {
        dwError = VmDirUrgentReplicationResponse(
                     pUrgentReplServerList->pInitiatorServerName,
                     pUtdVector,
                     gVmdirServerGlobals.invocationId.lberbv.bv_val,
                     gVmdirServerGlobals.bvServerObjName.lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        pUrgentReplServerList = pUrgentReplServerList->next;
    }

cleanup:
    VmDirdFreeUrgentReplicationServerList_InLock();
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirSendAllUrgentReplicationResponse: Failed with status: %d", dwError);
    goto cleanup;
}

DWORD
VmDirdUrgentReplSetUtdVector(
    PCSTR pUTDVector
    )
{
    PSTR     pTempUTDVector = NULL;
    DWORD    dwError = 0;
    BOOLEAN  bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    pTempUTDVector = gVmdirUrgentRepl.pUTDVector;
    dwError = VmDirAllocateStringA(pUTDVector, &gVmdirUrgentRepl.pUTDVector);
    BAIL_ON_VMDIR_ERROR(dwError);

    // free only if allocation succeed
    VMDIR_SAFE_FREE_MEMORY(pTempUTDVector);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirdUrgentReplSetUtdVector failed with status: %d", dwError);
    goto cleanup;
}

PCSTR
VmDirdUrgentReplGetUtdVector(
    VOID
    )
{
    PCSTR    pUTDVector = NULL;
    BOOLEAN  bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    pUTDVector = VmDirdUrgentReplGetUtdVector_InLock();
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return pUTDVector;
}

PCSTR
VmDirdUrgentReplGetUtdVector_InLock(
    VOID
    )
{
    return gVmdirUrgentRepl.pUTDVector;
}
/*
 * RPC response will have Corresponding partner's invocation id and UtdVector
 * Check if we have an entry for partner's invocation id in strong write partner content, if not create one
 * Find the entry in utd vector which has the initiator's USN and
 * update received USN to the strong write partner content
 */
VOID
VmDirReplUpdateUrgentReplCoordinatorTableForResponse(
    PVMDIR_REPL_UTDVECTOR pUtdVector,
    PCSTR pszRemoteServerInvocationId,
    PSTR pszRemoteServerName
    )
{
    BOOLEAN  bInLock = FALSE;
    DWORD    dwError = 0;
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT  pReplicationPartnerEntry = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    dwError = VmDirReplGetUrgentReplCoordinatorTableEntry_InLock(
                  pszRemoteServerInvocationId,
                  pszRemoteServerName,
                  &pReplicationPartnerEntry
                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pUtdVector != NULL)
    {
        VmDirReplUpdateUrgentReplCoordinatorTableForResponse_InLock(
            pUtdVector->pszPartnerInvocationId,
            pUtdVector->maxOriginatingUSN,
            pReplicationPartnerEntry
            );

        pUtdVector = pUtdVector->next;
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
        "VmDirReplUpdateUrgentReplCoordinatorTableForResponse: failed to update");
    goto cleanup;
}

DWORD
VmDirReplGetUrgentReplCoordinatorTableEntry_InLock(
    PCSTR pszRemoteServerInvocationId,
    PSTR  pszRemoteServerName,
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT *ppReplicationPartnerEntry
    )
{
    DWORD  dwError = 0;
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT pUrgentReplPartnerTable = NULL;

    pUrgentReplPartnerTable = gVmdirUrgentRepl.pUrgentReplPartnerTable;
    while (pUrgentReplPartnerTable != NULL)
    {
        if (VmDirStringCompareA(pUrgentReplPartnerTable->pInvocationId,
                                pszRemoteServerInvocationId,
                                TRUE) == 0)
        {
            break;
        }
        pUrgentReplPartnerTable = pUrgentReplPartnerTable->next;
    }

    if (pUrgentReplPartnerTable == NULL) // if no entry found allocate a new one
    {
        dwError = VmDirAllocateMemory(sizeof(VMDIR_STRONG_WRITE_PARTNER_CONTENT), (PVOID)&pUrgentReplPartnerTable);
        BAIL_ON_VMDIR_ERROR(dwError);

        //Initialize the new Entry
        pUrgentReplPartnerTable->pInvocationId = NULL;
        pUrgentReplPartnerTable->pServerName = NULL;
        pUrgentReplPartnerTable->isDeleted = FALSE;
        memset((PVOID)pUrgentReplPartnerTable->lastNotifiedTimeStamp, '\0', VMDIR_ORIG_TIME_STR_LEN);
        memset((PVOID)pUrgentReplPartnerTable->lastConfirmedTimeStamp,'\0', VMDIR_ORIG_TIME_STR_LEN);
        pUrgentReplPartnerTable->lastConfirmedUSN = 0;
        pUrgentReplPartnerTable->next = NULL;

        dwError = VmDirAllocateStringA(pszRemoteServerInvocationId, &pUrgentReplPartnerTable->pInvocationId);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pszRemoteServerName, &pUrgentReplPartnerTable->pServerName);
        BAIL_ON_VMDIR_ERROR(dwError);

        //Append to the list
        pUrgentReplPartnerTable->next = gVmdirUrgentRepl.pUrgentReplPartnerTable;
        gVmdirUrgentRepl.pUrgentReplPartnerTable = pUrgentReplPartnerTable;
    }

cleanup:
    *ppReplicationPartnerEntry = pUrgentReplPartnerTable;
    return dwError;

error:
    VmDirReplFreeUrgentReplPartnerEntry_InLock(pUrgentReplPartnerTable);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
        "VmDirReplUpdateUrgentReplCoordinatorTableForRequest: failed with status: %d", dwError);
    goto cleanup;
}

VOID
VmDirReplUpdateUrgentReplCoordinatorTableForResponse_InLock(
    CHAR* pInvocationId,
    USN   confirmedUSN,
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT pReplicationPartnerEntry
    )
{
    char   currentTimeStamp[VMDIR_ORIG_TIME_STR_LEN] = {0};

    /*
     * If VmDirGenOriginatingTimeStr fails, then lastConfirmedTimeStamp
     * could have just zeroes. Reason to override
     * the existing time stamp was to avoid confusion.
     */
    if (VmDirGenOriginatingTimeStr(currentTimeStamp) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
            "%s: failed to obtain time stamp", __func__);
    }

    if (VmDirStringCompareA(gVmdirServerGlobals.invocationId.lberbv.bv_val,
                            pInvocationId,
                            TRUE) == 0)
    {
        if (VmDirCopyMemory((PVOID)pReplicationPartnerEntry->lastConfirmedTimeStamp,
                            VMDIR_ORIG_TIME_STR_LEN,
                            (PVOID)currentTimeStamp,
                            VMDIR_ORIG_TIME_STR_LEN) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: failed to obtain time stamp", __func__);
        }

        pReplicationPartnerEntry->lastConfirmedUSN = confirmedUSN;
	VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
			"%s: hostname: %s USN: %lld ", __func__, pReplicationPartnerEntry->pServerName, confirmedUSN);
    }
}

VOID
VmDirReplUpdateUrgentReplCoordinatorTableForRequest(
    VOID
    )
{
    BOOLEAN  bInLock = FALSE;
    char     currentTimeStamp[VMDIR_ORIG_TIME_STR_LEN] = {0};
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT pUrgentReplPartnerTable = NULL;

    /*
     * If VmDirGenOriginatingTimeStr fails, then lastNotifiedTimeStamp
     * could have just zeroes. Reason to override
     * the existing time stamp was to avoid confusion.
     */
    if (VmDirGenOriginatingTimeStr(currentTimeStamp) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
            "VmDirReplUpdateUrgentReplCoordinatorTableForRequest: failed to obtain time stamp");
    }

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    pUrgentReplPartnerTable = gVmdirUrgentRepl.pUrgentReplPartnerTable;
    while (pUrgentReplPartnerTable != NULL)
    {
        if (VmDirCopyMemory((PVOID)pUrgentReplPartnerTable->lastNotifiedTimeStamp,
                            VMDIR_ORIG_TIME_STR_LEN,
                            (PVOID)currentTimeStamp,
                            VMDIR_ORIG_TIME_STR_LEN) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "VmDirReplUpdateUrgentReplCoordinatorTableForRequest: failed to obtain time stamp");
        }

        pUrgentReplPartnerTable = pUrgentReplPartnerTable->next;
    }

    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return;
}

VOID
VmDirReplUpdateUrgentReplCoordinatorTableForDelete(
    PVMDIR_REPLICATION_AGREEMENT  pReplAgr
    )
{
    DWORD     dwError = 0;
    BOOLEAN   bInLock = FALSE;
    PSTR      pszPartnerHostName = NULL;
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT   pUrgentReplPartnerTable = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    pUrgentReplPartnerTable = gVmdirUrgentRepl.pUrgentReplPartnerTable;
    while (pUrgentReplPartnerTable != NULL)
    {
        dwError = VmDirReplURIToHostname(pReplAgr->ldapURI, &pszPartnerHostName);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (VmDirStringCompareA(pUrgentReplPartnerTable->pServerName,
                                pszPartnerHostName,
                                FALSE) == 0) //hostname not case sensitive
        {
            pUrgentReplPartnerTable->isDeleted = TRUE;
            break;
        }

        VMDIR_SAFE_FREE_MEMORY(pszPartnerHostName);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    VMDIR_SAFE_FREE_MEMORY(pszPartnerHostName);
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
        "%s: failed %d", __func__, dwError);
    goto cleanup;
}

VOID
VmDirReplFreeUrgentReplCoordinatorTable(
    VOID
    )
{
    BOOLEAN    bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    VmDirReplFreeUrgentReplCoordinatorTable_InLock();
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return;
}

VOID
VmDirReplFreeUrgentReplCoordinatorTable_InLock(
    VOID
    )
{
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT pUrgentReplPartnerTable = NULL;
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT pTempUrgentReplPartnerTable = NULL;

    pUrgentReplPartnerTable = gVmdirUrgentRepl.pUrgentReplPartnerTable;
    while (pUrgentReplPartnerTable != NULL)
    {
        pTempUrgentReplPartnerTable = pUrgentReplPartnerTable;
        pUrgentReplPartnerTable = pUrgentReplPartnerTable->next;

        VmDirReplFreeUrgentReplPartnerEntry_InLock(pTempUrgentReplPartnerTable);
    }
    gVmdirUrgentRepl.pUrgentReplPartnerTable = NULL;
}

VOID
VmDirReplFreeUrgentReplPartnerEntry_InLock(
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT pUrgentReplPartnerTable
    )
{
    if (pUrgentReplPartnerTable)
    {
        VMDIR_SAFE_FREE_MEMORY(pUrgentReplPartnerTable->pInvocationId);
        VMDIR_SAFE_FREE_MEMORY(pUrgentReplPartnerTable->pServerName);
    }
    VMDIR_SAFE_FREE_MEMORY(pUrgentReplPartnerTable);

    return;
}

DWORD
VmDirReplGetUrgentReplResponseCount(
    VOID
    )
{
    BOOLEAN   bInLock = FALSE;
    DWORD    dwUrgentReplResponseCount = 0;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    dwUrgentReplResponseCount = gVmdirUrgentRepl.dwUrgentReplResponseCount;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return dwUrgentReplResponseCount;
}

VOID
VmDirReplUpdateUrgentReplResponseCount(
    VOID
    )
{
    BOOLEAN   bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    gVmdirUrgentRepl.dwUrgentReplResponseCount++;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return;
}

VOID
VmDirReplResetUrgentReplResponseCount(
    VOID
    )
{
    BOOLEAN   bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    VmDirReplResetUrgentReplResponseCount_InLock();
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return;
}

VOID
VmDirReplResetUrgentReplResponseCount_InLock(
    VOID
    )
{
    gVmdirUrgentRepl.dwUrgentReplResponseCount = 0;
}

VOID
VmDirReplSetUrgentReplResponseRecvCondition(
    BOOLEAN bUrgentReplResponseRecv
    )
{
    BOOLEAN   bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    gVmdirUrgentRepl.bUrgentReplResponseRecv = bUrgentReplResponseRecv;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return;
}

BOOLEAN
VmDirReplGetUrgentReplResponseRecvCondition(
    VOID
    )
{
    BOOLEAN   bInLock = FALSE;
    BOOLEAN   bUrgentReplResponseRecv = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    bUrgentReplResponseRecv = gVmdirUrgentRepl.bUrgentReplResponseRecv;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return bUrgentReplResponseRecv;
}

VOID
VmDirReplSetUrgentReplThreadCondition(
    BOOLEAN bUrgentReplThreadPredicate
    )
{
    BOOLEAN   bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    gVmdirUrgentRepl.bUrgentReplThreadPredicate = bUrgentReplThreadPredicate;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return;
}

BOOLEAN
VmDirReplGetUrgentReplThreadCondition(
    VOID
    )
{
    BOOLEAN   bInLock = FALSE;
    BOOLEAN   bUrgentReplThreadPredicate = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    bUrgentReplThreadPredicate = gVmdirUrgentRepl.bUrgentReplThreadPredicate;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return bUrgentReplThreadPredicate;
}

PVMDIR_STRONG_WRITE_PARTNER_CONTENT
VmDirReplGetUrgentReplCoordinatorTable(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT pUrgentReplPartnerTable = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    pUrgentReplPartnerTable = VmDirReplGetUrgentReplCoordinatorTable_InLock();
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return pUrgentReplPartnerTable;
}

PVMDIR_STRONG_WRITE_PARTNER_CONTENT
VmDirReplGetUrgentReplCoordinatorTable_InLock(
    VOID
    )
{
    return gVmdirUrgentRepl.pUrgentReplPartnerTable;
}

VOID
VmDirReplSetUrgentReplDoneCondition(
    BOOLEAN bUrgentReplDone
    )
{
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    VmDirReplSetUrgentReplDoneCondition_InLock(bUrgentReplDone);
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return;
}

VOID
VmDirReplSetUrgentReplDoneCondition_InLock(
    BOOLEAN bUrgentReplDone
    )
{
    gVmdirUrgentRepl.bUrgentReplDone = bUrgentReplDone;
}

BOOLEAN
VmDirReplGetUrgentReplDoneCondition(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;
    BOOLEAN bUrgentReplDone = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    bUrgentReplDone = VmDirReplGetUrgentReplDoneCondition_InLock();
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return bUrgentReplDone;
}

BOOLEAN
VmDirReplGetUrgentReplDoneCondition_InLock(
    VOID
    )
{
    return gVmdirUrgentRepl.bUrgentReplDone;
}

USN
VmDirGetUrgentReplConsensus(
    VOID
    )
{
    USN      consensusUSN = 0;
    BOOLEAN  bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    consensusUSN = VmDirGetUrgentReplConsensus_InLock();
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return consensusUSN;
}

USN
VmDirGetUrgentReplConsensus_InLock(
    VOID
    )
{
    return gVmdirUrgentRepl.consensusUSN;
}

BOOLEAN
VmDirUrgentReplUpdateConsensus(
    VOID
    )
{
    USN      minUSN = 0;
    BOOLEAN  bInLock = FALSE;
    BOOLEAN  bUpdated = FALSE;
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT pUrgentReplPartnerTable = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    pUrgentReplPartnerTable = VmDirReplGetUrgentReplCoordinatorTable_InLock();

    if (pUrgentReplPartnerTable != NULL)
    {
        minUSN = pUrgentReplPartnerTable->lastConfirmedUSN;

        while (pUrgentReplPartnerTable != NULL)
        {
            if (pUrgentReplPartnerTable->isDeleted == FALSE &&
                minUSN > pUrgentReplPartnerTable->lastConfirmedUSN)
            {
                minUSN = pUrgentReplPartnerTable->lastConfirmedUSN;
            }
            pUrgentReplPartnerTable = pUrgentReplPartnerTable->next;
        }

        if (gVmdirUrgentRepl.consensusUSN < minUSN)
        {
            gVmdirUrgentRepl.consensusUSN = minUSN;
            bUpdated = TRUE;
        }
        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
		        "%s: minUSN: %lld gVmdirUrgentRepl.ConsensusUSN: %lld",
                         __func__, minUSN, gVmdirUrgentRepl.consensusUSN);
    }

    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return bUpdated;
}

DWORD
VmDirGetUrgentReplTimeout(
    VOID
    )
{
    BOOLEAN  bInLock = FALSE;
    DWORD    dwUrgentReplTimeout = 0;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    dwUrgentReplTimeout = VmDirGetUrgentReplTimeout_InLock();
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return dwUrgentReplTimeout;
}

DWORD
VmDirGetUrgentReplTimeout_InLock(
    VOID
    )
{
    return gVmdirUrgentRepl.dwUrgentReplTimeout;
}

VOID
VmDirSetUrgentReplTimeout(
    DWORD dwUrgentReplTimeout
    )
{
    BOOLEAN  bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    VmDirSetUrgentReplTimeout_InLock(dwUrgentReplTimeout);
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return;
}

VOID
VmDirSetUrgentReplTimeout_InLock(
    DWORD dwUrgentReplTimeout
    )
{
    gVmdirUrgentRepl.dwUrgentReplTimeout = dwUrgentReplTimeout;
}

BOOLEAN
VmDirGetUrgentReplicationPending(
    VOID
    )
{
    BOOLEAN  bInLock = FALSE;
    BOOLEAN  bUrgentReplicationPending = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    bUrgentReplicationPending = VmDirGetUrgentReplicationPending_InLock();
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return bUrgentReplicationPending;
}

BOOLEAN
VmDirGetUrgentReplicationPending_InLock(
    VOID
    )
{
    return gVmdirUrgentRepl.bUrgentReplicationPending;
}

VOID
VmDirSetUrgentReplicationPending(
    BOOLEAN bUrgentReplicationPending
    )
{
    BOOLEAN  bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);
    VmDirSetUrgentReplicationPending_InLock(bUrgentReplicationPending);
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirUrgentRepl.pUrgentReplMutex);

    return;
}

VOID
VmDirSetUrgentReplicationPending_InLock(
    BOOLEAN bUrgentReplicationPending
    )
{
    gVmdirUrgentRepl.bUrgentReplicationPending = bUrgentReplicationPending;
}


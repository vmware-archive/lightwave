/*
* Copyright � 2018 VMware, Inc.  All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the �License�); you may not
* use this file except in compliance with the License.  You may obtain a copy
* of the License at http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an �AS IS� BASIS, without
* warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
* License for the specific language governing permissions and limitations
* under the License.
*/

#include "includes.h"

static
DWORD
VmDnsForwarderPacketListClear(
    PVMDNS_FORWARDER_PACKET_LIST pForwarderPacketList
    );

DWORD
VmDnsForwarderPacketEntryCreate(
    PVMDNS_FORWARDER_PACKET_CONTEXT pForwarderPacketContext,
    PVM_SOCKET                      pSocket,
    UINT64                          uiExpirationTime,
    PVMDNS_FORWARDER_PACKET_ENTRY   *ppForwarderPacketEntry
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_FORWARDER_PACKET_ENTRY  pForwarderPacketEntry = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppForwarderPacketEntry, dwError);

    dwError = VmDnsAllocateMemory(
                    sizeof(VMDNS_FORWARDER_PACKET_ENTRY),
                    (VOID*)&pForwarderPacketEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    pForwarderPacketEntry->lRefCount = 1;

    pForwarderPacketEntry->pForwarderPacketContext = VmDnsAcquireForwarderPacketContext(pForwarderPacketContext);
    pForwarderPacketEntry->uiExpirationTime = uiExpirationTime;

    pForwarderPacketEntry->pSocket = VmDnsSockAcquire(pSocket);

    *ppForwarderPacketEntry = pForwarderPacketEntry;

cleanup:
    return dwError;

error:
    VmDnsForwarderPacketEntryRelease(pForwarderPacketEntry);
    goto cleanup;
}

PVMDNS_FORWARDER_PACKET_ENTRY
VmDnsForwarderPacketEntryAcquire(
    PVMDNS_FORWARDER_PACKET_ENTRY  pForwarderPacketEntry
    )
{
    if (pForwarderPacketEntry)
    {
        InterlockedIncrement(&pForwarderPacketEntry->lRefCount);
    }
    return pForwarderPacketEntry;
}

VOID
VmDnsForwarderPacketEntryRelease(
    PVMDNS_FORWARDER_PACKET_ENTRY   pForwarderPacketEntry
    )
{
    if (pForwarderPacketEntry && InterlockedDecrement(&pForwarderPacketEntry->lRefCount) == 0)
    {
        VmDnsForwarderPacketEntryDelete(pForwarderPacketEntry);
    }
}

VOID
VmDnsForwarderPacketEntryDelete(
    PVMDNS_FORWARDER_PACKET_ENTRY   pForwarderPacketEntry
    )
{
    if (pForwarderPacketEntry)
    {
        if (pForwarderPacketEntry->pSocket)
        {
            VmDnsSockRelease(pForwarderPacketEntry->pSocket);
        }
        VmDnsFreeMemory(pForwarderPacketEntry);
    }
}

DWORD
VmDnsForwarderPacketListInitialize(
    PVMDNS_FORWARDER_PACKET_LIST* ppForwarderPacketList
    )
{
    DWORD dwError = 0;
    PVMDNS_FORWARDER_PACKET_LIST pForwarderPacketListTemp = NULL;

    if (!ppForwarderPacketList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_FORWARDER_PACKET_LIST),
                        (PVOID*)&pForwarderPacketListTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    pForwarderPacketListTemp->dwCurrentCount = 0;

    dwError = VmDnsAllocateMutex(&pForwarderPacketListTemp->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);

    InitializeListHead(&pForwarderPacketListTemp->ForwarderPacketListHead);

    *ppForwarderPacketList = pForwarderPacketListTemp;

cleanup:

    return dwError;

error:

    if (ppForwarderPacketList)
    {
        *ppForwarderPacketList = NULL;
    }

    VmDnsForwarderPacketListFree(pForwarderPacketListTemp);
    goto cleanup;
}

VOID
VmDnsForwarderPacketListFree(
    PVMDNS_FORWARDER_PACKET_LIST pForwarderPacketList
    )
{
    if (pForwarderPacketList)
    {
        if (!IsListEmpty(&pForwarderPacketList->ForwarderPacketListHead))
        {
            assert(0 == VmDnsForwarderPacketListClear(pForwarderPacketList));
        }

        VmDnsFreeMutex(pForwarderPacketList->pLock);
        VmDnsFreeMemory(pForwarderPacketList);
    }
}

DWORD
VmDnsForwarderPacketListAddEntry(
    PVMDNS_FORWARDER_PACKET_LIST pForwarderPacketList,
    PVMDNS_FORWARDER_PACKET_ENTRY pForwarderPacketEntry
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    assert(pForwarderPacketList);
    assert(pForwarderPacketEntry);
    assert(pForwarderPacketList->pLock);

    dwError = VmDnsLockMutex(pForwarderPacketList->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);
    bLocked = TRUE;

    InsertHeadList(&pForwarderPacketList->ForwarderPacketListHead, &pForwarderPacketEntry->ForwarderPacketList);

    VmDnsForwarderPacketEntryAcquire(pForwarderPacketEntry);

    ++pForwarderPacketList->dwCurrentCount;

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pForwarderPacketList->pLock);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsForwarderPacketListRemoveEntry(
    PVMDNS_FORWARDER_PACKET_LIST pForwarderPacketList,
    PVMDNS_FORWARDER_PACKET_ENTRY pForwarderPacketEntry
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    assert(pForwarderPacketList);
    assert(pForwarderPacketEntry);
    assert(pForwarderPacketList->pLock);

    dwError = VmDnsLockMutex(pForwarderPacketList->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);
    bLocked = TRUE;

    RemoveEntryList(&pForwarderPacketEntry->ForwarderPacketList);
    --pForwarderPacketList->dwCurrentCount;
    VmDnsForwarderPacketEntryRelease(pForwarderPacketEntry);

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pForwarderPacketList->pLock);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsForwarderPacketListClearEntries(
    PVMDNS_FORWARDER_PACKET_LIST pForwarderPacketList,
    DWORD dwCount
    )
{
    DWORD dwError = 0;
    PVMDNS_FORWARDER_PACKET_ENTRY pForwarderPacketEntry = NULL;
    PLIST_ENTRY pLink = NULL;

    for (pLink = pForwarderPacketList->ForwarderPacketListHead.Blink;
        (pLink != &pForwarderPacketList->ForwarderPacketListHead && dwCount > 0);
        pLink = pLink->Blink)
    {
        pForwarderPacketEntry = CONTAINING_RECORD(pLink, VMDNS_FORWARDER_PACKET_ENTRY, ForwarderPacketList);

        RemoveEntryList(&pForwarderPacketEntry->ForwarderPacketList);
        --pForwarderPacketList->dwCurrentCount;

        VmDnsForwarderPacketEntryRelease(pForwarderPacketEntry);

        --dwCount;
    }

    return dwError;
}

static
DWORD
VmDnsForwarderPacketListClear(
    PVMDNS_FORWARDER_PACKET_LIST pForwarderPacketList)
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    assert(pForwarderPacketList);
    assert(pForwarderPacketList->pLock);

    dwError = VmDnsLockMutex(pForwarderPacketList->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);
    bLocked = TRUE;

    while (!IsListEmpty(&pForwarderPacketList->ForwarderPacketListHead))
    {
        RemoveEntryList((&pForwarderPacketList->ForwarderPacketListHead)->Flink)
        pForwarderPacketList->dwCurrentCount--;
    }

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pForwarderPacketList->pLock);
    }

    return dwError;

error:
    goto cleanup;
}

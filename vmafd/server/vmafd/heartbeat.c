/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : heartbeat.c
 *
 * Abstract :
 *
 */
#include "includes.h"

static VMAFD_HB_TABLE gHeartbeatTable = {0};
static pthread_rwlock_t rwlockHeartbeatTable =
                        (pthread_rwlock_t)PTHREAD_RWLOCK_INITIALIZER;

static
DWORD
VmAfdFindNode(
    PCWSTR pwszServiceName,
    DWORD  dwPort,
    PVMAFD_HB_NODE *ppNode
    );

static
DWORD
VmAfdInsertNode(
    PCWSTR pwszServiceName,
    DWORD  dwPort,
    PVMAFD_HB_NODE *ppNode
    );

static
BOOL
VmAfdCheckServiceStatus(
    PVMAFD_HB_NODE pNode
    );

static
VOID
VmAfdUpdateAvailableIndex(
    VOID
    );

static
DWORD
VmAfdGetHeartbeatInfo(
    PVMAFD_HB_INFO_W *ppHeartbeatInfoArr,
    PDWORD pdwEntriesCount,
    PBOOL pbNodeIsAlive
    );

DWORD
VmAfSrvInitHeartbeatTable(
    VOID
    )
{
    DWORD dwError = 0;
    WCHAR heartbeatEntryNames[][32] = VMAFD_HEARTBEAT_ENTRY_NAMES_W;
    DWORD dwHeartbeatPorts[] = VMAFD_HEARTBEAT_ENTRY_PORTS;
    DWORD dwEntriesCount = 0;
    DWORD dwIndex = 0;

    dwEntriesCount = sizeof(dwHeartbeatPorts) / sizeof(dwHeartbeatPorts[0]);

    for (; dwIndex < dwEntriesCount; ++dwIndex)
    {
        PVMAFD_HB_NODE pNodeToInsert = NULL;
        dwError = VmAfdFindNode(
                            heartbeatEntryNames[dwIndex],
                            dwHeartbeatPorts[dwIndex],
                            &pNodeToInsert
                            );
        if (dwError == ERROR_OBJECT_NOT_FOUND)
        {
            dwError = VmAfdInsertNode(
                                heartbeatEntryNames[dwIndex],
                                dwHeartbeatPorts[dwIndex],
                                &pNodeToInsert
                                );
        }

        pNodeToInsert->tLastPing = VmAfdGetTickCount();

        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfSrvPostHeartbeat(
    PCWSTR pwszServiceName,
    DWORD  dwPort
    )
{
    DWORD dwError  = 0;
    PVMAFD_HB_NODE pNode = NULL;
    BOOL bIsHoldingLock = FALSE;

    if (IsNullOrEmptyString(pwszServiceName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdFindNode(
                        pwszServiceName,
                        dwPort,
                        &pNode
                        );
    if (dwError == ERROR_OBJECT_NOT_FOUND)
    {
        dwError = VmAfdInsertNode(
                            pwszServiceName,
                            dwPort,
                            &pNode
                            );
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    VMAFD_LOCK_MUTEX_EXCLUSIVE(&rwlockHeartbeatTable, bIsHoldingLock);

    pNode->tLastPing = VmAfdGetTickCount();

    VMAFD_LOCK_MUTEX_UNLOCK(&rwlockHeartbeatTable, bIsHoldingLock);

cleanup:

    VMAFD_LOCK_MUTEX_UNLOCK(&rwlockHeartbeatTable, bIsHoldingLock);
    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfSrvGetHeartbeatStatus(
    PVMAFD_HB_STATUS_W* ppHeartbeatStatus
    )
{
    DWORD dwError = 0;
    PVMAFD_HB_STATUS_W pHeartbeatStatus = NULL;

    BOOL bIsHoldingLock = FALSE;

    DWORD dwEntriesCount = 0;

    if (!ppHeartbeatStatus)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                            sizeof(VMAFD_HB_STATUS_W),
                            (PVOID *)&pHeartbeatStatus
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    pHeartbeatStatus->bIsAlive = TRUE;

    VMAFD_LOCK_MUTEX_SHARED(&rwlockHeartbeatTable, bIsHoldingLock);

    dwError = VmAfdGetHeartbeatInfo(
                              &pHeartbeatStatus->pHeartbeatInfoArr,
                              &dwEntriesCount,
                              &pHeartbeatStatus->bIsAlive
                              );
    BAIL_ON_VMAFD_ERROR(dwError);


    VMAFD_LOCK_MUTEX_UNLOCK(&rwlockHeartbeatTable, bIsHoldingLock);

    pHeartbeatStatus->dwCount = dwEntriesCount;

    *ppHeartbeatStatus = pHeartbeatStatus;


cleanup:

    return dwError;
error:

    if (ppHeartbeatStatus)
    {
        *ppHeartbeatStatus = NULL;
    }

    if (pHeartbeatStatus)
    {
        VmAfdFreeHbStatusW(pHeartbeatStatus);
    }

    goto cleanup;
}

VOID
VmAfdFreeHbNode(
    PVMAFD_HB_NODE pNode
    )
{
    if (pNode)
    {
        VMAFD_SAFE_FREE_MEMORY(pNode->pszServiceName);
        VMAFD_SAFE_FREE_MEMORY(pNode);
    }
}

VOID
VmAfdHeartbeatCleanup(
    VOID
    )
{
    DWORD dwIndex = 0;

    for (; dwIndex < VMAFD_HEARTBEAT_TABLE_COUNT; ++dwIndex)
    {
        if (gHeartbeatTable.pEntries[dwIndex])
        {
            VmAfdFreeHbNode(gHeartbeatTable.pEntries[dwIndex]);
            gHeartbeatTable.pEntries[dwIndex] = NULL;
        }
    }
}

DWORD
VmAfdRpcAllocateHeartbeatStatus(
    PVMAFD_HB_STATUS_W  pHeartbeatStatus,
    PVMAFD_HB_STATUS_W  *ppHeartbeatStatus
    )
{
    DWORD dwError = 0;
    PVMAFD_HB_STATUS_W pHeartbeatStatusDest = NULL;

    dwError = VmAfdRpcServerAllocateMemory(
                    sizeof(VMAFD_HB_STATUS_W),
                    (PVOID*)&pHeartbeatStatusDest
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    pHeartbeatStatusDest->bIsAlive = pHeartbeatStatus->bIsAlive;

    if (pHeartbeatStatus->dwCount > 0)
    {
        DWORD dwIndex = 0;

        dwError = VmAfdRpcServerAllocateMemory(
                        sizeof(VMAFD_HB_INFO_W) * pHeartbeatStatus->dwCount,
                        (PVOID*)&pHeartbeatStatusDest->pHeartbeatInfoArr
                        );
        BAIL_ON_VMAFD_ERROR(dwError);

        pHeartbeatStatusDest->dwCount = pHeartbeatStatus->dwCount;

        for (; dwIndex < pHeartbeatStatus->dwCount; ++dwIndex)
        {
            PVMAFD_HB_INFO_W pSrc =
                        &pHeartbeatStatus->pHeartbeatInfoArr[dwIndex];
            PVMAFD_HB_INFO_W pDst=
                        &pHeartbeatStatusDest->pHeartbeatInfoArr[dwIndex];

            pDst->dwPort = pSrc->dwPort;
            pDst->dwLastHeartbeat = pSrc->dwLastHeartbeat;
            pDst->bIsAlive = pSrc->bIsAlive;

            if (pSrc->pszServiceName)
            {
                dwError = VmAfdRpcServerAllocateStringW(
                                      pSrc->pszServiceName,
                                      &pDst->pszServiceName
                                      );
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        }
    }

    *ppHeartbeatStatus = pHeartbeatStatusDest;

cleanup:

    return dwError;
error:

    if (ppHeartbeatStatus)
    {
        *ppHeartbeatStatus = NULL;
    }
    if (pHeartbeatStatusDest)
    {
        VmAfdRpcFreeHeartbeatStatus(pHeartbeatStatusDest);
    }
    goto cleanup;
}

VOID
VmAfdRpcFreeHeartbeatStatus(
    PVMAFD_HB_STATUS_W pHeartbeatStatus
    )
{
    if (pHeartbeatStatus)
    {
        if (pHeartbeatStatus->pHeartbeatInfoArr)
        {
            DWORD dwIndex = 0;

            for (; dwIndex < pHeartbeatStatus->dwCount; ++dwIndex)
            {
                PVMAFD_HB_INFO_W pCursor=
                              &pHeartbeatStatus->pHeartbeatInfoArr[dwIndex];

                if (pCursor->pszServiceName)
                {
                    VmAfdRpcServerFreeMemory(pCursor->pszServiceName);
                }
            }

            VmAfdRpcServerFreeMemory(pHeartbeatStatus->pHeartbeatInfoArr);
        }

        VmAfdRpcServerFreeMemory(pHeartbeatStatus);
    }
}

static
DWORD
VmAfdFindNode(
    PCWSTR pwszServiceName,
    DWORD  dwPort,
    PVMAFD_HB_NODE *ppNode
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVMAFD_HB_NODE pNode = NULL;
    BOOL bIsHoldingLock = FALSE;

    VMAFD_LOCK_MUTEX_SHARED(&rwlockHeartbeatTable, bIsHoldingLock);

    for (;dwIndex<VMAFD_HEARTBEAT_TABLE_COUNT; ++dwIndex)
    {
        if (gHeartbeatTable.pEntries[dwIndex] &&
            VmAfdStringIsEqualW(
                         pwszServiceName,
                         gHeartbeatTable.pEntries[dwIndex]->pszServiceName,
                         FALSE
                         ) &&
            gHeartbeatTable.pEntries[dwIndex]->dwPort == dwPort
           )
        {
            pNode = gHeartbeatTable.pEntries[dwIndex];
            break;
        }
    }

    VMAFD_LOCK_MUTEX_UNLOCK(&rwlockHeartbeatTable, bIsHoldingLock);

    if (!pNode)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);
    }

    *ppNode = pNode;

cleanup:

    VMAFD_LOCK_MUTEX_UNLOCK(&rwlockHeartbeatTable, bIsHoldingLock);
    return dwError;
error:

    if (ppNode)
    {
        *ppNode = NULL;
    }
    goto cleanup;
}

static
DWORD
VmAfdInsertNode(
    PCWSTR pwszServiceName,
    DWORD  dwPort,
    PVMAFD_HB_NODE *ppNode
    )
{
    DWORD dwError = 0;
    PVMAFD_HB_NODE pNode = NULL;
    BOOL bIsHoldingLock = FALSE;

    if (IsNullOrEmptyString(pwszServiceName) ||
        !ppNode
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                            sizeof(VMAFD_HB_NODE),
                            (PVOID *)&pNode
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringW(
                            pwszServiceName,
                            &pNode->pszServiceName
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    pNode->dwPort = dwPort;

    *ppNode = pNode;

    VMAFD_LOCK_MUTEX_EXCLUSIVE(&rwlockHeartbeatTable, bIsHoldingLock);

    if (gHeartbeatTable.dwNextAvailableIndex == VMAFD_HEARTBEAT_TABLE_COUNT)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    gHeartbeatTable.pEntries[gHeartbeatTable.dwNextAvailableIndex] = pNode;

    VmAfdUpdateAvailableIndex();

    VMAFD_LOCK_MUTEX_UNLOCK(&rwlockHeartbeatTable, bIsHoldingLock);

cleanup:

    VMAFD_LOCK_MUTEX_UNLOCK(&rwlockHeartbeatTable, bIsHoldingLock);
    return dwError;
error:

    if (ppNode)
    {
        *ppNode = NULL;
    }
    if (pNode)
    {
        VmAfdFreeHbNode(pNode);
    }
    goto cleanup;
}

static
BOOL
VmAfdCheckServiceStatus(
    PVMAFD_HB_NODE pNode
    )
{
    BOOL bIsAlive = TRUE;
    DWORD dwHeartbeatInterval = 0;


    if (VmAfdRegGetInteger(
            VMAFD_REG_KEY_HEARTBEAT,
            &dwHeartbeatInterval) != 0
       )
    {
        dwHeartbeatInterval = VMAFD_HEARTBEAT_INTERVAL;
    }

    if (pNode->tLastPing < VmAfdGetTickCount()-(3*dwHeartbeatInterval))
    {
        bIsAlive = FALSE;
    }

    return bIsAlive;
}

static
VOID
VmAfdUpdateAvailableIndex(
    VOID
    )
{
    DWORD dwNextAvailableIndex = gHeartbeatTable.dwNextAvailableIndex + 1;
    BOOL  bNextIndexSet = FALSE;

    do
    {
        dwNextAvailableIndex =
                        dwNextAvailableIndex % VMAFD_HEARTBEAT_TABLE_COUNT;

        if (!gHeartbeatTable.pEntries[dwNextAvailableIndex])
        {
            bNextIndexSet = TRUE;
            break;
        }

        ++dwNextAvailableIndex;
    } while (dwNextAvailableIndex != gHeartbeatTable.dwNextAvailableIndex);

    if (!bNextIndexSet)
    {
        dwNextAvailableIndex = VMAFD_HEARTBEAT_TABLE_COUNT;
    }

    gHeartbeatTable.dwNextAvailableIndex = dwNextAvailableIndex;

    return;
}

static
DWORD
VmAfdGetHeartbeatInfo(
    PVMAFD_HB_INFO_W *ppHeartbeatInfoArr,
    PDWORD pdwEntriesCount,
    PBOOL pbNodeIsAlive
    )
{
    DWORD dwError = 0;
    DWORD dwEntriesCount = 0;
    PVMAFD_HB_INFO_W pHeartbeatInfoArr = NULL;
    BOOL bNodeIsAlive = TRUE;
    BOOL bServiceIsAlive = FALSE;
    DWORD dwEntriesIndex = 0;

    if (!ppHeartbeatInfoArr || !pdwEntriesCount || !pbNodeIsAlive)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwEntriesCount = gHeartbeatTable.dwNextAvailableIndex;

    if (dwEntriesCount)
    {
        DWORD dwIndex = 0;

        dwError = VmAfdAllocateMemory(
                                sizeof(VMAFD_HB_INFO_W)*dwEntriesCount,
                                (PVOID*)&pHeartbeatInfoArr
                                );
        BAIL_ON_VMAFD_ERROR(dwError);

        for ( dwIndex = 0;
              dwIndex < dwEntriesCount;
              ++dwIndex
            )
        {
            PVMAFD_HB_NODE pCurrentNode = gHeartbeatTable.pEntries[dwIndex];
            PVMAFD_HB_INFO_W pCurrentEntry = &pHeartbeatInfoArr[dwEntriesIndex];
            if (pCurrentNode)
            {
                dwError = VmAfdAllocateStringW(
                                    pCurrentNode->pszServiceName,
                                    &pCurrentEntry->pszServiceName
                                    );
                BAIL_ON_VMAFD_ERROR(dwError);

                bServiceIsAlive = VmAfdCheckServiceStatus(pCurrentNode);

                bNodeIsAlive = bNodeIsAlive && bServiceIsAlive;

                pCurrentEntry->bIsAlive = bServiceIsAlive? TRUE: FALSE;

                pCurrentEntry->dwLastHeartbeat = (DWORD)pCurrentNode->tLastPing;

                pCurrentEntry->dwPort = pCurrentNode->dwPort;

                ++dwEntriesIndex;
            }
        }
    }

    *pdwEntriesCount = dwEntriesIndex;
    *ppHeartbeatInfoArr = pHeartbeatInfoArr;
    *pbNodeIsAlive = bNodeIsAlive;

cleanup:

    return dwError;
error:

    if (ppHeartbeatInfoArr)
    {
        *ppHeartbeatInfoArr = NULL;
    }
    if (pdwEntriesCount)
    {
        *pdwEntriesCount = 0;
    }
    if (pbNodeIsAlive)
    {
        *pbNodeIsAlive = FALSE;
    }
    if (pHeartbeatInfoArr)
    {
        VmAfdFreeHbInfoArrayW(pHeartbeatInfoArr,dwEntriesCount);
    }
    goto cleanup;
}


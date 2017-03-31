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
VmDirAllocateSyncCounter(
    PVMDIR_SYNCHRONIZE_COUNTER*     ppSyncCounter,
    size_t                          iSyncValue,
    VMDIR_SYNC_MECHANISM            wakeupMethod,
    DWORD                           iCondWaitTimeInMilliSec
    )
{
    DWORD                       dwError = ERROR_SUCCESS;
    PVMDIR_SYNCHRONIZE_COUNTER  pSyncCounter = NULL;

    if ( ppSyncCounter == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory( sizeof(*pSyncCounter), ((PVOID*)&pSyncCounter));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitializeSynchCounterContent( pSyncCounter );
    BAIL_ON_VMDIR_ERROR(dwError);

    pSyncCounter->iSyncValue = iSyncValue;
    pSyncCounter->wakeupMethod = wakeupMethod;
    pSyncCounter->iCondWaitTimeInMilliSec = iCondWaitTimeInMilliSec;

    *ppSyncCounter = pSyncCounter;
    pSyncCounter = NULL;

error:

    VMDIR_SAFE_FREE_SYNCCOUNTER( pSyncCounter );

    return dwError;
}

DWORD
VmDirInitializeSynchCounterContent(
    PVMDIR_SYNCHRONIZE_COUNTER     pSyncCounter
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pSyncCounter == NULL ) || ( pSyncCounter->bInitialized != FALSE ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMutex(&pSyncCounter->pMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&pSyncCounter->pCond);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSyncCounter->bInitialized = TRUE;

error:

    return dwError;
}

VOID
VmDirFreeSyncCounter(
    PVMDIR_SYNCHRONIZE_COUNTER     pSyncCounter
    )
{
    VMDIR_SAFE_FREE_MUTEX( pSyncCounter->pMutex );
    VMDIR_SAFE_FREE_CONDITION( pSyncCounter->pCond );
    VMDIR_SAFE_FREE_MEMORY( pSyncCounter );
}

/*
 * wait for event or timeout
 * *pbWaitTimeOut is set based on timeout result.
 */
DWORD
VmDirSyncCounterWaitEvent(
    PVMDIR_SYNCHRONIZE_COUNTER     pSyncCounter,
    PBOOLEAN                       pbWaitTimeOut        // optional
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bInLock = FALSE;
    BOOLEAN     bWaitTimeOut = FALSE;

    if ( ( pSyncCounter == NULL )
         ||
         ( pSyncCounter->bInitialized == FALSE )
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bInLock, pSyncCounter->pMutex);

    while ( pSyncCounter->iCounter != pSyncCounter->iSyncValue
            &&
            bWaitTimeOut == FALSE
           )
    {
        VmDirLog( LDAP_DEBUG_TRACE, "Sync counter wait event - (%ld)(%ld)",
                pSyncCounter->iCounter, pSyncCounter->iSyncValue );

        if (pSyncCounter->iCondWaitTimeInMilliSec > 0)
        {
            dwError = VmDirConditionTimedWait(
                            pSyncCounter->pCond,
                            pSyncCounter->pMutex,
                            pSyncCounter->iCondWaitTimeInMilliSec
                            );
            if (dwError == ETIMEDOUT)
            {
                VmDirLog( LDAP_DEBUG_TRACE, "Sync counter wait event timeout");

                bWaitTimeOut = TRUE;
                dwError = 0;
            }
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirConditionWait(pSyncCounter->pCond, pSyncCounter->pMutex);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    VMDIR_UNLOCK_MUTEX(bInLock, pSyncCounter->pMutex);

    if (pbWaitTimeOut != NULL)
    {
        *pbWaitTimeOut = bWaitTimeOut;
    }

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, pSyncCounter->pMutex);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirSyncCounterChange(
    PVMDIR_SYNCHRONIZE_COUNTER     pSyncCounter,
    int                            iValue
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    BOOLEAN bInLock = FALSE;
    BOOLEAN bRevertChange = FALSE;

    if ( ( pSyncCounter == NULL )
         ||
         ( pSyncCounter->bInitialized == FALSE )
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bInLock, pSyncCounter->pMutex);

    pSyncCounter->iCounter += iValue;
    bRevertChange = TRUE;

    if (pSyncCounter->iCounter == pSyncCounter->iSyncValue)
    {
        switch (pSyncCounter->wakeupMethod)
        {
        case SYNC_SIGNAL:
            dwError = VmDirConditionSignal(pSyncCounter->pCond);
            BAIL_ON_VMDIR_ERROR(dwError);
            break;

        case SYNC_BROADCAST:    //TODO, not yet supported
        default:
            assert(FALSE);
        }
    }

    VMDIR_UNLOCK_MUTEX(bInLock, pSyncCounter->pMutex);

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, pSyncCounter->pMutex);

    return dwError;

error:

    if (bRevertChange)
    {
        VMDIR_LOCK_MUTEX(bInLock, pSyncCounter->pMutex);
        pSyncCounter->iCounter -= iValue;
        VMDIR_UNLOCK_MUTEX(bInLock, pSyncCounter->pMutex);
    }

    goto cleanup;
}

DWORD
VmDirSyncCounterIncrement(
    PVMDIR_SYNCHRONIZE_COUNTER     pSyncCounter
    )
{
    return VmDirSyncCounterChange(pSyncCounter, 1);
}

DWORD
VmDirSyncCounterDecrement(
    PVMDIR_SYNCHRONIZE_COUNTER     pSyncCounter
    )
{
    return VmDirSyncCounterChange(pSyncCounter, -1);
}

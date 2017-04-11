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



/*
 * Module Name: common
 *
 * Filename: tsstack.c
 *
 * Abstract:
 *
 * thread safe stack of PVOID
 *
 */

#include "includes.h"

VOID
VmDirFreeTSStack(
    PVMDIR_TSSTACK pStack
    )
{
    if ( pStack )
    {
        if ( pStack->iSize != 0 )
        {
            VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, " VmDirFreeTSStack potential leak, size (%l)", pStack->iSize );
        }
        VMDIR_SAFE_FREE_MUTEX( pStack->pMutex );
        // TODO, register a cleanup function in TSSTACK, so we can properly cleanup PVOID elements.
        VMDIR_SAFE_FREE_MEMORY( pStack->ppBuffer );
        VMDIR_SAFE_FREE_MEMORY( pStack );
    }

    return;
}

DWORD
VmDirAllocateTSStack(
    size_t          iCapacity,
    PVMDIR_TSSTACK* ppStack
    )
{
    DWORD           dwError = 0;
    PVMDIR_TSSTACK  pStack = NULL;
    size_t          iMinCap = (iCapacity == 0) ? 10 : iCapacity;

    if (!ppStack)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError =  VmDirAllocateMemory(sizeof(VMDIR_TSSTACK), (PVOID*) &pStack);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex( &(pStack->pMutex) );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(PVOID) * iMinCap, (PVOID*)&(pStack->ppBuffer) );
    BAIL_ON_VMDIR_ERROR(dwError);

    pStack->iCapacity = iMinCap;

    *ppStack = pStack;
    pStack = NULL;

cleanup:

    return dwError;

error:
    if ( pStack )
    {
        VmDirFreeTSStack( pStack );
    }

    goto cleanup;
}

/*
 * TSSTACK does NOT take ownership of pElement.
 * Caller should clean up pElement.
 *
 * TODO, should register a cleanup function and let TSSTACK takes ownership of pElement.
 */
DWORD
VmDirPushTSStack(
    PVMDIR_TSSTACK   pStack,
    PVOID            pElement
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bInLock = FALSE;

    if (!pStack || !pElement)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bInLock, pStack->pMutex);

    if ( pStack->iSize == pStack->iCapacity )
    {
        dwError = VmDirReallocateMemoryWithInit( pStack->ppBuffer,
                                                 (PVOID*)&(pStack->ppBuffer),
                                                 sizeof(PVOID) * pStack->iCapacity * 2,
                                                 sizeof(PVOID) * pStack->iCapacity);
        BAIL_ON_VMDIR_ERROR(dwError);

        pStack->iCapacity = pStack->iCapacity * 2;
    }

    pStack->ppBuffer[ pStack->iSize ] = pElement;
    pStack->iSize++;

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, pStack->pMutex);

    return dwError;

error:

    VMDIR_LOG_ERROR ( VMDIR_LOG_MASK_ALL, "VmDirPushSTStack failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmDirPopTSStack(
    PVMDIR_TSSTACK   pStack,
    PVOID*           ppElement
    )
{
    DWORD     dwError = 0;
    BOOLEAN   bInLock = FALSE;

    if (!pStack || !ppElement)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bInLock, pStack->pMutex);

    if ( pStack->iSize > 0 )
    {
        *ppElement = pStack->ppBuffer[ pStack->iSize - 1 ];
        pStack->ppBuffer[ pStack->iSize - 1 ] = NULL;
        pStack->iSize--;
    }
    else
    {
        *ppElement = NULL;
    }

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, pStack->pMutex);

    return dwError;

error:

    VMDIR_LOG_ERROR ( VMDIR_LOG_MASK_ALL, "VmDirPopSTStack failed. Error(%u)", dwError);

    goto cleanup;
}

size_t
VmDirGetTSStackSize(
    PVMDIR_TSSTACK pStack
    )
{
    size_t    iRtn = 0;
    BOOLEAN   bInLock = FALSE;

    if ( pStack )
    {
        VMDIR_LOCK_MUTEX(bInLock, pStack->pMutex);
        iRtn = pStack->iSize;
        VMDIR_UNLOCK_MUTEX(bInLock, pStack->pMutex);
    }

    return iRtn;
}

BOOLEAN
VmDirIsEmptyTSStack(
    PVMDIR_TSSTACK pStack
    )
{
    return VmDirGetTSStackSize(pStack) == 0;
}

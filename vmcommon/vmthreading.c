/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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


#include "vmincludes.h"

DWORD
VmAllocateMutex(
    PVM_MUTEX*  ppMutex
    )
{
    DWORD       dwError = 0;
    PVM_MUTEX   pVmMutex = NULL;

    if ( ppMutex == NULL )
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmAllocateMemory( sizeof(VM_MUTEX), ((PVOID*)&pVmMutex));
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmInitializeMutexContent( pVmMutex );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    *ppMutex = pVmMutex;
    pVmMutex = NULL;

error:
    VM_COMMON_SAFE_FREE_MUTEX( pVmMutex );

    return dwError;
}

DWORD
VmInitializeMutexContent(
    PVM_MUTEX   pMutex
    )
{
    DWORD dwError = 0;

    if ( ( pMutex == NULL ) || ( pMutex->bInitialized != FALSE ) )
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    memset( &(pMutex->critSect), 0, sizeof(pthread_mutex_t) );

    dwError = pthread_mutex_init( &(pMutex->critSect), NULL );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pMutex->bInitialized = TRUE;

error:
    return dwError;
}

VOID
VmFreeMutex(
    PVM_MUTEX   pMutex
    )
{
    VmFreeMutexContent(pMutex);
    VM_COMMON_SAFE_FREE_MEMORY( pMutex );
}

VOID
VmFreeMutexContent(
    PVM_MUTEX   pMutex
    )
{
    if ( ( pMutex != NULL ) &&  ( pMutex->bInitialized != FALSE ) )
    {
        pthread_mutex_destroy(&(pMutex->critSect));
        pMutex->bInitialized = FALSE;
    }
}

DWORD
VmLockMutex(
    PVM_MUTEX   pMutex
    )
{
    DWORD dwError = 0;

    if (
        ( pMutex == NULL )
        ||
        (pMutex->bInitialized == FALSE)
        )
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = pthread_mutex_lock( &(pMutex->critSect) );
    BAIL_ON_VM_COMMON_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmUnLockMutex(
    PVM_MUTEX   pMutex
    )
{
    DWORD dwError = 0;

    if (
        ( pMutex == NULL )
        ||
        (pMutex->bInitialized == FALSE)
        )
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = pthread_mutex_unlock( &(pMutex->critSect) );
    BAIL_ON_VM_COMMON_ERROR(dwError);

error:

    return dwError;
}

BOOLEAN
VmIsMutexInitialized(
    PVM_MUTEX   pMutex
    )
{
    return ( pMutex != NULL ) &&
           ( pMutex->bInitialized != FALSE );
}

DWORD
VmAllocateRWLock(
    PVM_RWLOCK*     ppLock
    )
{
    DWORD dwError = 0;
    PVM_RWLOCK pLock = NULL;

    if (!ppLock)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmAllocateMemory(sizeof(VM_RWLOCK), (PVOID*)&pLock);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmInitializeRWLockContent(pLock);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    *ppLock = pLock;
    pLock = NULL;

error:
    VmFreeRWLock(pLock);
    return dwError;
}

DWORD
VmInitializeRWLockContent(
    PVM_RWLOCK   pLock
    )
{
    DWORD dwError = 0;

    if (!pLock || pLock->bInitialized)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    memset(&pLock->lock, 0, sizeof(pthread_mutex_t));

    dwError = pthread_rwlock_init(&pLock->lock, NULL);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pLock->bInitialized = TRUE;

error:
    return dwError;
}

VOID
VmFreeRWLock(
    PVM_RWLOCK   pLock
    )
{
    VmFreeRWLockContent(pLock);
    VM_COMMON_SAFE_FREE_MEMORY(pLock);
}

VOID
VmFreeRWLockContent(
    PVM_RWLOCK   pLock
    )
{
    if (pLock && pLock->bInitialized)
    {
        pthread_rwlock_destroy(&pLock->lock);
        pLock->bInitialized = FALSE;
    }
}

DWORD
VmRWLockReadLock(
    PVM_RWLOCK      pLock,
    DWORD           dwMilliSec
    )
{
#ifdef __APPLE__
    return VM_COMMON_ERROR_OPERATION_NOT_ALLOWED;
#else
    DWORD dwError = 0;

    if (!pLock || !pLock->bInitialized)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    if (dwMilliSec)
    {
        struct timespec ts = {0};
        uint64_t iTimeInMSec = 0;

        iTimeInMSec =  dwMilliSec + VmGetTimeInMilliSec();
        ts.tv_sec = iTimeInMSec / MSECS_PER_SEC;
        ts.tv_nsec = (iTimeInMSec % MSECS_PER_SEC) * NSECS_PER_MSEC;

        dwError = pthread_rwlock_timedrdlock(&pLock->lock, &ts);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }
    else
    {
        dwError = pthread_rwlock_rdlock(&pLock->lock);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

error:
    return dwError;
#endif
}

DWORD
VmRWLockWriteLock(
    PVM_RWLOCK      pLock,
    DWORD           dwMilliSec
    )
{
#ifdef __APPLE__
    return VM_COMMON_ERROR_OPERATION_NOT_ALLOWED;
#else
    DWORD dwError = 0;

    if (!pLock || !pLock->bInitialized)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    if (dwMilliSec)
    {
        struct timespec ts = {0};
        uint64_t iTimeInMSec = 0;

        iTimeInMSec =  dwMilliSec + VmGetTimeInMilliSec();
        ts.tv_sec = iTimeInMSec / MSECS_PER_SEC;
        ts.tv_nsec = (iTimeInMSec % MSECS_PER_SEC) * NSECS_PER_MSEC;

        dwError = pthread_rwlock_timedwrlock(&pLock->lock, &ts);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }
    else
    {
        dwError = pthread_rwlock_wrlock(&pLock->lock);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

error:
    return dwError;
#endif
}

DWORD
VmRWLockUnlock(
    PVM_RWLOCK   pLock
    )
{
    DWORD dwError = 0;

    if (!pLock || !pLock->bInitialized)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = pthread_rwlock_unlock(&pLock->lock);
    BAIL_ON_VM_COMMON_ERROR(dwError);

error:
    return dwError;
}

BOOLEAN
VmIsRWLockInitialized(
    PVM_RWLOCK   pLock
    )
{
    return pLock && pLock->bInitialized;
}

DWORD
VmAllocateCondition(
    PVM_COND*   ppCondition
    )
{
    DWORD dwError = 0;
    PVM_COND pVmCond = NULL;

    if ( ppCondition == NULL )
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmAllocateMemory( sizeof(VM_COND), ((PVOID*)&pVmCond));
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmInitializeConditionContent( pVmCond );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    *ppCondition = pVmCond;
    pVmCond = NULL;

error:

    VM_COMMON_SAFE_FREE_CONDITION( pVmCond );

    return dwError;
}

DWORD
VmInitializeConditionContent(
    PVM_COND    pCondition
    )
{
    DWORD dwError = 0;

    if ( ( pCondition == NULL ) || ( pCondition->bInitialized != FALSE ) )
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    memset( &(pCondition->cond), 0, sizeof(pthread_cond_t) );

    dwError = pthread_cond_init( &(pCondition->cond), NULL);
    BAIL_ON_VM_COMMON_ERROR(dwError);
    pCondition->bInitialized = TRUE;

error:

    return dwError;
}

VOID
VmFreeCondition(
    PVM_COND    pCondition
    )
{
    VmFreeConditionContent( pCondition );
    VM_COMMON_SAFE_FREE_MEMORY( pCondition );
}

VOID
VmFreeConditionContent(
    PVM_COND    pCondition
    )
{
    if ( ( pCondition != NULL ) &&  ( pCondition->bInitialized != FALSE ) )
    {
        pthread_cond_destroy(&(pCondition->cond));
        pCondition->bInitialized = FALSE;
    }
}

DWORD
VmConditionWait(
    PVM_COND    pCondition,
    PVM_MUTEX   pMutex
    )
{
    DWORD dwError = 0;

    if ( ( pCondition == NULL )
         ||
         ( pCondition->bInitialized == FALSE )
         ||
         ( pMutex == NULL )
         ||
         ( pMutex->bInitialized == FALSE )
       )
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = pthread_cond_wait(
        &(pCondition->cond),
        &(pMutex->critSect)
    );
    BAIL_ON_VM_COMMON_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmConditionTimedWait(
    PVM_COND    pCondition,
    PVM_MUTEX   pMutex,
    DWORD       dwMilliseconds
    )
{
    DWORD dwError = 0;
    struct timespec ts = {0};
    uint64_t iTimeInMSec = 0;

    if ( ( pCondition == NULL )
         ||
         ( pCondition->bInitialized == FALSE )
         ||
         ( pMutex == NULL )
         ||
         ( pMutex->bInitialized == FALSE )
       )
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    iTimeInMSec =  dwMilliseconds + VmGetTimeInMilliSec();

    ts.tv_sec = iTimeInMSec / MSECS_PER_SEC;
    ts.tv_nsec = (iTimeInMSec % MSECS_PER_SEC) * NSECS_PER_MSEC;

    dwError = pthread_cond_timedwait(
        &(pCondition->cond),
        &(pMutex->critSect),
        &ts
    );
    BAIL_ON_VM_COMMON_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmConditionSignal(
    PVM_COND    pCondition
    )
{
    DWORD dwError = 0;

    if ( ( pCondition == NULL )
         ||
         ( pCondition->bInitialized == FALSE )
       )
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = pthread_cond_signal( &(pCondition->cond) );
    BAIL_ON_VM_COMMON_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmConditionBroadcast(
    PVM_COND    pCondition
    )
{
    DWORD dwError = 0;

    if ((pCondition == NULL) ||
        (pCondition->bInitialized == FALSE))
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = pthread_cond_broadcast(&(pCondition->cond));
    BAIL_ON_VM_COMMON_ERROR(dwError);

error:

    return dwError;
}

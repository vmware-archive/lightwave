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
LwCAAllocateMutex(
    PLWCA_MUTEX* ppMutex
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PLWCA_MUTEX pLwCAMutex = NULL;

    if ( ppMutex == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory( sizeof(LWCA_MUTEX), ((PVOID*)&pLwCAMutex));
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAInitializeMutexContent( pLwCAMutex );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppMutex = pLwCAMutex;
    pLwCAMutex = NULL;

error:

    LWCA_SAFE_FREE_MUTEX( pLwCAMutex );

    return dwError;
}

DWORD
LwCAInitializeMutexContent(
    PLWCA_MUTEX pMutex
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pMutex == NULL ) || ( pMutex->bInitialized != FALSE ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    memset( &(pMutex->critSect), 0, sizeof(pthread_mutex_t) );

    dwError = pthread_mutex_init( &(pMutex->critSect), NULL );
    BAIL_ON_LWCA_ERROR(dwError);

    pMutex->bInitialized = TRUE;

error:

    return dwError;
}

VOID
LwCAFreeMutex(
    PLWCA_MUTEX pMutex
    )
{
    LwCAFreeMutexContent(pMutex);
    LWCA_SAFE_FREE_MEMORY( pMutex );
}

VOID
LwCAFreeMutexContent(
    PLWCA_MUTEX pMutex
    )
{
    if ( ( pMutex != NULL ) &&  ( pMutex->bInitialized != FALSE ) )
    {
        pthread_mutex_destroy(&(pMutex->critSect));
        pMutex->bInitialized = FALSE;
    }
}

DWORD
LwCALockMutex(
    PLWCA_MUTEX pMutex
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if (
        ( pMutex == NULL )
        ||
        (pMutex->bInitialized == FALSE)
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = pthread_mutex_lock( &(pMutex->critSect) );
    BAIL_ON_LWCA_ERROR(dwError);

error:

    return dwError;
}

DWORD
LwCAUnlockMutex(
    PLWCA_MUTEX pMutex
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if (
        ( pMutex == NULL )
        ||
        (pMutex->bInitialized == FALSE)
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = pthread_mutex_unlock( &(pMutex->critSect) );
    BAIL_ON_LWCA_ERROR(dwError);

error:

    return dwError;
}

BOOLEAN
LwCAIsMutexInitialized(
    PLWCA_MUTEX pMutex
    )
{
    return ( pMutex != NULL ) &&
           ( pMutex->bInitialized != FALSE );
}

DWORD
LwCAAllocateCondition(
    PLWCA_COND* ppCondition
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PLWCA_COND pLwCACond = NULL;

    if ( ppCondition == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory( sizeof(LWCA_COND), ((PVOID*)&pLwCACond));
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAInitializeConditionContent( pLwCACond );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCondition = pLwCACond;
    pLwCACond = NULL;

error:

    LWCA_SAFE_FREE_CONDITION( pLwCACond );

    return dwError;
}

DWORD
LwCAInitializeConditionContent(
    PLWCA_COND pCondition
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pCondition == NULL ) || ( pCondition->bInitialized != FALSE ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    memset( &(pCondition->cond), 0, sizeof(pthread_cond_t) );

    dwError = pthread_cond_init( &(pCondition->cond), NULL);
    BAIL_ON_LWCA_ERROR(dwError);
    pCondition->bInitialized = TRUE;

error:

    return dwError;
}

VOID
LwCAFreeCondition(
    PLWCA_COND pCondition
    )
{
    LwCAFreeConditionContent( pCondition );
    LWCA_SAFE_FREE_MEMORY( pCondition );
}

VOID
LwCAFreeConditionContent(
    PLWCA_COND pCondition
    )
{
    if ( ( pCondition != NULL ) &&  ( pCondition->bInitialized != FALSE ) )
    {
        pthread_cond_destroy(&(pCondition->cond));
        pCondition->bInitialized = FALSE;
    }
}

DWORD
LwCAConditionWait(
    PLWCA_COND pCondition,
    PLWCA_MUTEX pMutex
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pCondition == NULL )
         ||
         ( pCondition->bInitialized == FALSE )
         ||
         ( pMutex == NULL )
         ||
         ( pMutex->bInitialized == FALSE )
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = pthread_cond_wait(
        &(pCondition->cond),
        &(pMutex->critSect)
    );
    BAIL_ON_LWCA_ERROR(dwError);

error:

    return dwError;
}

DWORD
LwCAConditionTimedWait(
    PLWCA_COND pCondition,
    PLWCA_MUTEX pMutex,
    DWORD dwMilliseconds
    )
{
    DWORD dwError = ERROR_SUCCESS;
    struct timespec ts = {0};
    BOOL bLocked = FALSE;

    if ( ( pCondition == NULL )
         ||
         ( pCondition->bInitialized == FALSE )
         ||
         ( pMutex == NULL )
         ||
         ( pMutex->bInitialized == FALSE )
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    ts.tv_sec = time(NULL) + dwMilliseconds/1000;
    ts.tv_nsec = 0;

    dwError = LwCALockMutex(pMutex);
    BAIL_ON_LWCA_ERROR(dwError);

    bLocked = TRUE;

    dwError = pthread_cond_timedwait(
        &(pCondition->cond),
        &(pMutex->critSect),
        &ts
    );
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:

    if (bLocked)
    {
        LwCAUnlockMutex(pMutex);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
LwCAConditionSignal(
    PLWCA_COND pCondition
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pCondition == NULL )
         ||
         ( pCondition->bInitialized == FALSE )
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = pthread_cond_signal( &(pCondition->cond) );
    BAIL_ON_LWCA_ERROR(dwError);

error:

    return dwError;
}

static
PVOID
ThreadFunction(
  PVOID pArgs
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PLWCA_START_ROUTINE pThreadStart = NULL;
    PVOID pThreadArgs = NULL;
    union
    {
        DWORD dwError;
        PVOID pvRet;
    } retVal = { 0 };

    if( pArgs == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pThreadStart = ((PLWCA_THREAD_START_INFO)pArgs)->pStartRoutine;
    pThreadArgs = ((PLWCA_THREAD_START_INFO)pArgs)->pArgs;

    if( pThreadStart == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_SAFE_FREE_MEMORY( pArgs );

    dwError = pThreadStart( pThreadArgs );
    BAIL_ON_LWCA_ERROR(dwError);

error:

    LWCA_SAFE_FREE_MEMORY( pArgs );

    retVal.dwError = dwError;
    return retVal.pvRet;
}

DWORD
LwCACreateThread(
    PLWCA_THREAD pThread,
    BOOLEAN bDetached,
    LwCAStartRoutine* pStartRoutine,
    PVOID pArgs
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PLWCA_THREAD_START_INFO pThreadStartInfo = NULL;
    pthread_attr_t     thrAttr;
    BOOLEAN bThreadAttrInited = FALSE;

    if ( ( pThread == NULL ) || ( pStartRoutine == NULL ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if( bDetached != FALSE )
    {
        pthread_attr_init(&thrAttr);
        bThreadAttrInited = TRUE;
        pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);
    }

    dwError = LwCAAllocateMemory(
        sizeof(LWCA_THREAD_START_INFO),
        ((PVOID*)&pThreadStartInfo)
    );
    BAIL_ON_LWCA_ERROR(dwError);

    pThreadStartInfo->pStartRoutine = pStartRoutine;
    pThreadStartInfo->pArgs = pArgs;

    dwError = pthread_create(
        pThread,
        ((bDetached == FALSE) ? NULL : &thrAttr),
        ThreadFunction,
        pThreadStartInfo
    );
    BAIL_ON_LWCA_ERROR(dwError);

    // we started successfully -> pThreadStartInfo is now owned by
    // ThreadFunction
    pThreadStartInfo = NULL;

error:

    if(bThreadAttrInited != FALSE)
    {
        pthread_attr_destroy(&thrAttr);
    }

    LWCA_SAFE_FREE_MEMORY( pThreadStartInfo );

    return dwError;
}

DWORD
LwCAThreadJoin(
    PLWCA_THREAD pThread,
    PDWORD pRetVal
    )
{
    DWORD dwError = ERROR_SUCCESS;
    union
    {
        DWORD dwError;
        PVOID pvRet;
    } retVal = { 0 };

    if(pThread == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = pthread_join(
        (*pThread),
        ((pRetVal != NULL) ? &(retVal.pvRet) : NULL)
    );
    BAIL_ON_LWCA_ERROR(dwError);

    if( pRetVal != NULL )
    {
        // our ThreadFunction returns error code
        *pRetVal = retVal.dwError;
    }

error:

    return dwError;
}

VOID
LwCAFreeThread(
    PLWCA_THREAD pThread
    )
{
    if ( pThread != NULL )
    {
        // nothing to free really
#ifndef _WIN32
        (*pThread) = 0;
#endif
    }
}

DWORD
LwCAAllocateRWLock(
    PLWCA_RWLOCK* ppLock
    )
{
    DWORD dwError = 0;
    PLWCA_RWLOCK pLock = NULL;

    BAIL_ON_LWCA_INVALID_POINTER(ppLock, dwError);

    dwError = LwCAAllocateMemory(sizeof(LWCA_RWLOCK), (void**)&pLock);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = pthread_rwlock_init(&pLock->rwLock, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppLock = pLock;

cleanup:
    return dwError;
error:
    LWCA_SAFE_FREE_MEMORY(pLock);
    goto cleanup;
}

VOID
LwCAFreeRWLock(
    PLWCA_RWLOCK pLock
    )
{
    if (pLock)
    {
        pthread_rwlock_destroy(&pLock->rwLock);
        LwCAFreeMemory(pLock);
    }
}

void
LwCALockRead(
    PLWCA_RWLOCK  pLock
    )
{
    pthread_rwlock_rdlock(&pLock->rwLock);
}

int
LwCATryLockRead(
    PLWCA_RWLOCK  pLock
    )
{
    return pthread_rwlock_tryrdlock(&pLock->rwLock);
}

void
LwCAUnlockRead(
    PLWCA_RWLOCK  pLock
    )
{
    pthread_rwlock_unlock(&pLock->rwLock);
}

void
LwCALockWrite(
    PLWCA_RWLOCK  pLock
    )
{
    pthread_rwlock_wrlock(&pLock->rwLock);
}

int
LwCATryLockWrite(
    PLWCA_RWLOCK  pLock
    )
{
    return pthread_rwlock_trywrlock(&pLock->rwLock);
}

void
LwCAUnlockWrite(
    PLWCA_RWLOCK  pLock
    )
{
    pthread_rwlock_unlock(&pLock->rwLock);
}

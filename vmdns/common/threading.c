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
 * Module   : logging.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Common Utilities (Client & Server)
 *
 *            Threading
 *
 */

#include "includes.h"

static
void
VmDnsFreeLockCount(
    void* pkeyData
    );

static
int*
VmDnsGetLockKey(
    pthread_key_t*  pLockKey
    );

DWORD
VmDnsAllocateMutex(
    PVMDNS_MUTEX* ppMutex
)
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_MUTEX pVmDnsMutex = NULL;

    if ( ppMutex == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory( sizeof(VMDNS_MUTEX), ((PVOID*)&pVmDnsMutex));
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsInitializeMutexContent( pVmDnsMutex );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppMutex = pVmDnsMutex;
    pVmDnsMutex = NULL;

error:

    VMDNS_SAFE_FREE_MUTEX( pVmDnsMutex );

    return dwError;
}

DWORD
VmDnsInitializeMutexContent(
    PVMDNS_MUTEX pMutex
)
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pMutex == NULL ) || ( pMutex->bInitialized != FALSE ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    memset( &(pMutex->critSect), 0, sizeof(pthread_mutex_t) );

    dwError = pthread_mutex_init( &(pMutex->critSect), NULL );
    BAIL_ON_VMDNS_ERROR(dwError);

    pMutex->bInitialized = TRUE;

error:

    return dwError;
}

VOID
VmDnsFreeMutex(
    PVMDNS_MUTEX pMutex
)
{
    VmDnsFreeMutexContent(pMutex);
    VMDNS_SAFE_FREE_MEMORY( pMutex );
}

VOID
VmDnsFreeMutexContent(
    PVMDNS_MUTEX pMutex
)
{
    if ( ( pMutex != NULL ) &&  ( pMutex->bInitialized != FALSE ) )
    {
        pthread_mutex_destroy(&(pMutex->critSect));
        pMutex->bInitialized = FALSE;
    }
}

DWORD
VmDnsLockMutex(
    PVMDNS_MUTEX pMutex
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
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = pthread_mutex_lock( &(pMutex->critSect) );
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDnsUnlockMutex(
    PVMDNS_MUTEX pMutex
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
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = pthread_mutex_unlock( &(pMutex->critSect) );
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
}

BOOLEAN
VmDnsIsMutexInitialized(
    PVMDNS_MUTEX pMutex
)
{
    return ( pMutex != NULL ) &&
           ( pMutex->bInitialized != FALSE );
}

DWORD
VmDnsAllocateCondition(
    PVMDNS_COND* ppCondition
)
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_COND pVmDnsCond = NULL;

    if ( ppCondition == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory( sizeof(VMDNS_COND), ((PVOID*)&pVmDnsCond));
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsInitializeConditionContent( pVmDnsCond );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppCondition = pVmDnsCond;
    pVmDnsCond = NULL;

error:

    VMDNS_SAFE_FREE_CONDITION( pVmDnsCond );

    return dwError;
}

DWORD
VmDnsInitializeConditionContent(
    PVMDNS_COND pCondition
)
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pCondition == NULL ) || ( pCondition->bInitialized != FALSE ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    memset( &(pCondition->cond), 0, sizeof(pthread_cond_t) );

    dwError = pthread_cond_init( &(pCondition->cond), NULL);
    BAIL_ON_VMDNS_ERROR(dwError);
    pCondition->bInitialized = TRUE;

error:

    return dwError;
}

VOID
VmDnsFreeCondition(
    PVMDNS_COND pCondition
)
{
    VmDnsFreeConditionContent( pCondition );
    VMDNS_SAFE_FREE_MEMORY( pCondition );
}

VOID
VmDnsFreeConditionContent(
    PVMDNS_COND pCondition
)
{
    if ( ( pCondition != NULL ) &&  ( pCondition->bInitialized != FALSE ) )
    {
        pthread_cond_destroy(&(pCondition->cond));
        pCondition->bInitialized = FALSE;
    }
}

DWORD
VmDnsConditionWait(
    PVMDNS_COND pCondition,
    PVMDNS_MUTEX pMutex
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
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = pthread_cond_wait(
        &(pCondition->cond),
        &(pMutex->critSect)
    );
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDnsConditionTimedWait(
    PVMDNS_COND pCondition,
    PVMDNS_MUTEX pMutex,
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
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    ts.tv_sec = time(NULL) + dwMilliseconds/1000;
    ts.tv_nsec = 0;

    dwError = VmDnsLockMutex(pMutex);
    BAIL_ON_VMDNS_ERROR(dwError);

    bLocked = TRUE;

    dwError = pthread_cond_timedwait(
        &(pCondition->cond),
        &(pMutex->critSect),
        &ts
    );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pMutex);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsConditionSignal(
    PVMDNS_COND pCondition
)
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pCondition == NULL )
         ||
         ( pCondition->bInitialized == FALSE )
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = pthread_cond_signal( &(pCondition->cond) );
    BAIL_ON_VMDNS_ERROR(dwError);

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
    PVMDNS_START_ROUTINE pThreadStart = NULL;
    PVOID pThreadArgs = NULL;
    union
    {
        DWORD dwError;
        PVOID pvRet;
    } retVal = { 0 };

    if( pArgs == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pThreadStart = ((PVMDNS_THREAD_START_INFO)pArgs)->pStartRoutine;
    pThreadArgs = ((PVMDNS_THREAD_START_INFO)pArgs)->pArgs;

    if( pThreadStart == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VMDNS_SAFE_FREE_MEMORY( pArgs );

    dwError = pThreadStart( pThreadArgs );
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    VMDNS_SAFE_FREE_MEMORY( pArgs );

    retVal.dwError = dwError;
    return retVal.pvRet;
}

DWORD
VmDnsCreateThread(
    PVMDNS_THREAD pThread,
    BOOLEAN bDetached,
    VmDnsStartRoutine* pStartRoutine,
    PVOID pArgs
)
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_THREAD_START_INFO pThreadStartInfo = NULL;
    pthread_attr_t     thrAttr;
    BOOLEAN bThreadAttrInited = FALSE;

    if ( ( pThread == NULL ) || ( pStartRoutine == NULL ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if( bDetached != FALSE )
    {
        pthread_attr_init(&thrAttr);
        bThreadAttrInited = TRUE;
        pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);
    }

    dwError = VmDnsAllocateMemory(
        sizeof(VMDNS_THREAD_START_INFO),
        ((PVOID*)&pThreadStartInfo)
    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pThreadStartInfo->pStartRoutine = pStartRoutine;
    pThreadStartInfo->pArgs = pArgs;

    dwError = pthread_create(
        pThread,
        ((bDetached == FALSE) ? NULL : &thrAttr),
        ThreadFunction,
        pThreadStartInfo
    );
    BAIL_ON_VMDNS_ERROR(dwError);

    // we started successfully -> pThreadStartInfo is now owned by
    // ThreadFunction
    pThreadStartInfo = NULL;

error:

    if(bThreadAttrInited != FALSE)
    {
        pthread_attr_destroy(&thrAttr);
    }

    VMDNS_SAFE_FREE_MEMORY( pThreadStartInfo );

    return dwError;
}

DWORD
VmDnsThreadJoin(
    PVMDNS_THREAD pThread,
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
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = pthread_join(
        (*pThread),
        ((pRetVal != NULL) ? &(retVal.pvRet) : NULL)
    );
    BAIL_ON_VMDNS_ERROR(dwError);

    if( pRetVal != NULL )
    {
        // our ThreadFunction returns error code
        *pRetVal = retVal.dwError;
    }

error:

    return dwError;
}

VOID
VmDnsFreeThread(
    PVMDNS_THREAD pThread
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
VmDnsAllocateRWLock(
    PVMDNS_RWLOCK* ppLock
    )
{
    DWORD dwError = 0;
    PVMDNS_RWLOCK pLock = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppLock, dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_RWLOCK), (void**)&pLock);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = pthread_rwlock_init(&pLock->rwLock, NULL);
    dwError = POSIX_TO_WIN32_ERROR(dwError);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = pthread_key_create(&pLock->readKey, VmDnsFreeLockCount);
    dwError = POSIX_TO_WIN32_ERROR(dwError);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = pthread_key_create(&pLock->writeKey, VmDnsFreeLockCount);
    dwError = POSIX_TO_WIN32_ERROR(dwError);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppLock = pLock;

cleanup:
    return dwError;
error:
    VMDNS_SAFE_FREE_MEMORY(pLock);
    goto cleanup;
}

VOID
VmDnsFreeRWLock(
    PVMDNS_RWLOCK pLock
    )
{
    pthread_key_delete(pLock->readKey);
    pthread_key_delete(pLock->writeKey);
    VmDnsFreeMemory(pLock);
}

void
VmDnsLockRead(
    PVMDNS_RWLOCK  pLock
    )
{
    int* pWriteLockCount = VmDnsGetLockKey(&pLock->writeKey);
    int* pReadLockCount = VmDnsGetLockKey(&pLock->readKey);

    if (!pWriteLockCount || !pReadLockCount)
    {
        VMDNS_LOG_WARNING("Out of memory, try plain locking.");
        pthread_rwlock_rdlock(&pLock->rwLock);
    }
    else
    {
        if (*pWriteLockCount > 0)
        {
            // Simply increment a read count but don't lock for read
            // as that would cause undefined behavior.
            VmDnsLog(VMDNS_LOG_LEVEL_DEBUG,
                     "Lock read when already holding %u write lock.",
                     *pWriteLockCount);
        }
        else
        {
            if (*pReadLockCount == 0)
            {
                pthread_rwlock_rdlock(&pLock->rwLock);
                VmDnsLog(VMDNS_LOG_LEVEL_DEBUG,
                         "Actually locking for read. Result read count: %u\n",
                         *pReadLockCount + 1);
            }
        }
        (*pReadLockCount)++;
        VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "[++READ %u]\n", *pReadLockCount);
    }
}

int
VmDnsTryLockRead(
    PVMDNS_RWLOCK  pLock
    )
{
    int result = ERROR_BUSY;
    int* pWriteLockCount = VmDnsGetLockKey(&pLock->writeKey);
    int* pReadLockCount = VmDnsGetLockKey(&pLock->readKey);

    if (pWriteLockCount && pReadLockCount)
    {
        if (*pWriteLockCount > 0)
        {
            // Simply increment a read count but don't lock for read
            // as that would cause undefined behavior.
            result = 0;
            VmDnsLog(VMDNS_LOG_LEVEL_DEBUG,
                     "Lock read when already holding %u write lock.",
                     *pWriteLockCount);
        }
        else
        {
            if (*pReadLockCount == 0)
            {
                result = pthread_rwlock_tryrdlock(&pLock->rwLock);
                if (!result)
                {
                    VmDnsLog(VMDNS_LOG_LEVEL_DEBUG,
                             "Locked for read. Result read count: %u\n",
                             *pReadLockCount + 1);
                }
            }
            else
            {
                result = 0;
            }
        }

        if (!result)
        {
            (*pReadLockCount)++;
            VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "[++READ %u]\n", *pReadLockCount);
        }
    }

    return result;
}

void
VmDnsUnlockRead(
    PVMDNS_RWLOCK  pLock
    )
{
    int* pWriteLockCount = VmDnsGetLockKey(&pLock->writeKey);
    int* pReadLockCount = VmDnsGetLockKey(&pLock->readKey);

    if (!pWriteLockCount || !pReadLockCount)
    {
        VMDNS_LOG_ERROR("Out of memory, can't decrement lock count.");
    }
    else
    {
        if (*pWriteLockCount > 0)
        {
            VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "Read unlock while already holding write lock.");
        }
        else
        {
            if (*pReadLockCount ==1)
            {
                pthread_rwlock_unlock(&pLock->rwLock);
                VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "[UNLOCK READ]");
            }
        }

        if (*pReadLockCount > 0)
        {
            (*pReadLockCount)--;
            VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "[--READ %u]\n", *pReadLockCount);
        }
        else
        {
            VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "Unexpected read unlock.");
        }
    }
}

void
VmDnsLockWrite(
    PVMDNS_RWLOCK  pLock
    )
{
    int* pWriteLockCount = VmDnsGetLockKey(&pLock->writeKey);
    if (!pWriteLockCount)
    {
        VMDNS_LOG_WARNING("Out of memory, try plain locking.");
        pthread_rwlock_wrlock(&pLock->rwLock);
    }
    else
    {
        if (*pWriteLockCount == 0)
        {
            pthread_rwlock_wrlock(&pLock->rwLock);
            VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "[LOCK WRITE]");
        }
        (*pWriteLockCount)++;
        VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "[++WRITE %u]\n", *pWriteLockCount);
    }
}

int
VmDnsTryLockWrite(
    PVMDNS_RWLOCK  pLock
    )
{
    int result = ERROR_BUSY;
    int* pWriteLockCount = VmDnsGetLockKey(&pLock->writeKey);
    int* pReadLockCount = VmDnsGetLockKey(&pLock->readKey);
    if (pWriteLockCount && pReadLockCount)
    {
        if (*pReadLockCount > 0)
        {
            VmDnsLog(VMDNS_LOG_LEVEL_DEBUG,
                    "Cannot wrlock with %u existing rdlock from same thread.\n",
                    *pReadLockCount);
            result = ERROR_POSSIBLE_DEADLOCK;
        }
        else
        {
            if (*pWriteLockCount <= 0)
            {
                result = pthread_rwlock_trywrlock(&pLock->rwLock);
                if (result)
                {
                    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "trywrlock returned %u\n", result);
                }
            }
            else
            {
                result = 0;
            }

            if (!result)
            {
                (*pWriteLockCount)++;
                VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "[++WRITE %u]\n", *pWriteLockCount);
            }
        }
    }

    return result;
}

void
VmDnsUnlockWrite(
    PVMDNS_RWLOCK  pLock
    )
{
    int* pWriteLockCount = VmDnsGetLockKey(&pLock->writeKey);
    if (!pWriteLockCount)
    {
        VMDNS_LOG_ERROR("Out of memory, can't decrement lock count.");
    }
    else
    {
        if (*pWriteLockCount ==1)
        {
            pthread_rwlock_unlock(&pLock->rwLock);
            VmDnsLog(VMDNS_LOG_LEVEL_DEBUG,
                    "[UNLOCK WRITE] result write count is %u.\n",
                    *pWriteLockCount);
        }

        if (*pWriteLockCount > 0)
        {
            (*pWriteLockCount)--;
            VmDnsLog(VMDNS_LOG_LEVEL_DEBUG, "[--WRITE %u]\n", *pWriteLockCount);
        }
        else
        {
            VmDnsLog(VMDNS_LOG_LEVEL_ERROR,
                     "Unexpected unlock write, write count is %u.\n",
                     *pWriteLockCount);
        }
    }
}

static
void
VmDnsFreeLockCount(
    void* pkeyData
    )
{
    VmDnsFreeMemory(pkeyData);
}

static
int*
VmDnsGetLockKey(
    pthread_key_t*  pLockKey
)
{
    DWORD dwError = ERROR_SUCCESS;
    int* pCounter = NULL;

    int* pCount = (int*)pthread_getspecific(*pLockKey);
    if (!pCount)
    {
        dwError = VmDnsAllocateMemory(sizeof(int), (void**)&(pCounter));
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = pthread_setspecific(*pLockKey, pCounter);
        dwError = POSIX_TO_WIN32_ERROR(dwError);
        BAIL_ON_VMDNS_ERROR(dwError);
        pCounter = NULL;

        pCount = (int*)pthread_getspecific(*pLockKey);
    }

cleanup:
    return pCount;

error:
    VMDNS_SAFE_FREE_MEMORY(pCounter);
    goto cleanup;
}

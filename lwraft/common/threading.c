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

#if !defined(_WIN32) || defined(HAVE_PTHREADS_WIN32)

DWORD
VmDirAllocateMutex(
    PVMDIR_MUTEX* ppMutex
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDIR_MUTEX pVmDirMutex = NULL;

    if ( ppMutex == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory( sizeof(VMDIR_MUTEX), ((PVOID*)&pVmDirMutex));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitializeMutexContent( pVmDirMutex );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMutex = pVmDirMutex;
    pVmDirMutex = NULL;

error:

    VMDIR_SAFE_FREE_MUTEX( pVmDirMutex );

    return dwError;
}

DWORD
VmDirInitializeMutexContent(
    PVMDIR_MUTEX pMutex
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pMutex == NULL ) || ( pMutex->bInitialized != FALSE ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    memset( &(pMutex->critSect), 0, sizeof(pthread_mutex_t) );

    dwError = pthread_mutex_init( &(pMutex->critSect), NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    pMutex->bInitialized = TRUE;

error:

    return dwError;
}

VOID
VmDirFreeMutex(
    PVMDIR_MUTEX pMutex
    )
{
    VmDirFreeMutexContent(pMutex);
    VMDIR_SAFE_FREE_MEMORY( pMutex );
}

VOID
VmDirFreeMutexContent(
    PVMDIR_MUTEX pMutex
    )
{
    if ( ( pMutex != NULL ) &&  ( pMutex->bInitialized != FALSE ) )
    {
        pthread_mutex_destroy(&(pMutex->critSect));
        pMutex->bInitialized = FALSE;
    }
}


DWORD
VmDirLockMutex(
    PVMDIR_MUTEX pMutex
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
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = pthread_mutex_lock( &(pMutex->critSect) );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDirUnLockMutex(
    PVMDIR_MUTEX pMutex
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
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = pthread_mutex_unlock( &(pMutex->critSect) );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

BOOLEAN
VmDirIsMutexInitialized(
    PVMDIR_MUTEX pMutex
    )
{
    return ( pMutex != NULL ) &&
           ( pMutex->bInitialized != FALSE );
}

DWORD
VmDirAllocateCondition(
    PVMDIR_COND* ppCondition
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDIR_COND pVmDirCond = NULL;

    if ( ppCondition == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory( sizeof(VMDIR_COND), ((PVOID*)&pVmDirCond));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitializeConditionContent( pVmDirCond );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppCondition = pVmDirCond;
    pVmDirCond = NULL;

error:

    VMDIR_SAFE_FREE_CONDITION( pVmDirCond );

    return dwError;
}

DWORD
VmDirInitializeConditionContent(
    PVMDIR_COND pCondition
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pCondition == NULL ) || ( pCondition->bInitialized != FALSE ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    memset( &(pCondition->cond), 0, sizeof(pthread_cond_t) );

    dwError = pthread_cond_init( &(pCondition->cond), NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pCondition->bInitialized = TRUE;

error:

    return dwError;
}

VOID
VmDirFreeCondition(
    PVMDIR_COND pCondition
    )
{
    VmDirFreeConditionContent( pCondition );
    VMDIR_SAFE_FREE_MEMORY( pCondition );
}

VOID
VmDirFreeConditionContent(
    PVMDIR_COND pCondition
    )
{
    if ( ( pCondition != NULL ) &&  ( pCondition->bInitialized != FALSE ) )
    {
        pthread_cond_destroy(&(pCondition->cond));
        pCondition->bInitialized = FALSE;
    }
}

DWORD
VmDirConditionWait(
    PVMDIR_COND pCondition,
    PVMDIR_MUTEX pMutex
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
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = pthread_cond_wait(
        &(pCondition->cond),
        &(pMutex->critSect)
    );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDirConditionTimedWait(
    PVMDIR_COND pCondition,
    PVMDIR_MUTEX pMutex,
    DWORD dwMilliseconds
    )
{
    DWORD dwError = ERROR_SUCCESS;
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
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    iTimeInMSec =  dwMilliseconds + VmDirGetTimeInMilliSec();

    ts.tv_sec = iTimeInMSec / MSECS_PER_SEC;
    ts.tv_nsec = (iTimeInMSec % MSECS_PER_SEC) * NSECS_PER_MSEC;

    dwError = pthread_cond_timedwait(
        &(pCondition->cond),
        &(pMutex->critSect),
        &ts
    );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDirConditionSignal(
    PVMDIR_COND pCondition
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pCondition == NULL )
         ||
         ( pCondition->bInitialized == FALSE )
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = pthread_cond_signal( &(pCondition->cond) );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDirConditionBroadcast(
    PVMDIR_COND pCondition
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if ((pCondition == NULL) ||
        (pCondition->bInitialized == FALSE))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = pthread_cond_broadcast(&(pCondition->cond));
    BAIL_ON_VMDIR_ERROR(dwError);

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
    PVMDIR_START_ROUTINE pThreadStart = NULL;
    PVOID pThreadArgs = NULL;
    union
    {
        DWORD dwError;
        PVOID pvRet;
    } retVal;

    memset(&retVal, 0, sizeof(retVal));
    if( pArgs == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pThreadStart = ((PVMDIR_THREAD_START_INFO)pArgs)->pStartRoutine;
    pThreadArgs = ((PVMDIR_THREAD_START_INFO)pArgs)->pArgs;

    if( pThreadStart == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_SAFE_FREE_MEMORY( pArgs );

    dwError = pThreadStart( pThreadArgs );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    VMDIR_SAFE_FREE_MEMORY( pArgs );

    retVal.dwError = dwError;
    return retVal.pvRet;
}

DWORD
VmDirCreateThread(
    PVMDIR_THREAD pThread,
    BOOLEAN bJoinThr,
    VmDirStartRoutine* pStartRoutine,
    PVOID pArgs
    )
{
    DWORD                       dwError = ERROR_SUCCESS;
    PVMDIR_THREAD_START_INFO    pThreadStartInfo = NULL;
    pthread_attr_t              thrAttr;
    BOOLEAN                     bThreadAttrInited = FALSE;
    int                         iRetryCnt = 0;

    if (!pThread || !pStartRoutine)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!bJoinThr)
    {
        pthread_attr_init(&thrAttr);
        bThreadAttrInited = TRUE;
        pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);
    }

    dwError = VmDirAllocateMemory(
        sizeof(VMDIR_THREAD_START_INFO),
        ((PVOID*)&pThreadStartInfo)
    );
    BAIL_ON_VMDIR_ERROR(dwError);

    pThreadStartInfo->pStartRoutine = pStartRoutine;
    pThreadStartInfo->pArgs = pArgs;

    do
    {
        dwError = pthread_create(
                        pThread,
                        (bJoinThr ? NULL : &thrAttr),
                        ThreadFunction,
                        pThreadStartInfo
                        );
        if (dwError == EAGAIN)  // no resources, retry after 1 second pause
        {
            iRetryCnt++ ;
            VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "pthread_create EAGAIN, retry (%d)", iRetryCnt );
            VmDirSleep(1 * 1000); // sleep one second
        }
        else
        {
            iRetryCnt = VMDIR_MAX_EAGAIN_RETRY;
        }
    } while (iRetryCnt < VMDIR_MAX_EAGAIN_RETRY);
    BAIL_ON_VMDIR_ERROR(dwError);

    // we started successfully -> pThreadStartInfo is now owned by
    // ThreadFunction
    pThreadStartInfo = NULL;

error:
    if (bThreadAttrInited)
    {
        pthread_attr_destroy(&thrAttr);
    }
    VMDIR_SAFE_FREE_MEMORY(pThreadStartInfo);
    return dwError;
}

DWORD
VmDirThreadJoin(
    PVMDIR_THREAD pThread,
    PDWORD pRetVal
    )
{
    DWORD dwError = ERROR_SUCCESS;
    union
    {
        DWORD dwError;
        PVOID pvRet;
    } retVal;

    memset(&retVal, 0, sizeof(retVal));

    if(pThread == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = pthread_join(
        (*pThread),
        ((pRetVal != NULL) ? &(retVal.pvRet) : NULL)
    );
    BAIL_ON_VMDIR_ERROR(dwError);

    if( pRetVal != NULL )
    {
        // our ThreadFunction returns error code
        *pRetVal = retVal.dwError;
    }

error:

    return dwError;
}

VOID
VmDirFreeVmDirThread(
    PVMDIR_THREAD pThread
    )
{
    if (pThread)
    {
        // on linux nothing to free really
        memset(pThread, 0, sizeof(*pThread));
    }
}

#endif

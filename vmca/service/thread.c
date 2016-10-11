/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

static
VOID
VMCAFreeThread(
    PVMCA_THREAD pThread
    );

static
DWORD
VMCACreateThreadData(
    PVOID              pThrData,
    PVMCA_THREAD_DATA* ppThrData
    );

static
VOID
VMCAShutdownThread(
    PVMCA_THREAD pThread
    );

static
VOID
VMCAFreeThreadData(
    PVMCA_THREAD_DATA pThrData
    );

DWORD
VMCACreateThread(
    PFN_VMCA_THR_FUNC pfnThrFunc,
    PVOID             pData,
    PVMCA_THREAD*     ppThread
    )
{
    DWORD dwError = 0;
    PVMCA_THREAD pThread = NULL;

    if (!pfnThrFunc || !ppThread)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                    sizeof(VMCA_THREAD),
                    (PVOID*)&pThread);
    BAIL_ON_VMCA_ERROR(dwError);

    pThread->refCount = 1;

    dwError = VMCACreateThreadData(
                    pData,
                    &pThread->pThrData);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = pthread_create(
                    &pThread->thread,
                    NULL,
                    pfnThrFunc,
                    pThread->pThrData);
    if (dwError)
    {
#ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
#endif
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pThread->pThread = &pThread->thread;

    *ppThread = pThread;

cleanup:

    return dwError;

error:

    if (ppThread)
    {
        *ppThread = NULL;
    }

    if (pThread)
    {
        VMCAReleaseThread(pThread);
    }

    goto cleanup;
}

PVMCA_THREAD
VMCAAcquireThread(
    PVMCA_THREAD pThread
    )
{
    if (pThread)
    {
        InterlockedIncrement(&pThread->refCount);
    }

    return pThread;
}

DWORD
VMCANotifyThread(
    PVMCA_THREAD pThread
    )
{
    DWORD dwError = 0;
    PVMCA_THREAD_DATA pThrData = pThread ? pThread->pThrData : NULL;

    if (!pThrData || !pThrData->pCond)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = pthread_cond_signal(pThrData->pCond);
    if (dwError)
    {
#ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
#endif
        BAIL_ON_VMCA_ERROR(dwError);
    }

error:

    return dwError;
}

DWORD
VMCAWaitNotifyThread(
    PVMCA_THREAD_DATA pThrData,
    DWORD             dwSecs
    )
{
    DWORD   dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (!pThrData || !pThrData->pMutex || !pThrData->pCond)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (dwSecs > 0)
    {
        BOOLEAN bRetryWait = FALSE;
        struct timespec ts = {0};

        ts.tv_sec = time(NULL) + dwSecs;

        VMCA_LOCK_MUTEX(bLocked, pThrData->pMutex);

        do
        {
            bRetryWait = FALSE;

            dwError = pthread_cond_timedwait(
                            pThrData->pCond,
                            pThrData->pMutex,
                            &ts);
            if (dwError == ETIMEDOUT)
            {
                if (time(NULL) < ts.tv_sec)
                {
                    bRetryWait = TRUE;
                    continue;
                }
            }

        } while (bRetryWait);
    }

error:

    VMCA_UNLOCK_MUTEX(bLocked, pThrData->pMutex);

    return dwError;
}

DWORD
VMCACheckThreadShutdown(
    PVMCA_THREAD_DATA pThrData,
    PBOOLEAN          pbShutdown
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (!pbShutdown || !pThrData)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    VMCA_LOCK_MUTEX(bLocked, pThrData->pMutex);

    *pbShutdown = pThrData->bShutdown;

cleanup:

    VMCA_UNLOCK_MUTEX(bLocked, pThrData->pMutex);

    return dwError;

error:

    if (pbShutdown)
    {
        *pbShutdown = FALSE;
    }

    goto cleanup;
}

VOID
VMCAReleaseThread(
    PVMCA_THREAD pThread
    )
{
    if (pThread && InterlockedDecrement(&pThread->refCount) == 0)
    {
        VMCAFreeThread(pThread);
    }
}

static
VOID
VMCAFreeThread(
    PVMCA_THREAD pThread
    )
{
    if (pThread)
    {
        if (pThread->pThread)
        {
            VMCAShutdownThread(pThread);
        }

        if (pThread->pThrData)
        {
            VMCAFreeThreadData(pThread->pThrData);
        }

        VMCAFreeMemory(pThread);
    }
}

static
DWORD
VMCACreateThreadData(
    PVOID              pData,
    PVMCA_THREAD_DATA* ppThrData
    )
{
    DWORD dwError = 0;
    PVMCA_THREAD_DATA pThrData = NULL;

    dwError = VMCAAllocateMemory(
                    sizeof(VMCA_THREAD_DATA),
                    (PVOID*)&pThrData);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = pthread_mutex_init(&pThrData->mutex, NULL);
    if (dwError)
    {
#ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
#endif
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pThrData->pMutex = &pThrData->mutex;

    dwError = pthread_cond_init(&pThrData->cond, NULL);
    if (dwError)
    {
#ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
#endif
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pThrData->pCond = &pThrData->cond;

    pThrData->pData = pData;

    *ppThrData = pThrData;

cleanup:

    return dwError;

error:

    *ppThrData = NULL;

    if (pThrData)
    {
        VMCAFreeThreadData(pThrData);
    }

    goto cleanup;
}

static
VOID
VMCAShutdownThread(
    PVMCA_THREAD pThread
    )
{
    DWORD dwError = 0;
    PVMCA_THREAD_DATA pThrData = pThread->pThrData;
    BOOLEAN bLocked = FALSE;

    if (pThrData)
    {
        VMCA_LOCK_MUTEX(bLocked, pThrData->pMutex);

        pThrData->bShutdown = TRUE; 

        VMCA_UNLOCK_MUTEX(bLocked, pThrData->pMutex);

        if (pThrData->pCond)
        {
            dwError = pthread_cond_signal(pThrData->pCond);
            if (dwError)
            {
#ifndef _WIN32
                dwError = LwErrnoToWin32Error(dwError);
#endif
                BAIL_ON_VMCA_ERROR(dwError);
            }
        }

        dwError = pthread_join(pThread->thread, NULL);
        if (dwError)
        {
#ifndef _WIN32
            dwError = LwErrnoToWin32Error(dwError);
#endif
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

error:

    // TODO : needs better error handling

    return;
}

static
VOID
VMCAFreeThreadData(
    PVMCA_THREAD_DATA pThrData
    )
{
    if (pThrData)
    {
        if (pThrData->pCond)
        {
            pthread_cond_destroy(&pThrData->cond);
            pThrData->pCond = NULL;
        }

        if (pThrData->pMutex)
        {
            pthread_mutex_destroy(&pThrData->mutex);
            pThrData->pMutex = NULL;
        }

        VMCAFreeMemory(pThrData);
    }
}


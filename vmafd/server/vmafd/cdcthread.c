/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : cdcthread.c
 *
 * Abstract :
 *
 */

#include "includes.h"

DWORD
CdcInitThreadContext(
    PCDC_THREAD_CONTEXT *ppThrContext
    )
{
    DWORD dwError = 0;
    PCDC_THREAD_CONTEXT pThrContext = NULL;

    if (!ppThrContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                          sizeof(CDC_THREAD_CONTEXT),
                          (PVOID*)&pThrContext
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    pThrContext->thr_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pThrContext->thr_cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

    *ppThrContext = pThrContext;

cleanup:

    return dwError;
error:

    if (ppThrContext)
    {
        *ppThrContext = NULL;
    }
    if (pThrContext)
    {
        CdcShutdownThread(pThrContext, NULL);
    }
    goto cleanup;
}

VOID
CdcShutdownThread(
    PCDC_THREAD_CONTEXT pThrContext,
    PSTR pszThreadName
    )
{
    DWORD dwError = 0;

    if (pThrContext && pThrContext->pThread)
    {
        CdcSetShutdownFlagThread(pThrContext);

        dwError = pthread_cond_signal(&pThrContext->thr_cond);
        if (dwError)
        {
            VmAfdLog(
                  VMAFD_DEBUG_ANY,
                  "Condition Signalling %s failed. Error [%d]",
                  IsNullOrEmptyString(pszThreadName)?"":pszThreadName,
                  dwError
                  );
            dwError = 0;
        }

        dwError = pthread_join(*pThrContext->pThread, NULL);
        if (dwError != 0)
        {
            VmAfdLog(
                VMAFD_DEBUG_ANY,
                "CdcShutdownThread Thread join Failed. Error [%d]",
                dwError);
        }

        pthread_cond_destroy(&pThrContext->thr_cond);
        pthread_mutex_destroy(&pThrContext->thr_mutex);
     }

     VMAFD_SAFE_FREE_MEMORY(pThrContext);
}

DWORD
CdcWakeupThread(
    PCDC_THREAD_CONTEXT pThrContext
    )
{
    DWORD dwError = 0;
    if (pThrContext)
    {
        dwError = pthread_cond_signal(&pThrContext->thr_cond);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    return dwError;
error:

    goto cleanup;
}

BOOLEAN
CdcToShutdownThread(
    PCDC_THREAD_CONTEXT pThrContext
    )
{
    BOOLEAN bShutdown = FALSE;

    if (pThrContext)
    {
        pthread_mutex_lock(&pThrContext->thr_mutex);

        bShutdown = pThrContext->bShutdown;

        pthread_mutex_unlock(&pThrContext->thr_mutex);
    }
    return bShutdown;
}

VOID
CdcSetShutdownFlagThread(
    PCDC_THREAD_CONTEXT pThrContext
    )
{
    if (pThrContext)
    {
        pthread_mutex_lock(&pThrContext->thr_mutex);

        pThrContext->bShutdown = TRUE;

        pthread_mutex_unlock(&pThrContext->thr_mutex);
    }
}

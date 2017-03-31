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

static
VOID
VmAfdSetShutdownFlagThr(
    PVMAFD_CERT_THR_DATA pData
    );

static
BOOLEAN
VmAfdGetShutdownFlagThr(
    PVMAFD_CERT_THR_DATA pData
    );

static
VOID
VmAfdHandlePassRefresh(
    );

static
PVOID
VmAfdPassRefreshWorker(
    PVOID pData
    );

DWORD
VmAfdInitPassRefreshThread(
    PVMAFD_THREAD* ppThread
    )
{
   DWORD dwError = 0;
   PVMAFD_THREAD pThread = NULL;

   VmAfdLog(VMAFD_DEBUG_ANY, "Starting Pass Refresh Thread, %s", __FUNCTION__);

   dwError = VmAfdAllocateMemory(
                sizeof(VMAFD_THREAD),
                (PVOID*)&pThread);
   BAIL_ON_VMAFD_ERROR(dwError);

   dwError = pthread_mutex_init(&pThread->thrData.mutex, NULL);
   if (dwError)
   {
#ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
#endif
        BAIL_ON_VMAFD_ERROR(dwError);
   }

   pThread->thrData.pMutex = &pThread->thrData.mutex;

   dwError = pthread_cond_init(&pThread->thrData.cond, NULL);
   if (dwError)
   {
#ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
#endif
        BAIL_ON_VMAFD_ERROR(dwError);
   }

   pThread->thrData.pCond = &pThread->thrData.cond;

   dwError = pthread_create(
                &pThread->thread,
                NULL,
                &VmAfdPassRefreshWorker,
                (PVOID)&pThread->thrData);
    if (dwError)
    {
#ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
#endif
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pThread->pThread = &pThread->thread;

    *ppThread = pThread;
    VmAfdLog(VMAFD_DEBUG_ANY, "Started Pass Refresh Thread successfully, %s", __FUNCTION__);

cleanup:

    return dwError;

error:

    *ppThread = NULL;

    if (pThread)
    {
        VmAfdShutdownPassRefreshThread(pThread);
    }
    goto cleanup;
}

VOID
VmAfdShutdownPassRefreshThread(
    PVMAFD_THREAD pThread
    )
{
    DWORD dwError = 0;

    if (pThread && pThread->pThread && pThread->thrData.pCond)
    {
        VmAfdSetShutdownFlagThr(&pThread->thrData);

        dwError = pthread_cond_signal(pThread->thrData.pCond);
        if (dwError != 0)
        {
            VmAfdLog(VMAFD_DEBUG_ANY, "PassRefresh Condition Signalling Failed. Error [%d]", dwError);
        }

        dwError = pthread_join(*pThread->pThread, NULL);
        if (dwError != 0)
        {
            VmAfdLog(VMAFD_DEBUG_ANY, "PassRefresh Thread join Failed. Error [%d]", dwError);
        }

        if (pThread->thrData.pCond)
        {
            pthread_cond_destroy(pThread->thrData.pCond);
            pThread->thrData.pCond = NULL;
        }

        if (pThread->thrData.pMutex)
        {
            pthread_mutex_destroy(pThread->thrData.pMutex);
            pThread->thrData.pMutex = NULL;
        }
    }

    VmAfdFreeMemory(pThread);
}

static
PVOID
VmAfdPassRefreshWorker(
    PVOID pData
  )
{
    DWORD   dwSleep8Hrs = 60 * 60 * 8;

    PVMAFD_CERT_THR_DATA pThrArgs = (PVMAFD_CERT_THR_DATA)pData;

    while (TRUE)
    {
        BOOLEAN bShutdown = FALSE;
        struct  timespec ts = {0};

        ts.tv_sec = time(NULL) + dwSleep8Hrs;

        pthread_mutex_lock(pThrArgs->pMutex);

        pthread_cond_timedwait(
                        pThrArgs->pCond,
                        pThrArgs->pMutex,
                        &ts);
        // ignore errors.  just loop if timeout,cond signal or spurious wakeup
        pthread_mutex_unlock(pThrArgs->pMutex);

        bShutdown = VmAfdGetShutdownFlagThr(pThrArgs);
        if (bShutdown)
        {
            break;
        }
        VmAfdHandlePassRefresh();
    }

    return NULL;

}

static
VOID
VmAfdHandlePassRefresh(
    )
{
    DWORD   dwError = 0;
    PWSTR   pwszActPassword=NULL;
    PSTR    pszActPassword=NULL;
    PWSTR   pwszDCName = NULL;
    PSTR    pszDCName = NULL;

    PVMAFD_REG_ARG pArgs = NULL;

    VmAfdFreeRegArgs( pArgs );
    pArgs = NULL;

    dwError = VmAfdGetMachineInfo( &pArgs );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetAffinitizedDC(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDCName, &pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirRefreshActPassword( pszDCName,
                                       pArgs->pszDomain,
                                       pArgs->pszAccountUPN,
                                       pArgs->pszAccountDN,
                                       pArgs->pszPassword,
                                       &pszActPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (! IsNullOrEmptyString(pszActPassword))
    {
        // store new password in registry
        dwError = VmAfdAllocateStringWFromA( pszActPassword, &pwszActPassword );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfSrvSetDCActPassword( pwszActPassword );
        BAIL_ON_VMAFD_ERROR(dwError);
        VmAfdLog(VMAFD_DEBUG_ANY, "PassRefresh set reg password passed");
    }


cleanup:

    if(pArgs){
        VmAfdFreeRegArgs( pArgs );
    }
    VMAFD_SAFE_FREE_MEMORY(pszActPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszActPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    return;

error:

    switch (dwError)
    {
        case ERROR_NOT_JOINED:

            VmAfdLog(VMAFD_DEBUG_ANY, "VM not joined yet");

            break;

        default:

            VmAfdLog(
                VMAFD_DEBUG_ANY,
                "PassRefresh failed (%d)",
                dwError);

            break;
    }
    goto cleanup;

}

static
VOID
VmAfdSetShutdownFlagThr(
    PVMAFD_CERT_THR_DATA pData
    )
{
    pthread_mutex_lock(pData->pMutex);

    pData->bShutdown = TRUE;

    pthread_mutex_unlock(pData->pMutex);
}

static
BOOLEAN
VmAfdGetShutdownFlagThr(
    PVMAFD_CERT_THR_DATA pData
    )
{
    BOOLEAN bShutdown = FALSE;

    pthread_mutex_lock(pData->pMutex);

    bShutdown = pData->bShutdown;

    pthread_mutex_unlock(pData->pMutex);

    return bShutdown;
}

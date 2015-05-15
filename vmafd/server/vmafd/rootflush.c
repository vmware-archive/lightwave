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
PVOID
VmAfdFlushTrustedRoots(
    PVOID pData
    );

static
DWORD
VmAfdFlushRoots(
    VOID
    );

static
DWORD
VmAfdSrvFlushRoot(
    PVMAFD_CERT_CONTAINER pCert,
    PCSTR                 pszCAPath
    );

static
DWORD
VmAfdSrvFlushRoot_MD5(
    PCSTR pszCertificate,
    PCSTR pszCAPath
    );

static
DWORD
VmAfdSrvFlushRoot_SHA_1(
    PCSTR pszCertificate,
    PCSTR pszCAPath
    );

static
DWORD
VmAfdSrvWriteRootToDisk(
    PCSTR pszCertificate,
    PCSTR pszCAPath,
    PCSTR pszFilename
    );

static
BOOLEAN
VmAfdToShutdownRootFlushThr(
    PVMAFD_CERT_THR_DATA pData
    );

static
VOID
VmAfdSetShutdownFlagRootFlushThr(
    PVMAFD_CERT_THR_DATA pData
    );

static
DWORD
VmAfdGetRootFlushSleepInterval(
    VOID
    );

static
DWORD
VmAfdRootFlushThrSleep(
    PVMAFD_CERT_THR_DATA pThrData,
    DWORD dwSleepSecs
    );

static
DWORD
VmAfdCheckCertOnDisk(
    PCSTR    pszThumbprint,
    PCSTR    pszCAPath,
    PCSTR    pszFilename,
    LONG     maxIndex,
    PBOOLEAN pbCertOnDisk
    );

DWORD
VmAfdInitRootFlushThread(
    PVMAFD_THREAD* ppThread
    )
{
   DWORD dwError = 0;
   PVMAFD_THREAD pThread = NULL;

   VmAfdLog(VMAFD_DEBUG_ANY, "Starting Roots Flush Thread, %s", __FUNCTION__);

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
                &VmAfdFlushTrustedRoots,
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

    VmAfdLog(VMAFD_DEBUG_ANY, "Started Roots Flush Thread successfully, %s", __FUNCTION__);

cleanup:

    return dwError;

error:

    *ppThread = NULL;

    if (pThread)
    {
        VmAfdShutdownRootFlushThread(pThread);
    }

    goto cleanup;
}

VOID
VmAfdShutdownRootFlushThread(
    PVMAFD_THREAD pThread
    )
{
    DWORD dwError = 0;

    if (pThread && pThread->pThread && pThread->thrData.pCond)
    {
        VmAfdSetShutdownFlagRootFlushThr(&pThread->thrData);

        dwError = pthread_cond_signal(pThread->thrData.pCond);
        if (dwError != 0)
        {
            VmAfdLog(VMAFD_DEBUG_ANY, "Condition Signaling Failed. Error [%d]", dwError);
        }

        dwError = pthread_join(*pThread->pThread, NULL);
        if (dwError != 0)
        {
            VmAfdLog(VMAFD_DEBUG_ANY, "Thread join Failed. Error [%d]", dwError);
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
VmAfdFlushTrustedRoots(
    PVOID pData
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_THR_DATA pThrArgs = (PVMAFD_CERT_THR_DATA)pData;

    while (TRUE)
    {
        DWORD   dwSleepSecs = VmAfdGetRootFlushSleepInterval() ;
        BOOLEAN bShutdown = FALSE;

        bShutdown = VmAfdToShutdownRootFlushThr(pThrArgs);
        if (bShutdown)
        {
            break;
        }

        dwError = VmAfdFlushRoots();
        if (dwError)
        {
            VmAfdLog(
                VMAFD_DEBUG_ANY,
                "Failed to flush trusted roots. Error [%u]",
                dwError);
        }

        dwError = VmAfdRootFlushThrSleep(pThrArgs, dwSleepSecs);
        if (dwError == ETIMEDOUT)
        {
            dwError = 0;
        }
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    return NULL;

error:

    goto cleanup;
}

static
DWORD
VmAfdFlushRoots(
    VOID
    )
{
    DWORD dwError = 0;
    WCHAR wszStoreName[] = TRUSTED_ROOTS_STORE_NAME_W;
    PVECS_SERV_STORE pCertStore = NULL;
    PVECS_SRV_ENUM_CONTEXT pEnumContext = NULL;
    PVMAFD_CERT_ARRAY pCertContainer = NULL;
    PWSTR pwszCAPath = NULL;
    PSTR  pszCAPath = NULL;

    dwError = VmAfSrvGetCAPath(&pwszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszCAPath, &pszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvOpenCertStore(wszStoreName, NULL, &pCertStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvAllocateCertEnumContext(
                    pCertStore,
                    0, /* default */
                    ENTRY_INFO_LEVEL_2,
                    &pEnumContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    do
    {
        DWORD idx = 0;

        if (pCertContainer)
        {
            VmAfdFreeCertArray(pCertContainer);
            pCertContainer = NULL;
        }

        dwError = VecsSrvEnumCertsInternal(pEnumContext, &pCertContainer);
        BAIL_ON_VMAFD_ERROR(dwError);

        for (; idx < pCertContainer->dwCount; idx++)
        {
            PVMAFD_CERT_CONTAINER pCert = &pCertContainer->certificates[idx];

            dwError = VmAfdSrvFlushRoot(pCert, pszCAPath);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

    } while (pCertContainer && pCertContainer->dwCount > 0);

cleanup:

    if (pEnumContext)
    {
        VecsSrvReleaseEnumContext(pEnumContext);
    }
    if (pCertContainer)
    {
        VmAfdFreeCertArray(pCertContainer);
    }
    if (pCertStore)
    {
        VecsSrvReleaseCertStore(pCertStore);
    }
    VMAFD_SAFE_FREE_MEMORY(pwszCAPath);
    VMAFD_SAFE_FREE_MEMORY(pszCAPath);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdSrvFlushRoot(
    PVMAFD_CERT_CONTAINER pCert,
    PCSTR                 pszCAPath
    )
{
    DWORD dwError = 0;
    PSTR  pszCertificate = NULL;

    if (!pCert ||
        IsNullOrEmptyString(pCert->pCert) ||
        IsNullOrEmptyString(pszCAPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringAFromW(pCert->pCert, &pszCertificate);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSrvFlushRoot_SHA_1(pszCertificate, pszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSrvFlushRoot_MD5(pszCertificate, pszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszCertificate);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdSrvFlushRoot_MD5(
    PCSTR pszCertificate,
    PCSTR pszCAPath
    )
{
    DWORD dwError = 0;
    PSTR  pszHash = NULL;

    dwError = VecsComputeCertHash_MD5((PSTR)pszCertificate, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSrvWriteRootToDisk(pszCertificate, pszCAPath, pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszHash);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdSrvFlushRoot_SHA_1(
    PCSTR pszCertificate,
    PCSTR pszCAPath
    )
{
    DWORD dwError = 0;
    PSTR  pszHash = NULL;

    dwError = VecsComputeCertHash_SHA_1((PSTR)pszCertificate, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSrvWriteRootToDisk(pszCertificate, pszCAPath, pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszHash);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdSrvWriteRootToDisk(
    PCSTR pszCertificate,
    PCSTR pszCAPath,
    PCSTR pszFilename
    )
{
    DWORD dwError = 0;
    LONG  maxIndex = -1;
    PSTR  pszPath = NULL;
    PSTR  pszAlias = NULL;
    FILE* pFile = NULL;
    size_t len = 0;
    size_t bytesToWrite = 0;
    BOOLEAN bCertOnDisk = FALSE;
    PCSTR   pszCursor = NULL;

    dwError = VmAfdFindFileIndex(pszCAPath, pszFilename, &maxIndex);
    if (dwError != ERROR_FILE_NOT_FOUND)
    {
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsComputeCertAliasA((PSTR)pszCertificate, &pszAlias);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdCheckCertOnDisk(
                        pszAlias,
                        pszCAPath,
                        pszFilename,
                        maxIndex,
                        &bCertOnDisk);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (bCertOnDisk)
        {
            goto cleanup;
        }
    }

    dwError = VmAfdAllocateStringPrintf(
                    &pszPath,
                    "%s%s%s.%ld",
                    pszCAPath,
                    VMAFD_PATH_SEPARATOR_STR,
                    pszFilename,
                    maxIndex+1);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdOpenFilePath(pszPath, "w", &pFile);
    BAIL_ON_VMAFD_ERROR(dwError);

    len = strlen(pszCertificate);
    bytesToWrite = len;
    pszCursor = pszCertificate;

    while (bytesToWrite > 0)
    {
        size_t bytesWritten = 0;

        if ((bytesWritten = fwrite(pszCursor, 1, len, pFile)) == 0)
        {
#ifndef _WIN32
            dwError = LwErrnoToWin32Error(errno);
#else
            dwError = GetLastError();
#endif
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        pszCursor += bytesWritten;
        bytesToWrite -= bytesWritten;
    }

cleanup:

    if (pFile)
    {
        fclose(pFile);
    }

    VMAFD_SAFE_FREE_MEMORY(pszPath);
    VMAFD_SAFE_FREE_MEMORY(pszAlias);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdWakeupRootFlushThr(
    PVMAFD_CERT_THR_DATA pCertThrData
    )
{
    DWORD dwError = 0;

    if (pCertThrData == NULL) {
        VmAfdLog(VMAFD_DEBUG_ANY, "No Thread Data to Wakeup the Thread.");
        goto cleanup;
    }

    dwError = pthread_cond_signal(pCertThrData->pCond);
    if (dwError != 0)
    {
        VmAfdLog(VMAFD_DEBUG_ANY, "Condition Signaling Failed. Error [%d]", dwError);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
BOOLEAN
VmAfdToShutdownRootFlushThr(
    PVMAFD_CERT_THR_DATA pData
    )
{
    BOOLEAN bShutdown = FALSE;

    pthread_mutex_lock(pData->pMutex);

    bShutdown = pData->bShutdown;

    pthread_mutex_unlock(pData->pMutex);

    return bShutdown;
}

static
VOID
VmAfdSetShutdownFlagRootFlushThr(
    PVMAFD_CERT_THR_DATA pData
    )
{
    pthread_mutex_lock(pData->pMutex);

    pData->bShutdown = TRUE;

    pthread_mutex_unlock(pData->pMutex);
}

static
DWORD
VmAfdGetRootFlushSleepInterval(
    VOID
    )
{
    DWORD dwSecs = 0;

    pthread_mutex_lock(&gVmafdGlobals.mutex);

    dwSecs = gVmafdGlobals.dwCertCheckSec; // TODO : Change

    pthread_mutex_unlock(&gVmafdGlobals.mutex);

    return dwSecs;
}

static
DWORD
VmAfdRootFlushThrSleep(
    PVMAFD_CERT_THR_DATA pThrData,
    DWORD dwSleepSecs
    )
{
    DWORD dwError = 0;
    BOOLEAN bRetryWait = FALSE;
    struct timespec ts = {0};

    ts.tv_sec = time(NULL) + dwSleepSecs;

    pthread_mutex_lock(pThrData->pMutex);

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

    pthread_mutex_unlock(pThrData->pMutex);

    return dwError;
}

static
DWORD
VmAfdCheckCertOnDisk(
    PCSTR    pszAlias,
    PCSTR    pszCAPath,
    PCSTR    pszFilename,
    LONG     maxIndex,
    PBOOLEAN pbCertOnDisk
    )
{
    DWORD dwError = 0;
    LONG  index = 0;
    BOOLEAN bCertOnDisk = FALSE;
    PSTR  pszPath = NULL;
    PSTR  pszAliasOther = NULL;

    // Note : maxIndex starts from 0
    for (; !bCertOnDisk && (index <= maxIndex); index++)
    {
        VMAFD_SAFE_FREE_MEMORY(pszPath);

        dwError = VmAfdAllocateStringPrintf(
                        &pszPath,
                        "%s%s%s.%ld",
                        pszCAPath,
                        VMAFD_PATH_SEPARATOR_STR,
                        pszFilename,
                        index);
        BAIL_ON_VMAFD_ERROR(dwError);

        VMAFD_SAFE_FREE_MEMORY(pszAliasOther);

        dwError = VecsComputeCertAliasFile(pszPath, &pszAliasOther);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (!VmAfdStringCompareA(pszAlias, pszAliasOther, FALSE))
        {
            bCertOnDisk = TRUE;
        }
    }

    *pbCertOnDisk = bCertOnDisk;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszPath);
    VMAFD_SAFE_FREE_MEMORY(pszAliasOther);

    return dwError;

error:

    *pbCertOnDisk = FALSE;

    goto cleanup;
}







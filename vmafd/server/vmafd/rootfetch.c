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
VmAfdHandleCertUpdates(
    PVOID pData
    );

static
BOOLEAN
VmAfdToShutdownCertUpdateThr(
    PVMAFD_CERT_THR_DATA pData
    );

static
VOID
VmAfdSetShutdownFlagCertUpdateThr(
    PVMAFD_CERT_THR_DATA pData
    );

static
DWORD
VmAfdGetSleepInterval(
    VOID
    );

static
DWORD
VmAfdGetStore(
    PCWSTR            pcwszStoreName,
    PVECS_SERV_STORE *ppStore
    );

static
DWORD
VmAfdCertUpdateThrSleep(
    PVMAFD_CERT_THR_DATA pThrData,
    DWORD dwSleepSecs
    );


static
DWORD
VmAfdGetThreadArgs(
    PVMAFD_ROOT_FETCH_ARG *ppArgs
    );

static
VOID
VmAfdFreeThreadArgs(
    PVMAFD_ROOT_FETCH_ARG pArgs
    );

static
VOID
VmAfdRemoveCertFromContainer(
    PVMAFD_CERT_ARRAY   pCertContainer,
    PWSTR               pwszAlias
    );

static
DWORD
VmAfdCleanupFlushedFiles();

static
DWORD
VmAfdDeleteObsoleteCerts(
    PVECS_SERV_STORE    pCertStore,
    PVMAFD_CERT_ARRAY   pCertContainer
    );

static
DWORD
VmAfdFindCertInArray(
    DWORD dwStoreId,
    PVMAFD_CERT_ARRAY pCertContainer,
    PWSTR pwszAlias,
    LONG* pIndexOfFound
    );

static
DWORD
VmAfdSetAutoRefreshOnCert(
    DWORD dwStoreId,
    PVMAFD_CERT_ARRAY pCertContainer,
    LONG index
    );

static
DWORD
VmAfdProcessCACerts(
    PVECS_SERV_STORE     pCertStore,
    CERT_ENTRY_TYPE      entryType,
    PVMAFD_CA_CERT_ARRAY pCACertArray,
    BOOLEAN              bLogOnDuplicate
    );

DWORD
VmAfdInitCertificateThread(
    PVMAFD_THREAD* ppThread
    )
{
   DWORD dwError = 0;
   PVMAFD_THREAD pThread = NULL;

   VmAfdLog(VMAFD_DEBUG_ANY, "Starting Roots Fetch Thread, %s", __FUNCTION__);

   (DWORD)VecsSrvFlushCertsToDisk();

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
                &VmAfdHandleCertUpdates,
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
    VmAfdLog(VMAFD_DEBUG_ANY, "Started Roots Fetch Thread successfully, %s", __FUNCTION__);

cleanup:

    return dwError;

error:

    *ppThread = NULL;

    if (pThread)
    {
        VmAfdShutdownCertificateThread(pThread);
    }
    VmAfdLog(VMAFD_DEBUG_ANY, "Starting Roots Fetch Thread failed with error: [%d]", dwError);
    goto cleanup;
}

VOID
VmAfdShutdownCertificateThread(
    PVMAFD_THREAD pThread
    )
{
    DWORD dwError = 0;

    if (pThread && pThread->pThread && pThread->thrData.pCond)
    {
        VmAfdSetShutdownFlagCertUpdateThr(&pThread->thrData);

        dwError = pthread_cond_signal(pThread->thrData.pCond);
        if (dwError != 0)
        {
            VmAfdLog(VMAFD_DEBUG_ANY, "Condition Signalling Failed. Error [%d]", dwError);
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

DWORD
VmAfdRootFetchTask(
    BOOLEAN bLogOnDuplicate
    )
{
    DWORD dwError = 0;
    LDAP* pLotus = NULL;
    PVMAFD_ROOT_FETCH_ARG   pArgs = NULL;
    PVMAFD_CA_CERT_ARRAY    pCACerts = NULL;
    PVECS_SERV_STORE        pCertStore = NULL;
    PVECS_SERV_STORE        pCrlStore = NULL;
    WCHAR trustedRootsStoreName[] = TRUSTED_ROOTS_STORE_NAME_W;
    WCHAR crlStoreName[] = CRL_STORE_NAME_W;
    BOOL bIsLocked = FALSE;

    dwError = VmAfdGetStore(trustedRootsStoreName, &pCertStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetStore(crlStoreName, &pCrlStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetThreadArgs(&pArgs);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdLDAPConnect(
                pArgs->pszDCName,
                pArgs->nPort,
                pArgs->pszUpn,
                pArgs->pszPassword,
                &pLotus);
    BAIL_ON_VMAFD_ERROR(dwError);

    VMAFD_LOCK_MUTEX(bIsLocked, &gVmafdGlobals.pCertUpdateMutex);

    dwError = VmAfdQueryCACerts(pLotus, NULL, TRUE, &pCACerts);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdProcessCACerts(pCertStore,
                CERT_ENTRY_TYPE_TRUSTED_CERT, pCACerts, bLogOnDuplicate);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdProcessCACerts(pCrlStore,
                CERT_ENTRY_TYPE_REVOKED_CERT_LIST, pCACerts, bLogOnDuplicate);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvFlushSSLCertFromDB(bLogOnDuplicate);
    BAIL_ON_VMAFD_ERROR(dwError);

    VMAFD_UNLOCK_MUTEX(bIsLocked, &gVmafdGlobals.pCertUpdateMutex);

cleanup:

    VMAFD_UNLOCK_MUTEX(bIsLocked, &gVmafdGlobals.pCertUpdateMutex);
    if (pCACerts)
    {
        VecsFreeCACertArray(pCACerts);
    }

    if (pCertStore != NULL)
    {
        VecsSrvReleaseCertStore(pCertStore);
    }

    if (pCrlStore != NULL)
    {
        VecsSrvReleaseCertStore(pCrlStore);
    }

    if (pLotus)
    {
        VmAfdLdapClose(pLotus);
    }

    if (pArgs)
    {
        VmAfdFreeThreadArgs(pArgs);
    }

    return dwError;

error :

    goto cleanup;
}


static
PVOID
VmAfdHandleCertUpdates(
    PVOID pData
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_THR_DATA pThrArgs = (PVMAFD_CERT_THR_DATA)pData;

    while (TRUE)
    {
        DWORD   dwSleepSecs = VmAfdGetSleepInterval() ;
        BOOLEAN bShutdown = FALSE;
        VMAFD_DOMAIN_STATE vmafdState = VMAFD_DOMAIN_STATE_NONE;

        bShutdown = VmAfdToShutdownCertUpdateThr(pThrArgs);
        if (bShutdown)
        {
            break;
        }

        dwError = VmAfSrvGetDomainState (&vmafdState);
        if (dwError)
        {
            vmafdState = VMAFD_DOMAIN_STATE_NONE;
            dwError = 0;
        }

        if (vmafdState == VMAFD_DOMAIN_STATE_CONTROLLER ||
            vmafdState == VMAFD_DOMAIN_STATE_CLIENT)
        {
            dwError = VmAfdRootFetchTask(pThrArgs->forceFlush);
            if (dwError)
            {
                VmAfdLog(
                    VMAFD_DEBUG_ANY,
                    "Failed to update trusted roots. Error [%u]",
                    dwError);
            }
            pThrArgs->forceFlush = FALSE;
        }

        dwError = VmAfdCertUpdateThrSleep(pThrArgs, dwSleepSecs);
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
VmAfdGetStore(
    PCWSTR            pcwszStoreName,
    PVECS_SERV_STORE *ppCertStore
    )
{
    DWORD dwError = 0;
    PVECS_SERV_STORE pCertStore = NULL;

    dwError = VecsSrvOpenCertStore(
                    pcwszStoreName,
                    NULL,
                    &pCertStore
                    );
    BAIL_ON_VMAFD_ERROR(dwError);
    *ppCertStore = pCertStore;

cleanup:

    return dwError;

error:
    if (pCertStore)
    {
        VecsSrvReleaseCertStore(pCertStore);
    }
    goto cleanup;
}

DWORD
VmAfdWakeupCertificateUpdatesThr(
    PVMAFD_THREAD pCertThread,
    BOOLEAN forceFlush
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_THR_DATA pThrData = NULL;

    if (!pCertThread)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pThrData = &pCertThread->thrData;
    pThrData->forceFlush = forceFlush;

    dwError = pthread_cond_signal(pThrData->pCond);
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
VmAfdToShutdownCertUpdateThr(
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
VmAfdSetShutdownFlagCertUpdateThr(
    PVMAFD_CERT_THR_DATA pData
    )
{
    pthread_mutex_lock(pData->pMutex);

    pData->bShutdown = TRUE;

    pthread_mutex_unlock(pData->pMutex);
}

static
DWORD
VmAfdGetSleepInterval(
    VOID
    )
{
    DWORD dwSecs = 0;

    pthread_mutex_lock(&gVmafdGlobals.mutex);

    dwSecs = gVmafdGlobals.dwCertCheckSec;

    pthread_mutex_unlock(&gVmafdGlobals.mutex);

    return dwSecs;
}

static
DWORD
VmAfdCertUpdateThrSleep(
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
VOID
VmAfdRemoveCertFromContainer(
    PVMAFD_CERT_ARRAY   pCertContainer,
    PWSTR               pwszAlias
    )
{
    int i = 0;

    for (; i < pCertContainer->dwCount; ++i)
    {
        if (VmAfdStringIsEqualW((PWSTR)pCertContainer->certificates[i].pAlias, pwszAlias, TRUE))
        {
            // Remove the cert from the container, any cert remaining after
            // the process call finishes will be deemed obsolete and will be
            // removed from VECS and CApath.
            VMAFD_SAFE_FREE_MEMORY(pCertContainer->certificates[i].pCert);
            break;
        }
    }
}

static
DWORD
VmAfdDeleteObsoleteCerts(
    PVECS_SERV_STORE    pCertStore,
    PVMAFD_CERT_ARRAY   pCertContainer
    )
{
    DWORD   dwError = 0;
    int     nNdx = 0;
    PCWSTR  pcwszCert = NULL;

    for (nNdx = 0; nNdx < pCertContainer->dwCount; nNdx++)
    {
        pcwszCert = (PCWSTR)pCertContainer->certificates[nNdx].pCert;
        if (pcwszCert && (BOOL)(pCertContainer->certificates[nNdx].dwAutoRefresh))
        {
            dwError = VecsSrvDeleteCertificate(
                pCertStore,
                pCertContainer->certificates[nNdx].pAlias);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }
cleanup:

    return dwError;

error :

    goto cleanup;
}

// This function deletes flushed cert files that VecsSrvDeleteCertificate
// was not to delete for some reason when it was called. This is called
// periodically from the synch thread to make sure we clean up them
// eventually.
static
DWORD
VmAfdCleanupFlushedFiles()
{
    DWORD   dwError = 0;
    DWORD   dwCount = 0;
    DWORD   nNdx = 0;
    DWORD   dwCertFileNameOffset = 0;
    PWSTR   pwszCAPath = NULL;
    PSTR    pszCAPath = NULL;
    PSTR    pszCertFile = NULL;
    PSTR    pszFlagFile = NULL;
    PSTR*   ppszFiles = NULL;
    BOOLEAN bFound = FALSE;

    dwError = VmAfSrvGetCAPath(&pwszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszCAPath, &pszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdListFilesInDir(pszCAPath, &dwCount, &ppszFiles);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (; nNdx < dwCount; ++nNdx)
    {
        if (VmAfdStringNCompareA(
            ppszFiles[nNdx],
            VECS_DEL_FILE_PREFIX,
            sizeof(VECS_DEL_FILE_PREFIX)/sizeof(CHAR) - 1,
            FALSE))
        {
            continue;
        }

        dwCertFileNameOffset = sizeof(VECS_DEL_FILE_PREFIX) - 1;
        dwError = VmAfdAllocateStringPrintf(&pszCertFile, "%s%s%s",
                        pszCAPath, VMAFD_PATH_SEPARATOR_STR,
                        ppszFiles[nNdx] + dwCertFileNameOffset);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringPrintf(&pszFlagFile, "%s%s%s",
                        pszCAPath, VMAFD_PATH_SEPARATOR_STR,
                        ppszFiles[nNdx]);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdFileExists(pszCertFile, &bFound);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (bFound)
        {
            dwError = VmAfdDeleteFile(pszCertFile);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = VmAfdDeleteFile(pszFlagFile);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VecsFillVacantFileSlot(pszCertFile);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszCAPath);
    VMAFD_SAFE_FREE_MEMORY(pwszCAPath);
    VMAFD_SAFE_FREE_MEMORY(pszCertFile);
    VMAFD_SAFE_FREE_MEMORY(pszFlagFile);

    VmAfdFreeStringArrayCountA(ppszFiles, dwCount);
    ppszFiles = NULL;

    return dwError;

error :
    goto cleanup;
}

static
DWORD
VmAfdFindCertInArray(
    DWORD dwStoreId,
    PVMAFD_CERT_ARRAY pCertContainer,
    PWSTR pwszAlias,
    LONG* pIndexOfFound
    )
{
    DWORD dwError = 0;
    LONG i = 0;
    BOOLEAN bFound = FALSE;

    if (!pCertContainer || !pIndexOfFound)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    for (; i < pCertContainer->dwCount; ++i)
    {
        if (VmAfdStringIsEqualW(
            (PWSTR)pCertContainer->certificates[i].pAlias, pwszAlias, TRUE))
        {
            *pIndexOfFound = i;
            bFound = TRUE;
            break;
        }
    }

    if (!bFound)
    {
        dwError = ERROR_OBJECT_NOT_FOUND;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    return dwError;

error :
    goto cleanup;
}

static
DWORD
VmAfdSetAutoRefreshOnCert(
    DWORD dwStoreId,
    PVMAFD_CERT_ARRAY pCertContainer,
    LONG index
    )
{
    DWORD dwError = 0;

    if (!pCertContainer->certificates[index].dwAutoRefresh)
    {
        // This cert was probably added by CA root initialization
        // via client API, where we block AutoRefresh config,
        // so we better fix it up here to mark it as AutoRefresh
        // because the cert has been published to Lotus.
        // Another possible approach is to add an internal API that
        // allows AutoRefresh parameter.
        dwError = VecsDbSetEntryDwordAttributeByAlias(
                    dwStoreId, pCertContainer->certificates[index].pAlias,
                    ATTR_AUTOREFRESH, 1);
        BAIL_ON_VMAFD_ERROR(dwError);
        pCertContainer->certificates[index].dwAutoRefresh = 1;
    }

cleanup:
    return dwError;

error :
    goto cleanup;
}

static
DWORD
VmAfdProcessCACerts(
    PVECS_SERV_STORE     pStore,
    CERT_ENTRY_TYPE      entryType,
    PVMAFD_CA_CERT_ARRAY pCACertArray,
    BOOLEAN              bLogOnDuplicate
    )
{
    DWORD   dwError = 0;
    LONG    nNdx = 0;
    LONG    nFound = -1;
    PWSTR   pwszAlias = NULL;
    PWSTR   pwszCert = NULL;
    PSTR    pszCert = NULL;
    PVECS_SRV_ENUM_CONTEXT  pEnumContext = NULL;
    PVMAFD_CERT_ARRAY       pVecsCertContainer = NULL;

    if (!pStore || !pCACertArray
        || (entryType != CERT_ENTRY_TYPE_TRUSTED_CERT
            && entryType != CERT_ENTRY_TYPE_REVOKED_CERT_LIST))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsSrvAllocateCertEnumContext(
                    pStore,
                    0, /* default */
                    ENTRY_INFO_LEVEL_2,
                    &pEnumContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsSrvEnumCerts(pEnumContext, &pVecsCertContainer);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (nNdx = 0; nNdx < pCACertArray->dwCount; nNdx++)
    {
        if (entryType == CERT_ENTRY_TYPE_TRUSTED_CERT
            && pCACertArray->pCACerts[nNdx].pCert)
        {
            pszCert = (PSTR) pCACertArray->pCACerts[nNdx].pCert;
            dwError = VmAfdAllocateStringWFromA(pszCert, &pwszCert);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VecsComputeCertAliasW(pwszCert, &pwszAlias);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else if (entryType == CERT_ENTRY_TYPE_REVOKED_CERT_LIST
            && pCACertArray->pCACerts[nNdx].pCrl)
        {
            pszCert = (PSTR) pCACertArray->pCACerts[nNdx].pCrl;
            dwError = VmAfdAllocateStringWFromA(pszCert, &pwszCert);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VecsComputeCrlAliasW(pwszCert, &pwszAlias);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (pwszCert && pwszAlias)
        {
            dwError = VmAfdFindCertInArray(
                            pStore->dwStoreId,
                            pVecsCertContainer,
                            pwszAlias,
                            &nFound);
            if (dwError == ERROR_OBJECT_NOT_FOUND)
            {
                dwError = VecsSrvAddCertificate(
                            pStore,
                            entryType,
                            pwszAlias,
                            pwszCert,
                            NULL,
                            NULL,
                            1);
            }
            else
            {
                BAIL_ON_VMAFD_ERROR(dwError);
                dwError = VmAfdSetAutoRefreshOnCert(pStore->dwStoreId,
                            pVecsCertContainer, nFound);
                BAIL_ON_VMAFD_ERROR(dwError);

                if (bLogOnDuplicate)
                {
                    VmAfdLog(VMAFD_DEBUG_ANY,
                        "VmAfdProcessCACerts: force flushing.");
                }

                if (entryType == CERT_ENTRY_TYPE_TRUSTED_CERT
                        && pStore->dwStoreId == VECS_TRUSTED_ROOT_STORE_ID)
                {
                    dwError = VecsSrvFlushRootCertificate(
                               pStore,
                               pVecsCertContainer->certificates[nFound].pCert,
                               bLogOnDuplicate
                               );
                    BAIL_ON_VMAFD_ERROR (dwError);
                }
                else if (entryType == CERT_ENTRY_TYPE_REVOKED_CERT_LIST
                        && pStore->dwStoreId == VECS_CRL_STORE_ID)
                {
                    dwError = VecsSrvFlushCrl(
                                pStore,
                                pVecsCertContainer->certificates[nFound].pCert,
                                bLogOnDuplicate
                                );
                    BAIL_ON_VMAFD_ERROR (dwError);
                }
            }

            VmAfdRemoveCertFromContainer(pVecsCertContainer, pwszAlias);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        VMAFD_SAFE_FREE_MEMORY(pwszAlias);
        VMAFD_SAFE_FREE_MEMORY(pwszCert);
    }

    dwError = VmAfdDeleteObsoleteCerts(pStore, pVecsCertContainer);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdCleanupFlushedFiles();
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszAlias);
    VMAFD_SAFE_FREE_MEMORY(pwszCert);

    VecsSrvReleaseEnumContext(pEnumContext);

    if (pVecsCertContainer)
    {
        VecsFreeCertArray(pVecsCertContainer);
    }

    return dwError;

error :

    goto cleanup;
}


static
DWORD
VmAfdGetThreadArgs(
    PVMAFD_ROOT_FETCH_ARG *ppArgs
    )
{
    DWORD dwError = 0;
    PWSTR pwszAccountName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountDN = NULL;
    PWSTR pwszDomain = NULL;
    PWSTR pwszDCName = NULL;
    PWSTR pwszUserName = NULL;
    PVMAFD_ROOT_FETCH_ARG pArgs = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE ;

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState == VMAFD_DOMAIN_STATE_NONE)
    {
        dwError = VECS_NOT_JOINED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetAffinitizedDC(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetMachineAccountInfo(
                    &pwszUserName,
                    &pwszPassword,
                    &pwszAccountDN,
                    NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError =  VmAfdAllocateMemory(
                    sizeof(VMAFD_ROOT_FETCH_ARG),
                    (PVOID*)&pArgs);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszDCName,
                    &pArgs->pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszAccountDN,
                    &pArgs->pszAccountDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszPassword,
                    &pArgs->pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                    pwszUserName,
                    &pArgs->pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainName(&pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);
    dwError = VmAfdAllocateStringAFromW(
                    pwszDomain,
                    &pArgs->pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                &pArgs->pszUpn,
                "%s@%s",
                pArgs->pszUserName,
                pArgs->pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    pArgs->nPort = LDAP_PORT;

    if (IsNullOrEmptyString(pArgs->pszDCName))
    {
        dwError = VECS_MISSING_DC_NAME;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pArgs->pszAccountDN) ||
        IsNullOrEmptyString(pArgs->pszPassword))
    {
        dwError = VECS_MISSING_CREDS;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppArgs = pArgs;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszAccountName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszAccountDN);
    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_MEMORY(pwszUserName);

    return dwError;

error :

    *ppArgs = NULL;

    if (pArgs)
    {
        VmAfdFreeThreadArgs(pArgs);
    }

    switch (dwError)
    {
        case VECS_MISSING_CREDS:

            VmAfdLog(VMAFD_DEBUG_ANY, "Account DN / Password missing");

            break;

        case VECS_MISSING_DC_NAME:

            VmAfdLog(VMAFD_DEBUG_ANY, "Invalid domain controller name");

            break;

        default:

            VmAfdLog(
                VMAFD_DEBUG_ANY,
                "Error [%d] fetching cert refresh args",
                dwError);

            break;
    }

    goto cleanup;
}

static
VOID
VmAfdFreeThreadArgs(
    PVMAFD_ROOT_FETCH_ARG pArgs
    )
{
    VMAFD_SAFE_FREE_MEMORY(pArgs->pszUserName);
    VMAFD_SAFE_FREE_MEMORY(pArgs->pszAccountDN);
    VMAFD_SAFE_FREE_MEMORY(pArgs->pszPassword);
    VMAFD_SAFE_FREE_MEMORY(pArgs->pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pArgs->pszUpn);
    VMAFD_SAFE_FREE_MEMORY(pArgs->pszDomain);
    VmAfdFreeMemory(pArgs);
}

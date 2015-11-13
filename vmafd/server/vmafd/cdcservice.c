/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : cdcservice.c
 *
 * Abstract :
 *
 */

#include "includes.h"

static pthread_mutex_t mutexStateChange =
                      (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

static
DWORD
VmAfSrvDirOpenConnection(
    PCWSTR pwszDCName,
    PCWSTR pwszDomain,
    PCWSTR pwszAccount,
    PCWSTR pwszPassword,
    PVMDIR_CONNECTION*ppConnection
    );

static
BOOLEAN
CdcToShutdownDCCacheThread(
    PVMAFD_CERT_THR_DATA pData
    );

static
VOID
CdcSetShutdownFlagDCCacheThread(
    PVMAFD_CERT_THR_DATA pData
    );

static
DWORD
CdcDCCacheThreadSleep(
    PVMAFD_CERT_THR_DATA pThrData,
    DWORD dwRefreshInterval,
    DWORD dwHeartBeat
    );

static
DWORD
CdcGetDomainControllers(
    PWSTR** ppszDomainControllers,
    PDWORD pdwCount
    );

static
PVOID
CdcHandleDCCaching(
    PVOID pData
    );

static
DWORD
CdcVmafdHeartbeatPing(
    PWSTR pwszDCName,
    PWSTR pwszAccount,
    PWSTR pwszPassword,
    PWSTR pwszDomainName,
    PBOOL pbIsAlive
    );

static
DWORD
CdcDCPing(
    PWSTR pszDCName,
    PWSTR pszDomainName,
    PDWORD pdwResult,
    time_t *ptimeTaken,
    PWSTR *ppszSiteName
    );

static
DWORD
CdcUpdateCache(
    BOOL bPurgeRefresh
    );

static
DWORD
CdcPurgeCache(
    VOID
    );

static
DWORD
CdcHandleNoDCList(
    VOID
    );

static
DWORD
CdcGetNewDC(
    PWSTR* ppszNewDCName,
    PCDC_DC_STATE pCdcNextState
    );

static
DWORD
CdcUpdateAffinitizedDC(
    PWSTR pszNewDCName
    );

static
DWORD
CdcHandleSiteAffinitized(
    VOID
    );

static
DWORD
CdcHandleOffSite(
    VOID
    );

static
DWORD
CdcHandleNoDCsAlive(
    VOID
    );

static
DWORD
CdcRefreshCache(
    BOOL bPurgeRefresh,
    PCDC_DB_ENTRY_W pCdcEntry,
    DWORD dwCount
    );

static
DWORD
CdcStateTransition(
    CDC_DC_STATE cdcNextState
    );

static
BOOL
CdcIsValidTransition(
    CDC_DC_STATE cdcCurrentState,
    CDC_DC_STATE cdcNewState
    );

static
DWORD
CdcGetClientSiteName(
    PWSTR* ppszSiteName
    );

static
DWORD
CdcSetClientSiteName(
    PCWSTR pszSiteName
    );

DWORD
CdcInitDCCacheThread(
    PVMAFD_THREAD* ppThread
    )
{
    DWORD dwError = 0;
    PVMAFD_THREAD pThread = NULL;

    VmAfdLog(VMAFD_DEBUG_ANY, "Starting DC Caching Thread, %s", __FUNCTION__);

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
                        &CdcHandleDCCaching,
                        (PVOID)&pThread->thrData
                        );
    if (dwError)
    {
#ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
#endif
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pThread->pThread = &pThread->thread;

    *ppThread = pThread;
    VmAfdLog(VMAFD_DEBUG_ANY, "Started DC Cache Thread successfully, %s", __FUNCTION__);

cleanup:

    return dwError;
error:

    *ppThread = NULL;

    if (pThread)
    {
        CdcShutdownDCCachingThread(pThread);
    }
    goto cleanup;
}

DWORD
CdcRunStateMachine(
    BOOLEAN bPurgeRefresh
    )
{
    DWORD dwError = 0;
    BOOL bHAEnabled = FALSE;
    CDC_DC_STATE cdcCurrentState = CDC_DC_STATE_UNDEFINED;

    dwError = CdcSrvGetCurrentState(&cdcCurrentState);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbGetHAClientState(&bHAEnabled);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bHAEnabled)
    {
        switch (cdcCurrentState)
        {
            case CDC_DC_STATE_DISABLED:
              dwError = CdcDisableClientAffinity();
              break;

            case CDC_DC_STATE_NO_DC_LIST:
              dwError = CdcHandleNoDCList();
              break;

            case CDC_DC_STATE_SITE_AFFINITIZED:
              dwError = CdcUpdateCache(bPurgeRefresh);
              BAIL_ON_VMAFD_ERROR(dwError);

              dwError = CdcHandleSiteAffinitized();
              break;

            case CDC_DC_STATE_OFF_SITE:
              dwError = CdcUpdateCache(bPurgeRefresh);
              BAIL_ON_VMAFD_ERROR(dwError);

              CdcHandleOffSite();
              break;

            case CDC_DC_STATE_NO_DCS_ALIVE:
              dwError = CdcUpdateCache(FALSE);
              BAIL_ON_VMAFD_ERROR(dwError);

              CdcHandleNoDCsAlive();
              break;

            default:
              dwError = ERROR_INVALID_STATE;
              break;
        }

        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    return dwError;
error:

    goto cleanup;
}

VOID
CdcShutdownDCCachingThread(
    PVMAFD_THREAD pThread
    )
{
    DWORD dwError = 0;

    if (pThread && pThread->pThread && pThread->thrData.pCond)
    {
        CdcSetShutdownFlagDCCacheThread(&pThread->thrData);

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
CdcEnableClientAffinity(
    VOID
    )
{
    DWORD dwError = 0;
    BOOL bIsLocked = 0;
    BOOL bEnabledHA = FALSE;

    dwError = CdcRegDbGetHAMode(&bEnabledHA);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!bEnabledHA)
    {
        VMAFD_LOCK_MUTEX(bIsLocked,&mutexStateChange);

        dwError = CdcDbSetHAClientState(CDC_DC_STATE_NO_DC_LIST);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = CdcRegDbSetHAMode(TRUE);
        BAIL_ON_VMAFD_ERROR(dwError);

        VMAFD_UNLOCK_MUTEX(bIsLocked,&mutexStateChange);
    }

    dwError = CdcWakeupDCCachingThread(
                                  gVmafdGlobals.pDCCacheThr,
                                  TRUE
                                  );
    if (dwError)
    {
        VmAfdLog (VMAFD_DEBUG_ANY,
                  "Failed to wake up DC Caching thread: %d",
                  dwError
                 );
        dwError = 0;
    }

cleanup:

    VMAFD_UNLOCK_MUTEX(bIsLocked,&mutexStateChange);
    return dwError;

error:
    goto cleanup;
}

DWORD
CdcDisableClientAffinity(
    VOID
    )
{
    DWORD dwError = 0;

    BOOL bIsLocked = FALSE;

    VMAFD_LOCK_MUTEX(bIsLocked,&mutexStateChange);

    dwError = CdcPurgeCache();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbSetHAClientState(CDC_DC_STATE_DISABLED);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcRegDbSetHAMode(FALSE);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_UNLOCK_MUTEX(bIsLocked,&mutexStateChange);
    return dwError;

error:
    goto cleanup;
}

DWORD
CdcWakeupDCCachingThread(
    PVMAFD_THREAD pDCCachingThread,
    BOOLEAN bPurgeRefresh
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_THR_DATA pThrData = NULL;

    if (!pDCCachingThread)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pThrData = &pDCCachingThread->thrData;
    pThrData->forceFlush = bPurgeRefresh;

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

DWORD
CdcSrvGetDCName(
    PCWSTR pszDomainName,
    PCDC_DC_INFO_W *ppAffinitizedDC
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W pAffinitizedDC = NULL;
    DWORD dwAffinitizedSince = 0;
    BOOL bHAState = FALSE;
    PWSTR pszDomainToUse = NULL;

    if (!ppAffinitizedDC)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDomainName(&pszDomainToUse);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!IsNullOrEmptyString(pszDomainName))
    {
        if (!VmAfdStringIsEqualW(
                            pszDomainName,
                            pszDomainToUse,
                            FALSE
                            )
           )
        {
            dwError = ERROR_INVALID_DOMAINNAME;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    dwError = CdcRegDbGetHAMode(&bHAState);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbGetAffinitizedDC(
                          pszDomainToUse,
                          &dwAffinitizedSince,
                          &pAffinitizedDC
                          );
    if (!bHAState || dwError)
    {
        dwError = VmAfdAllocateMemory(
                                sizeof(CDC_DC_INFO_W),
                                (PVOID *)&pAffinitizedDC
                                );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfSrvGetDCName(&pAffinitizedDC->pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfSrvGetDomainName(&pAffinitizedDC->pszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = CdcGetClientSiteName(&pAffinitizedDC->pszDcSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppAffinitizedDC = pAffinitizedDC;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDomainToUse);
    return dwError;
error:

    if (ppAffinitizedDC)
    {
        *ppAffinitizedDC = NULL;
    }
    if (pAffinitizedDC)
    {
        VmAfdFreeDomainControllerInfoW(pAffinitizedDC);
    }

    goto cleanup;
}

DWORD
CdcSrvGetCurrentState(
    PCDC_DC_STATE pCdcState
    )
{
    DWORD dwError = 0;
    DWORD dwCdcState = 0;
    BOOL bIsHAEnabled = FALSE;
    CDC_DC_STATE cdcState = CDC_DC_STATE_UNDEFINED;

    if (!pCdcState)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcRegDbGetHAMode(&bIsHAEnabled);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbGetHAClientState(&dwCdcState);

    if (!bIsHAEnabled && dwError == ERROR_OBJECT_NOT_FOUND)
    {
        cdcState = CDC_DC_STATE_DISABLED;
        dwError = 0;
    }

    else
    {
        BAIL_ON_VMAFD_ERROR(dwError);
        switch (dwCdcState)
        {
            case CDC_DC_STATE_NO_DC_LIST:
              cdcState = CDC_DC_STATE_NO_DC_LIST;
              break;

            case CDC_DC_STATE_SITE_AFFINITIZED:
              cdcState = CDC_DC_STATE_SITE_AFFINITIZED;
              break;

            case CDC_DC_STATE_OFF_SITE:
              cdcState = CDC_DC_STATE_OFF_SITE;
              break;

            case CDC_DC_STATE_NO_DCS_ALIVE:
              cdcState = CDC_DC_STATE_NO_DCS_ALIVE;
              break;

            case CDC_DC_STATE_DISABLED:
              cdcState = CDC_DC_STATE_DISABLED;
              break;

            default:
              dwError = ERROR_INVALID_STATE;
              break;
        }
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    *pCdcState = cdcState;

cleanup:

    return dwError;
error:

    if (pCdcState)
    {
        *pCdcState = CDC_DC_STATE_UNDEFINED;
    }
    goto cleanup;
}

DWORD
VmAfSrvGetAffinitizedDC(
    PWSTR* ppwszDCName
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W pAffinitizedDC = NULL;
    PWSTR pwszDCName = NULL;

    if (!ppwszDCName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError)
    }

    dwError = CdcSrvGetDCName(NULL, &pAffinitizedDC);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringW(
                              pAffinitizedDC->pszDCName,
                              &pwszDCName
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszDCName = pwszDCName;

cleanup:

    if (pAffinitizedDC)
    {
        VmAfdFreeDomainControllerInfoW(pAffinitizedDC);
    }
    return dwError;
error:

    if (ppwszDCName)
    {
        *ppwszDCName = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    goto cleanup;
}

static
PVOID
CdcHandleDCCaching(
    PVOID pData
    )
{
    DWORD dwError = 0;
    DWORD dwRefreshInterval = 0;
    DWORD dwHeartBeat = 0;
    BOOL  bHAState = FALSE;
    PVMAFD_CERT_THR_DATA pThrArgs = (PVMAFD_CERT_THR_DATA)pData;

    pThrArgs->forceFlush = TRUE;

    while (TRUE)
    {
        BOOLEAN bShutdown = FALSE;
        dwError = CdcRegDbGetRefreshInterval(&dwRefreshInterval);
        if (dwError)
        {
            dwRefreshInterval = DCCA_DEFAULT_SYNC_INTERVAL;
        }

        dwError = CdcRegDbGetHeartBeatInterval(&dwHeartBeat);
        if (dwError)
        {
            dwHeartBeat = DCCA_DEFAULT_HEARTBEAT;
        }

        bShutdown = CdcToShutdownDCCacheThread(pThrArgs);
        if (bShutdown)
        {
            break;
        }

        dwError = CdcRegDbGetHAMode(&bHAState);
        if (dwError)
        {
            bHAState = FALSE;
            dwError = 0;
        }

        if (bHAState)
        {
            dwError = CdcRunStateMachine(pThrArgs->forceFlush);
            if (dwError)
            {
                VmAfdLog(
                      VMAFD_DEBUG_ANY,
                      "Failed to populate the DC Cache. Error [%u]",
                      dwError);
            }
        }

        dwError = CdcDCCacheThreadSleep(
                                pThrArgs,
                                dwRefreshInterval,
                                dwHeartBeat
                                );
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
CdcHandleNoDCList(
    VOID
    )
{
    DWORD dwError = 0;
    PWSTR pszDCName = NULL;
    DWORD dwPingResult = 0;
    PWSTR pszSiteName = NULL;
    time_t timeTaken = 0;

    dwError = VmAfSrvGetDCName(&pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdTrimFQDNTrailingDot(pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDCPing(
                    pszDCName,
                    NULL,
                    &dwPingResult,
                    &timeTaken,
                    &pszSiteName
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!dwPingResult)
    {
        dwError = CdcUpdateCache(TRUE);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = CdcUpdateAffinitizedDC(pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = CdcSetClientSiteName(pszSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = CdcStateTransition(CDC_DC_STATE_SITE_AFFINITIZED);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    return dwError;
error:
    goto cleanup;
}

static
DWORD
CdcHandleSiteAffinitized(
    VOID
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W pCdcDCName = NULL;
    BOOL  bIsAlive = FALSE;
    PWSTR pszNewDCName = NULL;
    CDC_DC_STATE cdcNextState = CDC_DC_STATE_UNDEFINED;

    dwError = CdcSrvGetDCName(NULL,&pCdcDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbIsDCAlive(pCdcDCName, &bIsAlive);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!bIsAlive)
    {
        dwError = CdcGetNewDC(&pszNewDCName, &cdcNextState);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (cdcNextState != CDC_DC_STATE_NO_DCS_ALIVE)
        {
            dwError = CdcUpdateAffinitizedDC(pszNewDCName);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        dwError = CdcStateTransition(cdcNextState);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (pCdcDCName)
    {
        VmAfdFreeDomainControllerInfoW(pCdcDCName);
    }
    VMAFD_SAFE_FREE_MEMORY(pszNewDCName);
    return dwError;
error:

    goto cleanup;
}

static
DWORD
CdcHandleOffSite(
    VOID
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W pCdcDCName = NULL;
    PWSTR pszNextDCName = NULL;
    BOOL bIsAlive = FALSE;
    CDC_DC_STATE cdcNextState = CDC_DC_STATE_UNDEFINED;

    dwError = CdcSrvGetDCName(NULL,&pCdcDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbIsDCAlive(pCdcDCName, &bIsAlive);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcGetNewDC(&pszNextDCName, &cdcNextState);
    BAIL_ON_VMAFD_ERROR(dwError);

    switch (cdcNextState)
    {
        case CDC_DC_STATE_OFF_SITE:
          if (bIsAlive)
          {
              break;
          }
        case CDC_DC_STATE_SITE_AFFINITIZED:
          dwError = CdcUpdateAffinitizedDC(pszNextDCName);
          BAIL_ON_VMAFD_ERROR(dwError);
          break;
        case CDC_DC_STATE_NO_DCS_ALIVE:
          break;
        default:
          dwError = ERROR_INVALID_STATE;
          BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcStateTransition(cdcNextState);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pCdcDCName)
    {
        VmAfdFreeDomainControllerInfoW(pCdcDCName);
    }
    VMAFD_SAFE_FREE_MEMORY(pszNextDCName);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
CdcHandleNoDCsAlive(
    VOID
    )
{
    DWORD dwError = 0;
    PWSTR pszNextDCName = NULL;
    CDC_DC_STATE cdcNextState = CDC_DC_STATE_NO_DCS_ALIVE;

    dwError = CdcGetNewDC(&pszNextDCName, &cdcNextState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (cdcNextState != CDC_DC_STATE_NO_DCS_ALIVE)
    {
        dwError = CdcUpdateAffinitizedDC(
                              pszNextDCName
                              );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = CdcStateTransition(cdcNextState);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszNextDCName);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
CdcGetNewDC(
    PWSTR* ppszNewDCName,
    PCDC_DC_STATE pCdcNextState
    )
{
    DWORD dwError = 0;
    PWSTR pszNewDCName = NULL;
    PWSTR pszSiteName = NULL;
    PWSTR pszDomainName = NULL;
    CDC_DC_STATE cdcNextState = CDC_DC_STATE_UNDEFINED;

    dwError = VmAfSrvGetDomainName(&pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcGetClientSiteName(&pszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbGetClosestDCOnSite(
                                pszSiteName,
                                pszDomainName,
                                &pszNewDCName
                                );
    if (dwError)
    {
        VMAFD_SAFE_FREE_MEMORY(pszNewDCName);

        dwError = CdcDbGetClosestDC(
                                   pszDomainName,
                                   &pszNewDCName
                                   );
        if (dwError)
        {
            cdcNextState = CDC_DC_STATE_NO_DCS_ALIVE;
            dwError = 0;
        }
        else
        {
            cdcNextState = CDC_DC_STATE_OFF_SITE;
        }
    }
    else
    {
        cdcNextState = CDC_DC_STATE_SITE_AFFINITIZED;
    }

    *ppszNewDCName = pszNewDCName;
    *pCdcNextState = cdcNextState;


cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pszSiteName);
    return dwError;

error:

    if (ppszNewDCName)
    {
        *ppszNewDCName = NULL;
    }
    if (pCdcNextState)
    {
        *pCdcNextState = CDC_DC_STATE_UNDEFINED;
    }
    VMAFD_SAFE_FREE_MEMORY(pszNewDCName);

    goto cleanup;
}

static
DWORD
CdcUpdateAffinitizedDC(
    PWSTR pszNewDCName
    )
{
    DWORD dwError = 0;
    PWSTR pszDomainName = NULL;

    dwError = VmAfSrvGetDomainName(&pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbAddAffinitizedDC(
                              pszNewDCName,
                              pszDomainName
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDomainName);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
CdcUpdateCache(
    BOOL bPurgeRefresh
    )
{

    DWORD dwError = 0;
    PWSTR *ppszDomainControllers = NULL;
    DWORD dwDCCount = 0;
    DWORD dwIndex = 0;

    time_t timeTaken = 0;
    DWORD dwPingResult = 0;

    PCDC_DB_ENTRY_W pCdcEntry = NULL;

    if (bPurgeRefresh)
    {
        dwError = CdcGetDomainControllers(
                              &ppszDomainControllers,
                              &dwDCCount
                              );
    }
    if (!bPurgeRefresh || dwError)
    {
        dwError = CdcDbEnumDCEntries(
                               &ppszDomainControllers,
                               &dwDCCount
                               );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (dwDCCount)
    {
        dwError = VmAfdAllocateMemory(
                            sizeof(CDC_DB_ENTRY_W)*dwDCCount,
                            (PVOID *)&pCdcEntry
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        for (; dwIndex < dwDCCount; ++dwIndex)
        {
            dwError = VmAfdAllocateStringW(
                            ppszDomainControllers[dwIndex],
                            &pCdcEntry[dwIndex].pszDCName
                            );
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = CdcDCPing(
                          ppszDomainControllers[dwIndex],
                          NULL, //to accomodate multiple domains in future
                          &dwPingResult,
                          &timeTaken,
                          &pCdcEntry[dwIndex].pszSiteName
                          );

            pCdcEntry[dwIndex].dwPingTime = (DWORD) (timeTaken);

            if (!dwPingResult)
            {
                pCdcEntry[dwIndex].bIsAlive = TRUE;
            }

            pCdcEntry[dwIndex].dwLastPing = (DWORD)time(NULL);

            pCdcEntry[dwIndex].cdcEntryStatus =
                              bPurgeRefresh?CDC_DB_ENTRY_STATUS_NEW:
                              CDC_DB_ENTRY_STATUS_UPDATE;
        }

        dwError = CdcRefreshCache(bPurgeRefresh, pCdcEntry, dwDCCount);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (pCdcEntry)
    {
       VmAfdFreeCdcDbEntryArrayW(pCdcEntry,dwDCCount);
    }
    if (ppszDomainControllers)
    {
        VmAfdFreeStringArrayW(
                    ppszDomainControllers,
                    dwDCCount
                    );
    }
    return dwError;
error:

    goto cleanup;
}

static
DWORD
CdcGetDomainControllers(
      PWSTR** ppszDomainControllers,
      PDWORD pdwCount
      )
{
      DWORD dwError       = 0;
      DWORD dwIndex       = 0;
      PWSTR pwszDomain    = NULL;
      PSTR  pszDomain     = NULL;
      PSTR  pszDCName     = NULL;

      PWSTR pwszAccount   = NULL;
      PSTR  pszAccount    = NULL;
      PWSTR pwszPassword  = NULL;
      PSTR  pszPassword   = NULL;
      PWSTR pwszAccountDN = NULL;
      PSTR  pszAccountDN  = NULL;
      PVMDNS_SERVER_CONTEXT pConnection  = NULL;
      PVMDNS_RECORD_ARRAY   pRecordArray = NULL;
      PWSTR* pszDomainControllers        = NULL;
      PCDC_DC_INFO_W pAffinitizedDC      = NULL;

      if (!ppszDomainControllers || !pdwCount)
      {
          dwError = ERROR_INVALID_PARAMETER;
          BAIL_ON_VMAFD_ERROR(dwError);
      }

      dwError = VmAfSrvGetMachineAccountInfo(
                            &pwszAccount,
                            &pwszPassword,
                            &pwszAccountDN,
                            NULL
                            );
      BAIL_ON_VMAFD_ERROR(dwError);

      dwError = VmAfdAllocateStringAFromW(
                                   pwszAccount,
                                   &pszAccount
                                   );
      BAIL_ON_VMAFD_ERROR(dwError);

      dwError = VmAfdAllocateStringAFromW(
                                  pwszPassword,
                                  &pszPassword
                                  );
      BAIL_ON_VMAFD_ERROR(dwError);

      dwError = VmAfSrvGetDomainName(&pwszDomain);
      BAIL_ON_VMAFD_ERROR(dwError);

      dwError = VmAfdAllocateStringAFromW(
                                pwszDomain,
                                &pszDomain
                                );
      BAIL_ON_VMAFD_ERROR(dwError);

      dwError = CdcSrvGetDCName(pwszDomain,&pAffinitizedDC);
      BAIL_ON_VMAFD_ERROR(dwError);

      dwError = VmAfdAllocateStringAFromW(
                                pAffinitizedDC->pszDCName,
                                &pszDCName
                                );
      BAIL_ON_VMAFD_ERROR(dwError);

      dwError = VmDnsOpenServerA(
                          pszDCName,
                          pszAccount,
                          pszDomain,
                          pszPassword,
                          0,
                          NULL,
                          &pConnection);
      BAIL_ON_VMAFD_ERROR(dwError);

      dwError = VmDnsQueryRecordsA(
                          pConnection,
                          pszDomain,
                          "_ldap._tcp",
                          VMDNS_RR_TYPE_SRV,
                          0,
                          &pRecordArray);
      BAIL_ON_VMAFD_ERROR(dwError);

      if (pRecordArray->dwCount)
      {
          dwError = VmAfdAllocateMemory(
                            sizeof(PWSTR)*pRecordArray->dwCount,
                            (PVOID *)&pszDomainControllers
                            );
          BAIL_ON_VMAFD_ERROR(dwError);
      }

      for (; dwIndex < pRecordArray->dwCount; dwIndex++)
      {
          dwError = VmAfdAllocateStringWFromA(
                                  pRecordArray->Records[dwIndex].Data.SRV.pNameTarget,
                                  &pszDomainControllers[dwIndex]
                                  );
          BAIL_ON_VMAFD_ERROR(dwError);

          dwError = VmAfdTrimFQDNTrailingDot(pszDomainControllers[dwIndex]);
          BAIL_ON_VMAFD_ERROR(dwError);
      }

      dwIndex = 0;

      *ppszDomainControllers = pszDomainControllers;
      *pdwCount = pRecordArray->dwCount;

cleanup:
      VMAFD_SAFE_FREE_MEMORY(pwszDomain);
      VMAFD_SAFE_FREE_MEMORY(pszDomain);
      VMAFD_SAFE_FREE_MEMORY(pszDCName);
      VMAFD_SAFE_FREE_MEMORY(pwszAccount);
      VMAFD_SAFE_FREE_MEMORY(pszAccount);
      VMAFD_SAFE_FREE_MEMORY(pwszPassword);
      VMAFD_SAFE_FREE_MEMORY(pszPassword);
      VMAFD_SAFE_FREE_MEMORY(pwszAccountDN);
      VMAFD_SAFE_FREE_MEMORY(pszAccountDN);

      if (pAffinitizedDC)
      {
          VmAfdFreeDomainControllerInfoW(pAffinitizedDC);
      }
      if (pRecordArray)
      {
          VmDnsFreeRecordArray(pRecordArray);
      }
      if (pConnection)
      {
          VmDnsCloseServer(pConnection);
      }

      return dwError;

error:

      if (ppszDomainControllers)
      {
          *ppszDomainControllers = NULL;
      }
      if (pszDomainControllers)
      {
          VmAfdFreeStringArrayW(pszDomainControllers,pRecordArray->dwCount);
      }

      goto cleanup;
}


static
BOOLEAN
CdcToShutdownDCCacheThread(
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
CdcSetShutdownFlagDCCacheThread(
    PVMAFD_CERT_THR_DATA pData
    )
{
    pthread_mutex_lock(pData->pMutex);

    pData->bShutdown = TRUE;

    pthread_mutex_unlock(pData->pMutex);
}

static
DWORD
CdcDCCacheThreadSleep(
      PVMAFD_CERT_THR_DATA pThrData,
      DWORD dwRefreshInterval,
      DWORD dwHeartBeat
      )
{
      DWORD dwError = 0;
      BOOLEAN bRetryWait = FALSE;
      struct timespec ts = {0};
      static time_t tRefreshInterval = 0;
      BOOLEAN bRefresh = FALSE;

      if (!tRefreshInterval)
      {
          tRefreshInterval = time(NULL) + dwRefreshInterval;
      }

      ts.tv_sec = time(NULL) + dwHeartBeat;

      if (tRefreshInterval <= ts.tv_sec)
      {
          ts.tv_sec = tRefreshInterval;
          bRefresh = TRUE;
      }

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

      pThrData->forceFlush = bRefresh;
      tRefreshInterval -= ts.tv_sec;
      return dwError;
}

static
DWORD
VmAfSrvDirOpenConnection(
    PCWSTR pwszDCName,
    PCWSTR pwszDomain,
    PCWSTR pwszAccount,
    PCWSTR pwszPassword,
    PVMDIR_CONNECTION*ppConnection
    )
{
    DWORD dwError = 0;
    PSTR  pszDCName = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszAccount = NULL;
    PSTR  pszPassword = NULL;
    PSTR  pszURI = NULL;
    PVMDIR_CONNECTION pConnection = NULL;

    dwError = VmAfdAllocateStringAFromW(pwszDCName, &pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDomain, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszAccount, &pszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                &pszURI,
                "ldap://%s:%d",
                pszDCName,
                LDAP_PORT);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirConnectionOpen(
                pszURI,
                pszDomain,
                pszAccount,
                pszPassword,
                &pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppConnection = pConnection;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszURI);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    VMAFD_SAFE_FREE_MEMORY(pszAccount);
    VMAFD_SAFE_FREE_MEMORY(pszPassword);

    return dwError;

error:

    if (ppConnection)
    {
        *ppConnection = NULL;
    }

    if (pConnection)
    {
        VmDirConnectionClose(pConnection);
    }

    goto cleanup;
}

static
DWORD
CdcVmafdHeartbeatPing(
    PWSTR pwszDCName,
    PWSTR pwszAccount,
    PWSTR pwszPassword,
    PWSTR pwszDomainName,
    PBOOL pbIsAlive
    )
{
    DWORD dwError = 0;
    PSTR pszUPN = NULL;
    PSTR pszAccount = NULL;
    PSTR pszDomainName = NULL;
    PWSTR pwszUPN = NULL;
    BOOL bIsAlive = FALSE;

    PVMAFD_SERVER pServer = NULL;
    PVMAFD_HB_STATUS_W pHeartbeatStatus = NULL;

    if (IsNullOrEmptyString(pwszDCName) ||
        IsNullOrEmptyString(pwszAccount) ||
        IsNullOrEmptyString(pwszPassword) ||
        IsNullOrEmptyString(pwszDomainName) ||
        !pbIsAlive
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringAFromW(
                                  pwszAccount,
                                  &pszAccount
                                  );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                                  pwszDomainName,
                                  &pszDomainName
                                  );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                                  &pszUPN,
                                  "%s@%s",
                                  pszAccount,
                                  pszDomainName
                                  );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(
                                 pszUPN,
                                 &pwszUPN
                                 );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdOpenServerW(
                         pwszDCName,
                         pwszUPN,
                         pwszPassword,
                         &pServer
                         );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetHeartbeatStatusW(
                                  pServer,
                                  &pHeartbeatStatus
                                  );
    BAIL_ON_VMAFD_ERROR(dwError);

    bIsAlive = pHeartbeatStatus->bIsAlive? TRUE: FALSE;

    *pbIsAlive = bIsAlive;

cleanup:

    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }
    if (pHeartbeatStatus)
    {
        VmAfdFreeHeartbeatStatusW(pHeartbeatStatus);
    }
    VMAFD_SAFE_FREE_MEMORY(pszUPN);
    VMAFD_SAFE_FREE_MEMORY(pwszUPN);
    VMAFD_SAFE_FREE_MEMORY(pszAccount);
    VMAFD_SAFE_FREE_MEMORY(pszDomainName);

    return dwError;
error:

    if (pbIsAlive)
    {
        *pbIsAlive = FALSE;
    }
    VmAfdLog(VMAFD_DEBUG_ANY,
             "Failed to get heartbeat Status due to Error: %d",
             dwError
            );
    goto cleanup;
}

static
DWORD
CdcDCPing(
    PWSTR pszDCName,
    PWSTR pszDomainName,
    PDWORD pdwResult,
    time_t *ptimeTaken,
    PWSTR *ppszSiteName
    )
{
    DWORD dwError = 0;
    DWORD dwAfdPingResult = 0;
    DWORD dwDirPingResult = 0;

    time_t timeBefore = 0;
    time_t timeAfter = 0;
    time_t timeDiff = 0;

    PWSTR pwszAccount = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountDN = NULL;
    PWSTR pwszDomain = NULL;
    PSTR  pszSiteName = NULL;
    PWSTR pwszSiteName = NULL;
    PVMDIR_CONNECTION pConnection = NULL;

    BOOL bDCIsAlive = FALSE;

    if (!pszDCName || !pdwResult || !ptimeTaken || !ppszSiteName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetMachineAccountInfo(
                          &pwszAccount,
                          &pwszPassword,
                          &pwszAccountDN,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(pszDomainName))
    {
        dwError = VmAfSrvGetDomainName(&pwszDomain);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    else
    {
        dwError = VmAfdAllocateStringW(
                                pszDomainName,
                                &pwszDomain);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    time(&timeBefore);

    dwAfdPingResult = CdcVmafdHeartbeatPing(
                                  pszDCName,
                                  pwszAccount,
                                  pwszPassword,
                                  pwszDomain,
                                  &bDCIsAlive
                                  );

    time(&timeAfter);

    timeDiff = timeAfter-timeBefore;

    dwDirPingResult = VmAfSrvDirOpenConnection(
                        pszDCName,
                        pwszDomain,
                        pwszAccount,
                        pwszPassword,
                        &pConnection
                        );
    if (!dwDirPingResult)
    {
        dwDirPingResult = VmDirGetSiteName(pConnection,&pszSiteName);
    }

    if (dwDirPingResult)
    {
        *ppszSiteName = NULL;
    }
    else
    {
        dwError = VmAfdAllocateStringWFromA(
                                    pszSiteName,
                                    &pwszSiteName
                                    );
        BAIL_ON_VMAFD_ERROR(dwError);
        *ppszSiteName = pwszSiteName;
        pwszSiteName = NULL;
    }

    *ptimeTaken = timeDiff;
    *pdwResult = !(!dwDirPingResult && !dwAfdPingResult && bDCIsAlive);

cleanup:

    if (pConnection)
    {
        VmDirConnectionClose(pConnection);
    }
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszAccountDN);
    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);
    VMAFD_SAFE_FREE_MEMORY(pszSiteName);

    return dwError;
error:
    if (ptimeTaken)
    {
        *ptimeTaken = 0;
    }
    if (ppszSiteName)
    {
      *ppszSiteName = NULL;
    }
    if (pdwResult)
    {
        *pdwResult = 0;
    }
    goto cleanup;
}

static
DWORD
CdcPurgeCache(
    VOID
    )
{
    DWORD dwError = 0;
    PWSTR *ppszEntryNames = NULL;
    DWORD dwDCCount = 0;
    DWORD dwIndex = 0;

    dwError = CdcDbEnumDCEntries(
                        &ppszEntryNames,
                        &dwDCCount
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwDCCount)
    {
        for (; dwIndex<dwDCCount; ++dwIndex)
        {
            dwError = CdcDbDeleteDCEntry(ppszEntryNames[dwIndex]);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

cleanup:

    if (ppszEntryNames)
    {
        VmAfdFreeStringArrayW(ppszEntryNames, dwDCCount);
    }
    return dwError;
error:

    goto cleanup;
}

static
DWORD
CdcRefreshCache(
    BOOL bPurgeRefresh,
    PCDC_DB_ENTRY_W pCdcEntry,
    DWORD dwCount
    )
{
    //TODO: Or simply always purge and repopulate?
    DWORD dwError = 0;
    PWSTR *ppszDomainControllers = NULL;
    DWORD dwDBDCCount = 0;
    DWORD dwDbIndex = 0;
    DWORD dwIndex = 0;
    PWSTR pwszDomainName = NULL;

    if (!pCdcEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDomainName(&pwszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bPurgeRefresh)
    {
        dwError = CdcDbEnumDCEntries(&ppszDomainControllers, &dwDBDCCount);
        BAIL_ON_VMAFD_ERROR(dwError);

        for (; dwDbIndex<dwDBDCCount; ++dwDbIndex)
        {
            BOOL bFoundDC = FALSE;
            for (dwIndex = 0;dwIndex < dwCount; ++dwIndex)
            {
               if (VmAfdStringIsEqualW(
                            ppszDomainControllers[dwDbIndex],
                            pCdcEntry[dwIndex].pszDCName,
                            FALSE
                     )
                  )
               {
                    pCdcEntry[dwIndex].cdcEntryStatus =
                                          CDC_DB_ENTRY_STATUS_UPDATE;
                    bFoundDC = TRUE;
                    break;
               }
            }

            if (!bFoundDC)
            {
               CdcDbDeleteDCEntry(ppszDomainControllers[dwDbIndex]);
            }
        }
    }

    for (dwIndex = 0; dwIndex < dwCount; ++dwIndex)
    {

        dwError = VmAfdAllocateStringW(
                                pwszDomainName,
                                &pCdcEntry[dwIndex].pszDomainName
                                );
        BAIL_ON_VMAFD_ERROR(dwError);

        switch (pCdcEntry[dwIndex].cdcEntryStatus)
        {
            case CDC_DB_ENTRY_STATUS_NEW:
                dwError = CdcDbAddDCEntry(&pCdcEntry[dwIndex]);
                break;

            case CDC_DB_ENTRY_STATUS_UPDATE:
                dwError = CdcDbUpdateDCEntry(&pCdcEntry[dwIndex]);
                break;

            default:
                dwError = ERROR_INVALID_PARAMETER;
                break;
        }
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (ppszDomainControllers)
    {
        VmAfdFreeStringArrayW(ppszDomainControllers, dwDBDCCount);
    }
    VMAFD_SAFE_FREE_MEMORY(pwszDomainName);
    return dwError;
error:

    goto cleanup;
}

static
DWORD
CdcStateTransition(
    CDC_DC_STATE cdcNextState
    )
{
    DWORD dwError = 0;
    CDC_DC_STATE cdcCurrentState = CDC_DC_STATE_UNDEFINED;
    BOOL bIsLocked = FALSE;

    dwError = CdcSrvGetCurrentState(&cdcCurrentState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (CdcIsValidTransition(cdcCurrentState,cdcNextState) &&
        cdcCurrentState != cdcNextState
       )
    {
        dwError = CdcDbSetHAClientState(cdcNextState);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_INVALID_OPERATION;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    VMAFD_UNLOCK_MUTEX(bIsLocked, &mutexStateChange);
    return dwError;
error:

    goto cleanup;
}

static
BOOL
CdcIsValidTransition(
    CDC_DC_STATE cdcCurrentState,
    CDC_DC_STATE cdcNewState
    )
{
    BOOL bIsValidTransaction = FALSE;

    switch (cdcNewState)
    {
        case CDC_DC_STATE_DISABLED:
          bIsValidTransaction = TRUE;
          break;
        case CDC_DC_STATE_NO_DC_LIST:
          if (cdcCurrentState == CDC_DC_STATE_DISABLED)
          {
              bIsValidTransaction = TRUE;
          }
          break;
        case CDC_DC_STATE_SITE_AFFINITIZED:
          if (cdcCurrentState == CDC_DC_STATE_NO_DC_LIST ||
              cdcCurrentState == CDC_DC_STATE_OFF_SITE ||
              cdcCurrentState == CDC_DC_STATE_NO_DCS_ALIVE
             )
          {
              bIsValidTransaction = TRUE;
          }
          break;
        case CDC_DC_STATE_NO_DCS_ALIVE:
        case CDC_DC_STATE_OFF_SITE:
          if (cdcCurrentState == CDC_DC_STATE_OFF_SITE ||
              cdcCurrentState == CDC_DC_STATE_NO_DCS_ALIVE ||
              cdcCurrentState == CDC_DC_STATE_SITE_AFFINITIZED
             )
          {
              bIsValidTransaction = TRUE;
          }
          break;
        default:
          break;
    }

    if (cdcCurrentState == cdcNewState)
    {
        bIsValidTransaction = TRUE;
    }

    return bIsValidTransaction;
}

static
DWORD
CdcSetClientSiteName(
    PCWSTR pszSiteName
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pszSiteName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcRegDbSetSiteName(pszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
DWORD
CdcGetClientSiteName(
    PWSTR* ppszSiteName
    )
{
    DWORD dwError = 0;
    PWSTR pszSiteName = NULL;

    if (!ppszSiteName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcRegDbGetSiteName(&pszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszSiteName = pszSiteName;

cleanup:

    return dwError;
error:

    if (ppszSiteName)
    {
        *ppszSiteName = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY(pszSiteName);
    goto cleanup;
}


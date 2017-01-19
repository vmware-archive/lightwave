/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : cdcupdate.c
 *
 * Abstract :
 *
 */

#include "includes.h"

static
DWORD
CdcDCCacheThreadSleep(
    PCDC_CACHE_UPDATE_CONTEXT pThrData,
    DWORD dwRefreshInterval,
    DWORD dwHeartBeat
    );

static
PVOID
CdcDCPing(
    PVOID pCdcDbEntryArg
    );

static
PVOID
CdcHandleDCCaching(
    PVOID pData
    );

static
DWORD
CdcGetDomainControllers(
    PCDC_DB_ENTRY_W* ppDomainControllers,
    PDWORD pdwCount
    );

static
DWORD
CdcUpdateDBEntry(
    PCDC_DB_ENTRY_W pDCEntry
    );

static
DWORD
CdcUpdateDBCache(
    PCDC_DB_ENTRY_W pDomainControllers,
    DWORD dwDCCount
    );

static
VOID
CdcRefreshCache(
    VOID
    );

static
DWORD
CdcVmafdHeartbeatPing(
    PWSTR pwszDCName,
    PWSTR pwszAccount,
    PWSTR pwszPassword,
    PWSTR pwszDomainName,
    PBOOL pbIsAlive,
    PVMAFD_HB_STATUS_W *ppHeartbeatStatus
    );

static
DWORD
CdcPingDCsAndUpdate(
    BOOL  bSiteAndActive
    );

static
DWORD
CdcUpdateCache(
    PCDC_CACHE_UPDATE_CONTEXT pCdcCacheUpdateContext
    );

DWORD
CdcInitCdcCacheUpdate(
    PCDC_CACHE_UPDATE_CONTEXT *ppContext
    )
{
    DWORD dwError = 0;
    PCDC_CACHE_UPDATE_CONTEXT pContext = NULL;

    VmAfdLog(VMAFD_DEBUG_ANY, "Starting CDC Caching Thread, %s", __FUNCTION__);

    dwError = VmAfdAllocateMemory(
                          sizeof(CDC_CACHE_UPDATE_CONTEXT),
                          (PVOID*)&pContext
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcInitThreadContext(&pContext->pUpdateThrContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    pContext->bRefreshCache = TRUE;

    pContext->update_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pContext->update_cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    pContext->cdcThrState = CDC_UPDATE_THREAD_STATE_UNDEFINED;

    dwError = pthread_create(
                        &pContext->pUpdateThrContext->thread,
                        NULL,
                        &CdcHandleDCCaching,
                        (PVOID)pContext
                        );
    if (dwError)
    {
#ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
#endif
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pContext->pUpdateThrContext->pThread = &pContext->pUpdateThrContext->thread;

    *ppContext = pContext;
    VmAfdLog(VMAFD_DEBUG_ANY, "Started CDC Cache Thread successfully, %s", __FUNCTION__);

cleanup:

    return dwError;
error:


    if (ppContext)
    {
        *ppContext = NULL;
    }
    if (pContext)
    {
        CdcShutdownCdcCacheUpdate(pContext);
    }
    goto cleanup;
}

VOID
CdcShutdownCdcCacheUpdate(
    PCDC_CACHE_UPDATE_CONTEXT pCDCUpdateCtxt
    )
{
    if (pCDCUpdateCtxt)
    {
        if (pCDCUpdateCtxt->pUpdateThrContext)
        {
            CdcShutdownThread(
                        pCDCUpdateCtxt->pUpdateThrContext,
                        "CDC Update Thread"
                        );
        }
    }

    VMAFD_SAFE_FREE_MEMORY(pCDCUpdateCtxt);
}

DWORD
CdcWakeupCdcCacheUpdate(
    PCDC_CACHE_UPDATE_CONTEXT pDCCaching,
    BOOLEAN bPurgeRefresh,
    BOOLEAN bWaitForRefresh
    )
{
    DWORD dwError = 0;
    CDC_UPDATE_THREAD_STATE cdcCurrentState = CDC_UPDATE_THREAD_STATE_UNDEFINED;

    if (!pDCCaching)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pDCCaching->bRefreshCache = bPurgeRefresh;

    if (bWaitForRefresh)
    {
        pthread_mutex_lock(&pDCCaching->update_mutex);
        cdcCurrentState = pDCCaching->cdcThrState;
        if (cdcCurrentState == CDC_UPDATE_THREAD_STATE_UPDATED)
        {
            pDCCaching->cdcThrState = CDC_UPDATE_THREAD_STATE_SIGNALLED;
        }
        pthread_mutex_unlock(&pDCCaching->update_mutex);
    }

    dwError = CdcWakeupThread(pDCCaching->pUpdateThrContext);
    if (dwError != 0)
    {
        VmAfdLog(VMAFD_DEBUG_ANY, "Condition Signaling Update Thread Failed. Error [%d]", dwError);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bWaitForRefresh)
    {
        pthread_mutex_lock(&pDCCaching->update_mutex);
        cdcCurrentState = pDCCaching->cdcThrState;
        if (cdcCurrentState != CDC_UPDATE_THREAD_STATE_UPDATED)
        {
            pthread_cond_wait(
                  &pDCCaching->update_cond,
                  &pDCCaching->update_mutex
                  );
        }
        pthread_mutex_unlock(&pDCCaching->update_mutex);
    }

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
PVOID
CdcDCPing(
    PVOID pCdcDbEntryArg
    )
{
    DWORD dwError = 0;
    DWORD dwAfdPingResult = 0;

    time_t timeBefore = 0;
    time_t timeAfter = 0;
    time_t timeDiff = 0;

    PCDC_DB_ENTRY_W pCdcDbEntry = NULL;
    PVMAFD_HB_STATUS_W pHeartbeatStatus = NULL;

    PWSTR pwszAccount = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountDN = NULL;
    PWSTR pwszDomain = NULL;

    BOOL bDCIsAlive = FALSE;

    if (!pCdcDbEntryArg)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCdcDbEntry = (PCDC_DB_ENTRY_W)pCdcDbEntryArg;

    dwError = VmAfSrvGetMachineAccountInfo(
                          &pwszAccount,
                          &pwszPassword,
                          &pwszAccountDN,
                          NULL
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(pCdcDbEntry->pszDomainName))
    {
        dwError = VmAfSrvGetDomainName(&pwszDomain);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    else
    {
        dwError = VmAfdAllocateStringW(
                                pCdcDbEntry->pszDomainName,
                                &pwszDomain);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    time(&timeBefore);

    dwAfdPingResult = CdcVmafdHeartbeatPing(
                                  pCdcDbEntry->pszDCName,
                                  pwszAccount,
                                  pwszPassword,
                                  pwszDomain,
                                  &bDCIsAlive,
                                  &pHeartbeatStatus
                                  );

    time(&timeAfter);

    timeDiff = timeAfter-timeBefore;

    pCdcDbEntry->dwPingTime = timeDiff;

    pCdcDbEntry->bIsAlive = (!dwAfdPingResult && bDCIsAlive);

    pCdcDbEntry->dwLastPing = time(NULL);

    pCdcDbEntry->dwLastError = dwAfdPingResult;

    dwError = CdcUpdateDBEntry(pCdcDbEntry);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!dwAfdPingResult && pHeartbeatStatus)
    {
        dwError = CdcDbUpdateHeartbeatStatus(
                                pCdcDbEntry,
                                pHeartbeatStatus
                                );
    }
    else
    {
        dwError = CdcDbDeleteHeartbeatStatus(pCdcDbEntry);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    if (CdcIsAffinitizedDC(pCdcDbEntry->pszDCName,
                           pwszDomain) &&
        pCdcDbEntry->bIsAlive == FALSE)
    {
        (DWORD)CdcSrvWakeupStateMachine(gVmafdGlobals.pCdcContext);
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszAccountDN);
    if (pCdcDbEntry)
    {
        VmAfdFreeCdcDbEntry(pCdcDbEntry);
    }
    if (pHeartbeatStatus)
    {
        VmAfdFreeHeartbeatStatusW(pHeartbeatStatus);
    }

    return NULL;
error:

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
    CDC_DB_ENUM_HA_MODE cdcHAMode = CDC_DB_ENUM_HA_MODE_UNDEFINED;
    PCDC_CACHE_UPDATE_CONTEXT pThrArgs = (PCDC_CACHE_UPDATE_CONTEXT)pData;
    CDC_DC_STATE cdcCurrentState = CDC_DC_STATE_UNDEFINED;

    pThrArgs->bRefreshCache = TRUE;

    while (TRUE)
    {
        BOOLEAN bShutdown = FALSE;

        bShutdown = CdcToShutdownThread(pThrArgs->pUpdateThrContext);
        if (bShutdown)
        {
            break;
        }

        pthread_mutex_lock(&pThrArgs->update_mutex);
        pThrArgs->cdcThrState = CDC_UPDATE_THREAD_STATE_POLLING;
        pthread_mutex_unlock(&pThrArgs->update_mutex);

        dwError = CdcRegDbGetRefreshInterval(&dwRefreshInterval);
        if (dwError)
        {
            dwRefreshInterval = CDC_DEFAULT_SYNC_INTERVAL;
        }

        dwError = CdcRegDbGetHeartBeatInterval(&dwHeartBeat);
        if (dwError)
        {
            dwHeartBeat = CDC_DEFAULT_HEARTBEAT;
        }

        dwError = CdcRegDbGetHAMode(&cdcHAMode);
        if (dwError)
        {
            cdcHAMode = CDC_DB_ENUM_HA_MODE_DEFAULT;
            dwError = 0;
        }

        if (cdcHAMode == CDC_DB_ENUM_HA_MODE_DEFAULT)
        {
            if (!CdcSrvGetCurrentState(&cdcCurrentState) &&
                cdcCurrentState != CDC_DC_STATE_UNDEFINED &&
                cdcCurrentState != CDC_DC_STATE_LEGACY)
            {
                dwError = CdcUpdateCache(pThrArgs);
                if (dwError)
                {
                    VmAfdLog(
                          VMAFD_DEBUG_ANY,
                          "Failed to populate the DC Cache. Error [%u]",
                          dwError);
                }
            }
        }

        pthread_mutex_lock(&pThrArgs->update_mutex);
        pThrArgs->cdcThrState = CDC_UPDATE_THREAD_STATE_UPDATED;
        pthread_cond_broadcast(&pThrArgs->update_cond);
        pthread_mutex_unlock(&pThrArgs->update_mutex);

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

    VmAfdLog(
        VMAFD_DEBUG_ANY,
        "Exiting CdcUpdateThread due to error :[%d]",
        dwError
        );
    return NULL;
error:

    goto cleanup;
}

static
DWORD
CdcUpdateCache(
    PCDC_CACHE_UPDATE_CONTEXT pCdcCacheUpdateContext
    )
{

    DWORD dwError = 0;
    //DWORD dwStateMachineError = 0;
    //BOOL bActiveAlive = FALSE;
    BOOL bPingSiteOnly = TRUE;

    if (pCdcCacheUpdateContext->bRefreshCache)
    {
        CdcRefreshCache();
    }

    dwError = CdcPingDCsAndUpdate(bPingSiteOnly);
    BAIL_ON_VMAFD_ERROR(dwError);


    //TODO: Comment out off site refresh for now
    //Fix it properly to wake up state machine in the event when no PSCs are
    //alive

/*
 *    if (!bActiveAlive)
 *    {
 *        BOOL bOffSiteAlive = FALSE;
 *        dwError = CdcPingDCsAndUpdate(!bPingSiteOnly, TRUE, &bOffSiteAlive);
 *        BAIL_ON_VMAFD_ERROR(dwError);
 *
 *        if (!bOffSiteAlive)
 *        {
 *           dwStateMachineError = CdcSrvWakeupStateMachine(gVmafdGlobals.pCdcContext);
 *        }
 *    }
 */
#ifndef CDC_DISABLE_OFFSITE

    if (!pCdcCacheUpdateContext->dwOffsiteRefresh)
    {
        dwError = CdcPingDCsAndUpdate(!bPingSiteOnly);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCdcCacheUpdateContext->dwOffsiteRefresh =
                          (pCdcCacheUpdateContext->dwOffsiteRefresh+1) % 3;
#endif

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
DWORD
CdcGetDomainControllers(
      PCDC_DB_ENTRY_W* ppDomainControllers,
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
      PCDC_DB_ENTRY_W pDomainControllers = NULL;
      PCDC_DC_INFO_W pAffinitizedDC      = NULL;

      if (!ppDomainControllers || !pdwCount)
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

      dwError = CdcSrvGetDCName(pwszDomain, 0, &pAffinitizedDC);
      BAIL_ON_VMAFD_ERROR(dwError);

      if (!VmAfdCheckIfServerIsUp(pAffinitizedDC->pszDCName, 2015))
      {
          dwError = ERROR_HOST_DOWN;
          BAIL_ON_VMAFD_ERROR(dwError);
      }

      dwError = VmAfdAllocateStringAFromW(
                                pAffinitizedDC->pszDCName,
                                &pszDCName
                                );
      BAIL_ON_VMAFD_ERROR(dwError);

      dwError = VmDnsOpenServerWithTimeOutA(
                          pszDCName,
                          pszAccount,
                          pszDomain,
                          pszPassword,
                          0,
                          NULL,
                          RPC_PING_TIMEOUT,
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
                            sizeof(CDC_DB_ENTRY_W)*pRecordArray->dwCount,
                            (PVOID *)&pDomainControllers
                            );
          BAIL_ON_VMAFD_ERROR(dwError);
      }

      for (; dwIndex < pRecordArray->dwCount; dwIndex++)
      {
          dwError = VmAfdAllocateStringWFromA(
                                  pRecordArray->Records[dwIndex].Data.SRV.pNameTarget,
                                  &pDomainControllers[dwIndex].pszDCName
                                  );
          BAIL_ON_VMAFD_ERROR(dwError);

          dwError = VmAfdTrimFQDNTrailingDot(pDomainControllers[dwIndex].pszDCName);
          BAIL_ON_VMAFD_ERROR(dwError);

          dwError = VmAfdAllocateStringW(
                                 pwszDomain,
                                 &pDomainControllers[dwIndex].pszDomainName
                                 );
          BAIL_ON_VMAFD_ERROR(dwError);

          pDomainControllers[dwIndex].cdcEntryStatus = CDC_DB_ENTRY_STATUS_NEW;
      }

      dwIndex = 0;

      *ppDomainControllers = pDomainControllers;
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

      if (ppDomainControllers)
      {
          *ppDomainControllers = NULL;
      }
      if (pDomainControllers)
      {
          VmAfdFreeCdcDbEntriesW(pDomainControllers,pRecordArray->dwCount);
      }
      VmAfdLog(
            VMAFD_DEBUG_ANY,
            "Failed to fetch DNS entries due to error: [%d]", 
            dwError
            );

      goto cleanup;
}

static
DWORD
CdcUpdateDBEntry(
    PCDC_DB_ENTRY_W pDCEntry
    )
{
    DWORD dwError = 0;

    if (!pDCEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    switch (pDCEntry->cdcEntryStatus)
    {
        case CDC_DB_ENTRY_STATUS_NEW:
            dwError = CdcDbAddDCEntry(pDCEntry);
            break;

        case CDC_DB_ENTRY_STATUS_SITE_UPDATE:
            dwError = CdcDbUpdateDCEntryWithSite(pDCEntry);
            break;

        case CDC_DB_ENTRY_STATUS_UPDATE:
            dwError = CdcDbUpdateDCEntry(pDCEntry);

            break;

        case CDC_DB_ENTRY_STATUS_EXISTING:
            break;

        default:
            dwError = ERROR_INVALID_PARAMETER;
            break;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
DWORD
CdcUpdateDBCache(
    PCDC_DB_ENTRY_W pDomainControllers,
    DWORD dwDCCount
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;

    if (!pDomainControllers)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    for (; dwIndex < dwDCCount; ++dwIndex)
    {
        dwError = CdcUpdateDBEntry(&pDomainControllers[dwIndex]);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
VOID
CdcRefreshCache()
{
    DWORD dwError = 0;
    PCDC_DB_ENTRY_W pDomainControllers = NULL;
    PCDC_DB_ENTRY_W pDBDomainControllers = NULL;
    DWORD  dwCount = 0;
    DWORD  dwDBDCCount = 0;
    DWORD  dwDbIndex = 0;
    DWORD  dwIndex = 0;

    dwError = CdcGetDomainControllers(
                            &pDomainControllers,
                            &dwCount
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbEnumDCEntries(&pDBDomainControllers, &dwDBDCCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    for (; dwDbIndex<dwDBDCCount; ++dwDbIndex)
    {
        BOOL bFoundDC = FALSE;
        for (dwIndex = 0;dwIndex < dwCount; ++dwIndex)
        {
           if (VmAfdStringIsEqualW(
                        pDBDomainControllers[dwDbIndex].pszDCName,
                        pDomainControllers[dwIndex].pszDCName,
                        FALSE
                 )
              )
           {
                pDomainControllers[dwIndex].cdcEntryStatus =
                                      CDC_DB_ENTRY_STATUS_EXISTING;
                bFoundDC = TRUE;

                if (IsNullOrEmptyString(
                          pDBDomainControllers[dwDbIndex].pszSiteName
                          )
                   )
                {
                    VMAFD_SAFE_FREE_MEMORY(
                                pDomainControllers[dwIndex].pszSiteName
                                );

                    dwError = VmAfSrvGetSiteNameForDC(
                                        pDomainControllers[dwIndex].pszDCName,
                                        &pDomainControllers[dwIndex].pszSiteName
                                        );

                    if (!dwError)
                    {
                        pDomainControllers[dwIndex].cdcEntryStatus =
                                              CDC_DB_ENTRY_STATUS_SITE_UPDATE;
                    }
                    dwError = 0; //If we cannot get site name it is fine
                }
                break;
           }
        }

        if (!bFoundDC)
        {
           CdcDbDeleteDCEntry(pDBDomainControllers[dwDbIndex].pszDCName);
        }
    }

    for (dwIndex = 0; dwIndex < dwCount; ++dwIndex)
    {
        if (pDomainControllers[dwIndex].cdcEntryStatus ==
                                              CDC_DB_ENTRY_STATUS_NEW
           )
        {
            (DWORD) VmAfSrvGetSiteNameForDC(
                                      pDomainControllers[dwIndex].pszDCName,
                                      &pDomainControllers[dwIndex].pszSiteName
                                      );
        }
    }

    dwError = CdcUpdateDBCache(pDomainControllers, dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pDomainControllers)
    {
        VmAfdFreeCdcDbEntriesW(pDomainControllers, dwCount);
    }
    if (pDBDomainControllers)
    {
        VmAfdFreeCdcDbEntriesW(pDBDomainControllers, dwDBDCCount);
    }

    return;
error:

    goto cleanup;
}

static
DWORD
CdcDCCacheThreadSleep(
      PCDC_CACHE_UPDATE_CONTEXT pContext,
      DWORD dwRefreshInterval,
      DWORD dwHeartBeat
      )
{
      DWORD dwError = 0;
      BOOLEAN bRetryWait = FALSE;
      struct timespec ts = {0};
      static time_t tRefreshInterval = 0;
      BOOLEAN bRefresh = FALSE;
      PCDC_THREAD_CONTEXT pThrData = NULL;

      if (!pContext || !pContext->pUpdateThrContext)
      {
          dwError = ERROR_INVALID_PARAMETER;
          BAIL_ON_VMAFD_ERROR(dwError);
      }

      pThrData = pContext->pUpdateThrContext;

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

      pthread_mutex_lock(&pThrData->thr_mutex);

      do
      {
            bRetryWait = FALSE;

            dwError = pthread_cond_timedwait(
                                &pThrData->thr_cond,
                                &pThrData->thr_mutex,
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

      pthread_mutex_unlock(&pThrData->thr_mutex);

      pContext->bRefreshCache = bRefresh;
      tRefreshInterval -= ts.tv_sec;

cleanup:

      return dwError;
error:

      goto cleanup;
}

static
DWORD
CdcVmafdHeartbeatPing(
    PWSTR pwszDCName,
    PWSTR pwszAccount,
    PWSTR pwszPassword,
    PWSTR pwszDomainName,
    PBOOL pbIsAlive,
    PVMAFD_HB_STATUS_W *ppHeartbeatStatus
    )
{
    DWORD dwError = 0;
    PSTR pszUPN = NULL;
    PWSTR pwszUPN = NULL;
    PSTR pszAccount = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszDCName = NULL;
    BOOL bIsAlive = FALSE;

    PVMAFD_HB_STATUS_W pHeartbeatStatus = NULL;
    PVMAFD_SERVER pServer = NULL;

    if (IsNullOrEmptyString(pwszDCName) ||
        IsNullOrEmptyString(pwszAccount) ||
        IsNullOrEmptyString(pwszPassword) ||
        IsNullOrEmptyString(pwszDomainName) ||
        !pbIsAlive ||
        !ppHeartbeatStatus
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

    (DWORD) VmAfdAllocateStringAFromW(
                                  pwszDCName,
                                  &pszDCName
                                  );

    if (!VmAfdCheckIfServerIsUp(pwszDCName, 2020))
    {
        dwError = ERROR_HOST_DOWN;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdOpenServerWithTimeoutW(
                         pwszDCName,
                         pwszUPN,
                         pwszPassword,
                         RPC_PING_TIMEOUT,
                         &pServer
                         );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetHeartbeatStatusW(
                                  pServer,
                                  &pHeartbeatStatus
                                  );
    BAIL_ON_VMAFD_ERROR(dwError);

    bIsAlive = pHeartbeatStatus->bIsAlive? TRUE: FALSE;

    if (!bIsAlive)
    {
        VmAfdLog(VMAFD_DEBUG_ANY,
                 "PSC [%s] is not Alive for requests",
                 pszDCName?pszDCName:""
                 );
    }

    *pbIsAlive = bIsAlive;
    *ppHeartbeatStatus = pHeartbeatStatus;

cleanup:

    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }
    VMAFD_SAFE_FREE_MEMORY(pszUPN);
    VMAFD_SAFE_FREE_MEMORY(pszAccount);
    VMAFD_SAFE_FREE_MEMORY(pszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pwszUPN);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);

    return dwError;
error:

    if (pbIsAlive)
    {
        *pbIsAlive = FALSE;
    }
    if (ppHeartbeatStatus)
    {
        *ppHeartbeatStatus = NULL;
    }
    if (pHeartbeatStatus)
    {
        VmAfdFreeHeartbeatStatusW(pHeartbeatStatus);
    }

    VmAfdLog(VMAFD_DEBUG_ANY,
             "Failed to get heartbeat status of [%s] due to Error: %d",
             pszDCName?pszDCName:"",
             dwError
            );
    goto cleanup;
}


static
DWORD
CdcPingDCsAndUpdate(
    BOOL  bSiteAndActive
    )
{
    DWORD dwError = 0;
    DWORD dwDCCount = 0;
    DWORD dwIndex = 0;
    PCDC_DB_ENTRY_ARRAY pDomainControllers = NULL;
    PCDC_DB_ENTRY_W pDCCursor = NULL;
    PWSTR pwszSiteName = NULL;
    pthread_t pCdcPingThread;

    dwError = VmAfSrvGetSiteName(&pwszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbEnumDCEntriesFiltered(
                                bSiteAndActive?
                                CDC_DB_ENUM_FILTER_ON_SITE_AND_ACTIVE:
                                CDC_DB_ENUM_FILTER_OFF_SITE,
                                pwszSiteName,
                                &pDomainControllers
                                );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwDCCount = pDomainControllers->dwCount;

    if (dwDCCount)
    {
        PCDC_DB_ENTRY_W *pCdcDbEntries = pDomainControllers->pCdcDbEntries;
        for (; (dwIndex<dwDCCount) && pCdcDbEntries[dwIndex]; ++dwIndex)
        {
            pDCCursor = pCdcDbEntries[dwIndex];

            pDCCursor->cdcEntryStatus = CDC_DB_ENTRY_STATUS_UPDATE;

            if (IsNullOrEmptyString(pDCCursor->pszSiteName))
            {
                VMAFD_SAFE_FREE_MEMORY(pDCCursor->pszSiteName);

                dwError = VmAfSrvGetSiteNameForDC(
                                      pDCCursor->pszDCName,
                                      &pDCCursor->pszSiteName
                                      );

                if (!dwError)
                {
                    pDCCursor->cdcEntryStatus =
                                          CDC_DB_ENTRY_STATUS_SITE_UPDATE;
                    dwError = 0;
                }
            }

            dwError = pthread_create(
                                &pCdcPingThread,
                                NULL,
                                CdcDCPing,
                                (PVOID)pDCCursor
                                );

            if (dwError)
            {
#ifndef _WIN32
              dwError = LwErrnoToWin32Error(dwError);
#endif
              BAIL_ON_VMAFD_ERROR(dwError);
            }

            pDomainControllers->pCdcDbEntries[dwIndex] = NULL;
            //The individual threads will now be responsible for freeing
            //memory
            pthread_detach(pCdcPingThread);
        }
    }

cleanup:

    if (pDomainControllers)
    {
        VmAfdFreeCdcDbEntryArray(pDomainControllers);
    }
    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);
    return dwError;
error:

    goto cleanup;
}

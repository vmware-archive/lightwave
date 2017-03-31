/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : cdcstatemachine.c
 *
 * Abstract :
 *
 */

#include "includes.h"

static
DWORD
CdcStateMachineThreadSleep(
    PCDC_THREAD_CONTEXT pThrData,
    DWORD dwHeartBeat
    );

static
PVOID
CdcStateMachineWorker(
    PVOID pData
    );

static
BOOL
CdcDCIsOffsite(
    PCDC_DC_INFO_W pCdcInfo
    );

static
VOID
CdcLogDCFailure(
    PWSTR pwszDCName
    );

static
VOID
CdcLogAffinitizedDCFailure(
    PCDC_DC_INFO_W pCdcDCName
    );

static
VOID
CdcLogAllDCStates(
    VOID
    );

static
DWORD
CdcHandleNoDCList(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    PCDC_DC_STATE pCdcEndingState
    );

static
DWORD
CdcHandleSiteAffinitized(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    PCDC_DC_STATE pCdcEndingState
    );

#ifndef CDC_DISABLE_OFFSITE
static
DWORD
CdcHandleOffSite(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    PCDC_DC_STATE pCdcEndingState
    );
#endif

static
DWORD
CdcHandleNoDCsAlive(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    PCDC_DC_STATE pCdcEndingState
    );

static
DWORD
CdcHandleLegacyHAState(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    PCDC_DC_STATE pCdcEndingState
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
PCSTR
CdcStateToString(
    CDC_DC_STATE cdcState
    );

static
DWORD
CdcStateTransition(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
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
CdcInitStateMachine(
    PCDC_STATE_MACHINE_CONTEXT* ppStateMachine
    )
{
    DWORD dwError = 0;
    PCDC_STATE_MACHINE_CONTEXT pStateMachine    = NULL;

    VmAfdLog(
            VMAFD_DEBUG_ANY,
             "Starting the CDC State machine, %s",
             __FUNCTION__
            );

    dwError = VmAfdAllocateMemory(
                          sizeof(CDC_STATE_MACHINE_CONTEXT),
                          (PVOID*)&pStateMachine);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcInitThreadContext(&pStateMachine->pStateThrContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    pStateMachine->state_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pStateMachine->update_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pStateMachine->update_cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    pStateMachine->cdcThrState = CDC_STATE_THREAD_STATE_UNDEFINED;
    pStateMachine->bFirstRun = 1;

    dwError = pthread_create(
                        &pStateMachine->pStateThrContext->thread,
                        NULL,
                        &CdcStateMachineWorker,
                        (PVOID)pStateMachine
                        );
    if (dwError)
    {
#ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
#endif
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pStateMachine->pStateThrContext->pThread =
                                  &pStateMachine->pStateThrContext->thread;

    *ppStateMachine = pStateMachine;
    VmAfdLog(VMAFD_DEBUG_ANY, "Started CDC State Machine Thread successfully, %s", __FUNCTION__);

cleanup:

    return dwError;
error:

    if (ppStateMachine)
    {
      *ppStateMachine = NULL;
    }
    if (pStateMachine)
    {
        CdcShutdownStateMachine(pStateMachine);
    }
    goto cleanup;
}

DWORD
CdcEnableStateMachine(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine
    )
{
    DWORD dwError = 0;

    if (!pStateMachine)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcStateTransition(pStateMachine, CDC_DC_STATE_NO_DC_LIST);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
CdcDisableStateMachine(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine
    )
{
    DWORD dwError = 0;

    if (!pStateMachine)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcStateTransition(pStateMachine, CDC_DC_STATE_LEGACY);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcRegDbRemoveDCNameHA();
    if (dwError == ERROR_FILE_NOT_FOUND)
    {
        dwError = 0;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
CdcRunStateMachine(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    PCDC_DC_STATE pCdcEndingState
    )
{
    DWORD dwError = 0;
    CDC_DB_ENUM_HA_MODE cdcHAMode = CDC_DB_ENUM_HA_MODE_UNDEFINED;
    CDC_DC_STATE cdcCurrentState = CDC_DC_STATE_UNDEFINED;
    CDC_DC_STATE cdcEndingState = CDC_DC_STATE_UNDEFINED;
    UINT64 iStart = 0;
    UINT64 iEnd = 0;
    PCDC_DC_INFO_W pCdcInfo = NULL;

    if (!pStateMachine || !pCdcEndingState)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    iStart = VmAfdGetTimeInMilliSec();
    dwError = CdcSrvGetCurrentState(&cdcCurrentState);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcRegDbGetHAMode(&cdcHAMode);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (cdcHAMode == CDC_DB_ENUM_HA_MODE_DEFAULT)
    {
        switch (cdcCurrentState)
        {
            case CDC_DC_STATE_NO_DC_LIST:
              dwError = CdcHandleNoDCList(pStateMachine, &cdcEndingState);
              break;

            case CDC_DC_STATE_SITE_AFFINITIZED:
              dwError = CdcHandleSiteAffinitized(pStateMachine, &cdcEndingState);
              break;
#ifndef CDC_DISABLE_OFFSITE
            case CDC_DC_STATE_OFF_SITE:
              CdcHandleOffSite(pStateMachine, &cdcEndingState);
              break;
#endif
            case CDC_DC_STATE_NO_DCS_ALIVE:
              dwError = CdcHandleNoDCsAlive(pStateMachine, &cdcEndingState);
              break;

            case CDC_DC_STATE_LEGACY:
              dwError = CdcHandleLegacyHAState(pStateMachine, &cdcEndingState);
              break;

            default:
              dwError = ERROR_INVALID_STATE;
              break;
        }

        InterlockedExchange(&pStateMachine->bFirstRun,0);

        iEnd = VmAfdGetTimeInMilliSec();

        dwError = CdcSrvGetDCName(NULL, 0, &pCdcInfo);
        BAIL_ON_VMAFD_ERROR(dwError);

        (DWORD)VmAfdAddCDCSuperLogEntry(
                    gVmafdGlobals.pLogger,
                    iStart,
                    iEnd,
                    pCdcInfo,
                    cdcEndingState,
                    dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *pCdcEndingState = cdcEndingState;

cleanup:

    if (pCdcInfo)
    {
        VmAfdFreeDomainControllerInfoW(pCdcInfo);
    }

    return dwError;
error:

    if (pCdcEndingState)
    {
        *pCdcEndingState = CDC_DC_STATE_UNDEFINED;
    }
    goto cleanup;
}

VOID
CdcShutdownStateMachine(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine
    )
{
    if (pStateMachine)
    {
        if (pStateMachine->pStateThrContext)
        {
            CdcShutdownThread(
                          pStateMachine->pStateThrContext,
                          "State machine thread"
                          );
        }

        pthread_mutex_destroy(&pStateMachine->state_mutex);
    }
    VMAFD_SAFE_FREE_MEMORY(pStateMachine);
}

DWORD
CdcWakeupStateMachine(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    BOOLEAN                    bWaitForCompletion
    )
{
    DWORD dwError = 0;
    CDC_STATE_THREAD_STATE cdcCurrentState = CDC_STATE_THREAD_STATE_UNDEFINED;

    if (!pStateMachine)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (bWaitForCompletion)
    {
        pthread_mutex_lock(&pStateMachine->update_mutex);
        cdcCurrentState = pStateMachine->cdcThrState;
        if (cdcCurrentState == CDC_STATE_THREAD_STATE_UPDATED)
        {
            pStateMachine->cdcThrState = CDC_STATE_THREAD_STATE_SIGNALLED;
        }
        pthread_mutex_unlock(&pStateMachine->update_mutex);
    }

    dwError = CdcWakeupThread(pStateMachine->pStateThrContext);
    if (dwError != 0)
    {
        VmAfdLog(
              VMAFD_DEBUG_ANY,
              "Condition Signaling of State machine thread failed. Error [%d]",
              dwError
              );
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bWaitForCompletion)
    {
        pthread_mutex_lock(&pStateMachine->update_mutex);
        cdcCurrentState = pStateMachine->cdcThrState;
        if (cdcCurrentState != CDC_STATE_THREAD_STATE_UPDATED)
        {
            pthread_cond_wait(
                  &pStateMachine->update_cond,
                  &pStateMachine->update_mutex
                  );
        }
        pthread_mutex_unlock(&pStateMachine->update_mutex);
    }


cleanup:

    return dwError;
error:

    goto cleanup;
}


static
PVOID
CdcStateMachineWorker(
    PVOID pData
    )
{
    DWORD dwError = 0;
    DWORD dwStateMachineInterval = 0;
    CDC_DB_ENUM_HA_MODE cdcHAMode = CDC_DB_ENUM_HA_MODE_UNDEFINED;
    CDC_DC_STATE cdcCurrentState = CDC_DC_STATE_UNDEFINED;
    CDC_DC_STATE cdcEndingState = CDC_DC_STATE_UNDEFINED;
    PCDC_STATE_MACHINE_CONTEXT pStateMachine =
                                    (PCDC_STATE_MACHINE_CONTEXT)pData;
    while (TRUE && pStateMachine)
    {
        BOOLEAN bShutdown = FALSE;
        BOOL bCanRunStateMachine = TRUE;

        bShutdown = CdcToShutdownThread(pStateMachine->pStateThrContext);
        if (bShutdown)
        {
            break;
        }

        pthread_mutex_lock(&pStateMachine->update_mutex);
        pStateMachine->cdcThrState = CDC_STATE_THREAD_STATE_RUNNING;
        pthread_mutex_unlock(&pStateMachine->update_mutex);

        dwError = CdcRegDbGetHeartBeatInterval(&dwStateMachineInterval);
        if (dwError)
        {
            dwStateMachineInterval = CDC_DEFAULT_HEARTBEAT;
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
                cdcCurrentState == CDC_DC_STATE_LEGACY)
            {
                if(VmAfdCheckDomainFunctionalLevel(
                                        SSO_HA_MIN_DOMAIN_LEVEL_MAJOR,
                                        SSO_HA_MIN_DOMAIN_LEVEL_MINOR))
                {
                    bCanRunStateMachine = FALSE;
                }
            }

            if (bCanRunStateMachine)
            {
                dwError = CdcRunStateMachine(pStateMachine, &cdcEndingState);
                if (cdcEndingState == CDC_DC_STATE_NO_DC_LIST)
                {
                    continue;
                }
            }
        }

        pthread_mutex_lock(&pStateMachine->update_mutex);
        pStateMachine->cdcThrState = CDC_STATE_THREAD_STATE_UPDATED;
        pthread_cond_broadcast(&pStateMachine->update_cond);
        pthread_mutex_unlock(&pStateMachine->update_mutex);

        dwError = CdcStateMachineThreadSleep(
                                pStateMachine->pStateThrContext,
                                dwStateMachineInterval
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
        "Exiting CdcStateMachine thread due to error: [%d]",
        dwError
        );
    return NULL;
error:

    goto cleanup;
}

static
BOOL
CdcDCIsOffsite(
    PCDC_DC_INFO_W pCdcInfo
    )
{
    BOOL bIsOffsite = TRUE;

    PWSTR pwszClientSiteName = NULL;
    PWSTR pwszDCSiteName = NULL;

    DWORD dwError = 0;

    if (!pCdcInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pCdcInfo->pszDcSiteName))
    {
        dwError = VmAfSrvGetSiteNameForDC(
                              pCdcInfo->pszDCName,
                              &pwszDCSiteName
                              );
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdAllocateStringW(
                              pCdcInfo->pszDcSiteName,
                              &pwszDCSiteName
                              );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetSiteName(&pwszClientSiteName);
    if (
        dwError == ERROR_FILE_NOT_FOUND ||
        dwError == ERROR_OBJECT_NOT_FOUND
       )
    {
        bIsOffsite = FALSE;
        dwError = CdcSetClientSiteName(pwszClientSiteName);
        dwError = 0;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    if (VmAfdStringIsEqualW(pwszClientSiteName, pwszDCSiteName, FALSE))
    {
        bIsOffsite = FALSE;
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszClientSiteName);
    VMAFD_SAFE_FREE_MEMORY(pwszDCSiteName);
    return bIsOffsite;
error:
    goto cleanup;
}

static
VOID
CdcLogDCFailure(
    PWSTR pwszDCName
    )
{
    DWORD dwIndex = 0;
    DWORD dwError = 0;
    PSTR  pszDCName = NULL;
    PSTR  pszServiceName = 0;
    PCDC_DC_STATUS_INFO_W pCdcStatus = NULL;
    PVMAFD_HB_STATUS_W pHeartbeatStatus = NULL;

    dwError = VmAfdAllocateStringAFromW(
                                pwszDCName,
                                &pszDCName
                                );
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_ANY, "PSC: [%s] is down", pszDCName);

    dwError = CdcSrvGetDCStatusInfo(
                            pwszDCName,
                            NULL,
                            &pCdcStatus,
                            &pHeartbeatStatus
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(
         VMAFD_DEBUG_ANY,
         "Last error: [%d]",
         pCdcStatus->dwLastError
         );

    if (pHeartbeatStatus)
    {
        for (; dwIndex < pHeartbeatStatus->dwCount; ++dwIndex)
        {
            PVMAFD_HB_INFO_W pCursor = &pHeartbeatStatus->pHeartbeatInfoArr[dwIndex];
            dwError = VmAfdAllocateStringAFromW(
                                        pCursor->pszServiceName,
                                        &pszServiceName
                                        );
            BAIL_ON_VMAFD_ERROR(dwError);
            VmAfdLog(
              VMAFD_DEBUG_ANY,
              "Service Name: [%s] \t IsAlive: [%s] \t Last Response: [%d]",
               pszServiceName,
               pCursor->bIsAlive?"YES":"NO",
               pCursor->dwLastHeartbeat
               );
            VMAFD_SAFE_FREE_MEMORY(pszServiceName);
        }
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszServiceName);
    if (pCdcStatus)
    {
        VmAfdFreeCdcStatusInfoW(pCdcStatus);
    }

    if (pHeartbeatStatus)
    {
        VmAfdFreeHbStatusW(pHeartbeatStatus);
    }
    return;
error:

    goto cleanup;
}

static
VOID
CdcLogAffinitizedDCFailure(
    PCDC_DC_INFO_W pCdcDCName
    )
{
    VmAfdLog(
         VMAFD_DEBUG_ANY,
         "Current affinitized DC is down"
         );

    CdcLogDCFailure(pCdcDCName->pszDCName);
    return;

}

static
VOID
CdcLogAllDCStates(
    VOID
    )
{
    PWSTR *ppszDCNames = NULL;
    DWORD dwCount = 0;

    (DWORD)CdcSrvEnumDCEntries(
                        &ppszDCNames,
                        &dwCount
                        );
    if (ppszDCNames && dwCount)
    {
        DWORD dwIndex = 0;
        for (; dwIndex<dwCount; ++dwIndex)
        {
            CdcLogDCFailure(ppszDCNames[dwIndex]);
        }

    }

    if (ppszDCNames)
    {
        VmAfdFreeStringArrayW(ppszDCNames, dwCount);
    }
}

static
DWORD
CdcHandleNoDCList(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    PCDC_DC_STATE pCdcEndingState
    )
{
    DWORD dwError = 0;
    DWORD dwLogError = 0;
    PSTR  pszDCName = NULL;
    BOOL bIsAlive = FALSE;
    PCDC_DC_INFO_W pCdcInfo = NULL;
    UINT64 iStart = 0;
    UINT64 iEnd = 0;
    CDC_DC_STATE cdcEndingState = CDC_DC_STATE_UNDEFINED;

    iStart = VmAfdGetTimeInMilliSec();
    dwError = CdcSrvGetDCName(NULL, 0, &pCdcInfo);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbIsDCAlive(pCdcInfo, &bIsAlive);

    if (!dwError && bIsAlive)
    {
        dwError = CdcUpdateAffinitizedDC(pCdcInfo->pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (CdcDCIsOffsite(pCdcInfo))
        {
            dwError = CdcStateTransition(
                               pStateMachine,
                               CDC_DC_STATE_OFF_SITE
                               );
            BAIL_ON_VMAFD_ERROR(dwError);
            cdcEndingState = CDC_DC_STATE_OFF_SITE;
        }
        else
        {
            dwError = CdcStateTransition(
                               pStateMachine,
                               CDC_DC_STATE_SITE_AFFINITIZED
                               );
            BAIL_ON_VMAFD_ERROR(dwError);
            cdcEndingState = CDC_DC_STATE_SITE_AFFINITIZED;
        }

        dwLogError = VmAfdAllocateStringAFromW(
                                      pCdcInfo->pszDCName,
                                      &pszDCName
                                      );
        if (!dwLogError)
        {
            VmAfdLog(
                VMAFD_DEBUG_ANY,
                "Affinitized to [%s]",
                pszDCName
                );
        }
    }

    iEnd = VmAfdGetTimeInMilliSec();
    (DWORD)VmAfdAddCDCSuperLogEntry(
                            gVmafdGlobals.pLogger,
                            iStart,
                            iEnd,
                            pCdcInfo,
                            cdcEndingState,
                            dwError);

    *pCdcEndingState = cdcEndingState;

cleanup:

    if (pCdcInfo)
    {
        VmAfdFreeDomainControllerInfoW(pCdcInfo);
    }
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    return dwError;
error:

    if(pCdcEndingState)
    {
        *pCdcEndingState = CDC_DC_STATE_UNDEFINED;
    }
    VmAfdLog(
        VMAFD_DEBUG_ERROR,
        "Handling state No DC List failed with error [%d]",
        dwError
        );

    goto cleanup;
}

static
DWORD
CdcHandleSiteAffinitized(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    PCDC_DC_STATE pCdcEndingState
    )
{
    DWORD dwError = 0;
    DWORD dwLogError = 0;
    PCDC_DC_INFO_W pCdcDCName = NULL;
    BOOL  bIsAlive = FALSE;
    PWSTR pszNewDCName = NULL;

    PSTR pszLogNewDCName = NULL;
    PSTR pszLogOldDCName = NULL;
    CDC_DC_STATE cdcNextState = CDC_DC_STATE_UNDEFINED;
    UINT64 iStart = 0;
    UINT64 iEnd = 0;

    iStart = VmAfdGetTimeInMilliSec();
    dwError = CdcSrvGetDCName(NULL, 0, &pCdcDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbIsDCAlive(pCdcDCName, &bIsAlive);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!bIsAlive || InterlockedExchange(&pStateMachine->bFirstRun,0))
    {
        if (!bIsAlive)
        {
              CdcLogAffinitizedDCFailure(pCdcDCName);
        }
        dwError = CdcGetNewDC(&pszNewDCName, &cdcNextState);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (cdcNextState != CDC_DC_STATE_NO_DCS_ALIVE)
        {
            dwError = CdcUpdateAffinitizedDC(pszNewDCName);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwLogError = VmAfdAllocateStringAFromW(
                                          pszNewDCName,
                                          &pszLogNewDCName
                                          ) ||
                         VmAfdAllocateStringAFromW(
                                          pCdcDCName->pszDCName,
                                          &pszLogOldDCName
                                          );
            if (!dwLogError)
            {
                VmAfdLog(
                    VMAFD_DEBUG_ANY,
                    "Affinitized to [%s]. Previously affinitized to [%s]",
                    pszLogNewDCName,
                    pszLogOldDCName
                    );
            }
        }
        else if (cdcNextState == CDC_DC_STATE_NO_DCS_ALIVE)
        {
            CdcLogAllDCStates();
        }

        dwError = CdcStateTransition(pStateMachine,cdcNextState);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    iEnd = VmAfdGetTimeInMilliSec();
    (DWORD)VmAfdAddCDCSuperLogEntry(
                    gVmafdGlobals.pLogger,
                    iStart,
                    iEnd,
                    pCdcDCName,
                    cdcNextState,
                    dwError);

    *pCdcEndingState = cdcNextState;

cleanup:

    if (pCdcDCName)
    {
        VmAfdFreeDomainControllerInfoW(pCdcDCName);
    }
    VMAFD_SAFE_FREE_MEMORY(pszLogOldDCName);
    VMAFD_SAFE_FREE_MEMORY(pszLogNewDCName);
    VMAFD_SAFE_FREE_MEMORY(pszNewDCName);
    return dwError;
error:

    if (pCdcEndingState)
    {
        *pCdcEndingState = CDC_DC_STATE_UNDEFINED;
    }
    VmAfdLog(
          VMAFD_DEBUG_ERROR,
          "Handling state SiteAffinitized failed with error [%d]",
          dwError
          );

    goto cleanup;
}

#ifndef CDC_DISABLE_OFFSITE
static
DWORD
CdcHandleOffSite(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    PCDC_DC_STATE pCdcEndingState
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W pCdcDCName = NULL;
    PWSTR pszNextDCName = NULL;
    BOOL bIsAlive = FALSE;
    CDC_DC_STATE cdcNextState = CDC_DC_STATE_UNDEFINED;
    UINT64 iStart = 0;
    UINT64 iEnd = 0;

    iStart = VmAfdGetTimeInMilliSec();

    dwError = CdcSrvGetDCName(NULL,0, &pCdcDCName);
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

          VmAfdLog(
               VMAFD_DEBUG_ANY,
               "Affinitized to [%s]. Previously affinitized to [%s]",
               pszNextDCName,
               pCdcDCName->pszDCName
               );

          break;
        case CDC_DC_STATE_NO_DCS_ALIVE:
          break;
        default:
          dwError = ERROR_INVALID_STATE;
          BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcStateTransition(pStateMachine,cdcNextState);
    BAIL_ON_VMAFD_ERROR(dwError);

    iEnd = VmAfdGetTimeInMilliSec();
    (DWORD)VmAfdAddCDCSuperLogEntry(
                    gVmafdGlobals.pLogger,
                    iStart,
                    iEnd,
                    pCdcDCName,
                    cdcNextState,
                    dwError);

    *pCdcEndingState = cdcNextState;

cleanup:

    if (pCdcDCName)
    {
        VmAfdFreeDomainControllerInfoW(pCdcDCName);
    }
    VMAFD_SAFE_FREE_MEMORY(pszNextDCName);
    return dwError;

error:

    if (pCdcEndingState)
    {
        *pCdcEndingState = CDC_DC_STATE_UNDEFINED;
    }
    VmAfdLog(
          VMAFD_DEBUG_ERROR,
          "Handling State OffSite failed with error [%d]",
          dwError
          );

    goto cleanup;
}
#endif

static
DWORD
CdcHandleNoDCsAlive(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    PCDC_DC_STATE pCdcEndingState
    )
{
    DWORD dwError = 0;
    DWORD dwLogError = 0;
    PWSTR pszNextDCName = NULL;
    PSTR pszLogNextDCName = NULL;
    CDC_DC_STATE cdcNextState = CDC_DC_STATE_NO_DCS_ALIVE;
    UINT64 iStart = 0;
    UINT64 iEnd = 0;
    PCDC_DC_INFO_W pCdcInfo = NULL;

    iStart = VmAfdGetTimeInMilliSec();

    dwError = CdcGetNewDC(&pszNextDCName, &cdcNextState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (cdcNextState != CDC_DC_STATE_NO_DCS_ALIVE)
    {
        dwError = CdcUpdateAffinitizedDC(
                              pszNextDCName
                              );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwLogError = VmAfdAllocateStringAFromW(
                                        pszNextDCName,
                                        &pszLogNextDCName
                                        );
        if (!dwLogError)
        {
            VmAfdLog(
                VMAFD_DEBUG_ANY,
                "Affinitized to [%s]",
                pszLogNextDCName
                );
        }

        dwError = CdcStateTransition(pStateMachine,cdcNextState);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    iEnd = VmAfdGetTimeInMilliSec();

    dwError = CdcSrvGetDCName(NULL,0, &pCdcInfo);
    BAIL_ON_VMAFD_ERROR(dwError);

    (DWORD)VmAfdAddCDCSuperLogEntry(
                    gVmafdGlobals.pLogger,
                    iStart,
                    iEnd,
                    pCdcInfo,
                    cdcNextState,
                    dwError);

    *pCdcEndingState = cdcNextState;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszNextDCName);
    VMAFD_SAFE_FREE_MEMORY(pszLogNextDCName);

    if (pCdcInfo)
    {
        VmAfdFreeDomainControllerInfoW(pCdcInfo);
    }
    return dwError;

error:

    if (pCdcEndingState)
    {
        *pCdcEndingState = CDC_DC_STATE_UNDEFINED;
    }
    VmAfdLog(
          VMAFD_DEBUG_ERROR,
          "Handling State NoDCsAlive failed with error [%d]",
          dwError
          );

    goto cleanup;
}

static
DWORD
CdcHandleLegacyHAState(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    PCDC_DC_STATE pCdcEndingState
    )
{
    DWORD dwError = 0;
    PWSTR pszNextDCName = NULL;
    CDC_DC_STATE cdcNextState = CDC_DC_STATE_UNDEFINED;
    VMAFD_DOMAIN_STATE DomainState = VMAFD_DOMAIN_STATE_NONE;


    dwError = VmAfSrvGetDomainState(&DomainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (DomainState == VMAFD_DOMAIN_STATE_CLIENT)
    {
        dwError = CdcEnableStateMachine(pStateMachine);
        BAIL_ON_VMAFD_ERROR(dwError);
        cdcNextState = CDC_DC_STATE_NO_DC_LIST;
    }

    *pCdcEndingState = cdcNextState;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszNextDCName);
    return dwError;

error:

    if (pCdcEndingState)
    {
        *pCdcEndingState = CDC_DC_STATE_UNDEFINED;
    }
    VmAfdLog(
          VMAFD_DEBUG_ERROR,
          "Handling LegacyHA State failed with error [%d]",
          dwError
          );

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
#ifndef CDC_DISABLE_OFFSITE
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
#else
        cdcNextState = CDC_DC_STATE_NO_DCS_ALIVE;
        dwError = 0;
#endif
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

    dwError = CdcRegDbSetDCNameHA(pszNewDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDomainName);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
CdcStateMachineThreadSleep(
      PCDC_THREAD_CONTEXT pThrData,
      DWORD dwHeartBeat
      )
{
      DWORD dwError = 0;
      BOOLEAN bRetryWait = FALSE;
      struct timespec ts = {0};

      if (!pThrData)
      {
          dwError = ERROR_INVALID_PARAMETER;
          BAIL_ON_VMAFD_ERROR(dwError);
      }

      ts.tv_sec = time(NULL) + dwHeartBeat;

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

cleanup:

      return dwError;
error:

      goto cleanup;
}

static
PCSTR
CdcStateToString(
    CDC_DC_STATE cdcState
    )
{
    PCSTR pszStateString = NULL;

    switch (cdcState)
    {
        case CDC_DC_STATE_LEGACY:
          pszStateString = "CDC_DC_STATE_LEGACY";
          break;

        case CDC_DC_STATE_NO_DC_LIST:
          pszStateString = "CDC_DC_STATE_NO_DC_LIST";
          break;

        case CDC_DC_STATE_SITE_AFFINITIZED:
          pszStateString = "CDC_DC_STATE_SITE_AFFINITIZED";
          break;

        case CDC_DC_STATE_OFF_SITE:
          pszStateString = "CDC_DC_STATE_OFF_SITE";
          break;

        case CDC_DC_STATE_NO_DCS_ALIVE:
          pszStateString = "CDC_DC_STATE_NO_DCS_ALIVE";
          break;

        default:
          pszStateString = "CDC_DC_STATE_UNDEFINED";
          break;
    }

    return pszStateString;
}


static
DWORD
CdcStateTransition(
    PCDC_STATE_MACHINE_CONTEXT pStateMachine,
    CDC_DC_STATE cdcNextState
    )
{
    DWORD dwError = 0;
    CDC_DC_STATE cdcCurrentState = CDC_DC_STATE_UNDEFINED;
    BOOL bIsLocked = FALSE;

    if (!pStateMachine)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcSrvGetCurrentState(&cdcCurrentState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (CdcIsValidTransition(cdcCurrentState,cdcNextState) &&
        cdcCurrentState != cdcNextState
       )
    {
        VMAFD_LOCK_MUTEX(bIsLocked, &pStateMachine->state_mutex);

        dwError = CdcDbSetHAClientState(cdcNextState);
        BAIL_ON_VMAFD_ERROR(dwError);

        VmAfdLog(
             VMAFD_DEBUG_ANY,
             "CDC State transitioned from %s to %s",
             CdcStateToString(cdcCurrentState),
             CdcStateToString(cdcNextState)
            );

        VMAFD_UNLOCK_MUTEX(bIsLocked, &pStateMachine->state_mutex);
    }
    else if (cdcCurrentState != cdcNextState)
    {
        dwError = ERROR_INVALID_OPERATION;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    VMAFD_UNLOCK_MUTEX(bIsLocked, &pStateMachine->state_mutex);
    return dwError;
error:

    VmAfdLog(
          VMAFD_DEBUG_ERROR,
          "CDC State transition from %s to %s failed with error [%d]",
           CdcStateToString(cdcCurrentState),
           CdcStateToString(cdcNextState),
           dwError
           );

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
        case CDC_DC_STATE_LEGACY:
          bIsValidTransaction = TRUE;
          break;
        case CDC_DC_STATE_NO_DC_LIST:
          if (cdcCurrentState == CDC_DC_STATE_LEGACY)
          {
              bIsValidTransaction = TRUE;
          }
          break;
        case CDC_DC_STATE_SITE_AFFINITIZED:
          if (cdcCurrentState == CDC_DC_STATE_NO_DC_LIST ||
#ifndef CDC_DISABLE_OFFSITE
              cdcCurrentState == CDC_DC_STATE_OFF_SITE ||
#endif
              cdcCurrentState == CDC_DC_STATE_NO_DCS_ALIVE
             )
          {
              bIsValidTransaction = TRUE;
          }
          break;

#ifndef CDC_DISABLE_OFFSITE
        case CDC_DC_STATE_OFF_SITE:
#endif
        case CDC_DC_STATE_NO_DCS_ALIVE:
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

    PWSTR pszSiteNameCurr = NULL;

    dwError = CdcGetClientSiteName(&pszSiteNameCurr);
    if (dwError)
    {
        dwError = 0;
        if (IsNullOrEmptyString(pszSiteName))
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = CdcRegDbSetSiteName(pszSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszSiteNameCurr);
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

/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : cdcservice.c
 *
 * Abstract :
 *
 */

#include "includes.h"

static
DWORD
CdcPurgeCache(
    VOID
    );

DWORD
CdcInitCdcService(
    PCDC_CONTEXT *ppCdcContext
    )
{
    DWORD dwError = 0;
    PCDC_CONTEXT pCdcContext = NULL;
    PCDC_STATE_MACHINE_CONTEXT pStateMachineContext = NULL;
    PCDC_CACHE_UPDATE_CONTEXT pCdcUpdateContext = NULL;

    if (!ppCdcContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcInitStateMachine(&pStateMachineContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcInitCdcCacheUpdate(&pCdcUpdateContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateMemory(
                          sizeof(CDC_CONTEXT),
                          (PVOID*)&pCdcContext
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    pCdcContext->pCdcStateMachineContext = pStateMachineContext;
    pCdcContext->pCdcCacheUpdateContext = pCdcUpdateContext;

    pStateMachineContext = NULL;
    pCdcUpdateContext = NULL;

    pCdcContext->context_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

    *ppCdcContext = pCdcContext;

cleanup:

    if (pStateMachineContext)
    {
        CdcShutdownStateMachine(pStateMachineContext);
    }
    if (pCdcUpdateContext)
    {
        CdcShutdownCdcCacheUpdate(pCdcUpdateContext);
    }
    return dwError;
error:

    if (ppCdcContext)
    {
        *ppCdcContext = NULL;
    }
    if (pCdcContext)
    {
        CdcShutdownCdcService(pCdcContext);
    }
    goto cleanup;
}

VOID
CdcShutdownCdcService(
    PCDC_CONTEXT pCdcContext
    )
{
    if (pCdcContext)
    {
        if (pCdcContext->pCdcStateMachineContext)
        {
            CdcShutdownStateMachine(pCdcContext->pCdcStateMachineContext);
        }
        if (pCdcContext->pCdcCacheUpdateContext)
        {
            CdcShutdownCdcCacheUpdate(pCdcContext->pCdcCacheUpdateContext);
        }
        pthread_mutex_destroy(&pCdcContext->context_mutex);
    }

    VMAFD_SAFE_FREE_MEMORY(pCdcContext);
}

DWORD
CdcSrvInitDefaultHAMode(
    PCDC_CONTEXT pCdcContext
    )
{
    DWORD dwError = 0;
    CDC_DB_ENUM_HA_MODE cdcHAMode = CDC_DB_ENUM_HA_MODE_UNDEFINED;

    if (!pCdcContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcRegDbGetHAMode(&cdcHAMode);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (cdcHAMode == CDC_DB_ENUM_HA_MODE_DEFAULT)
    {
        (DWORD)CdcWakeupCdcCacheUpdate(
                                pCdcContext->pCdcCacheUpdateContext,
                                TRUE,
                                FALSE
                                );
        (DWORD)CdcWakeupStateMachine(
                                pCdcContext->pCdcStateMachineContext,
                                FALSE
                                );
    }

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
CdcSrvShutdownDefaultHAMode(
    PCDC_CONTEXT pCdcContext
    )
{
    DWORD dwError = 0;

    if (!pCdcContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcDisableStateMachine(pCdcContext->pCdcStateMachineContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcPurgeCache();
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}


DWORD
CdcSrvEnableDefaultHA(
    PCDC_CONTEXT pCdcContext
    )
{
    DWORD dwError = 0;
    CDC_DB_ENUM_HA_MODE cdcHAMode = CDC_DB_ENUM_HA_MODE_UNDEFINED;
    BOOL bIsLocked  = FALSE;

    if (!pCdcContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcRegDbGetHAMode(&cdcHAMode);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (cdcHAMode != CDC_DB_ENUM_HA_MODE_DEFAULT)
    {
        VMAFD_LOCK_MUTEX(bIsLocked, &pCdcContext->context_mutex);

        dwError = CdcRegDbSetHAMode(CDC_DB_ENUM_HA_MODE_DEFAULT);
        BAIL_ON_VMAFD_ERROR(dwError);

        VMAFD_UNLOCK_MUTEX(bIsLocked, &pCdcContext->context_mutex);
    }

    dwError = CdcSrvInitDefaultHAMode(pCdcContext);
    BAIL_ON_VMAFD_ERROR(dwError);


cleanup:

    if (pCdcContext)
    {
        VMAFD_UNLOCK_MUTEX(bIsLocked, &pCdcContext->context_mutex);
    }
    return dwError;

error:
    goto cleanup;
}

DWORD
CdcSrvEnableLegacyModeHA(
    PCDC_CONTEXT pCdcContext
    )
{
    DWORD dwError = 0;
    BOOL  bIsLocked = FALSE;

    if (!pCdcContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        VmAfdLog(
              VMAFD_DEBUG_ERROR,
              "Invalid state machine context received"
              );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    VMAFD_LOCK_MUTEX(bIsLocked, &pCdcContext->context_mutex);

    dwError = CdcRegDbSetHAMode(CDC_DB_ENUM_HA_MODE_LEGACY);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcSrvShutdownDefaultHAMode(pCdcContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    VMAFD_UNLOCK_MUTEX(bIsLocked, &pCdcContext->context_mutex);

cleanup:

    if (pCdcContext)
    {
        VMAFD_UNLOCK_MUTEX(bIsLocked,&pCdcContext->context_mutex);
    }
    return dwError;

error:
    goto cleanup;
}

DWORD
CdcSrvForceRefreshCache(
    PCDC_CONTEXT pCdcContext
    )
{
    DWORD dwError = 0;
    CDC_DB_ENUM_HA_MODE cdcHAMode = CDC_DB_ENUM_HA_MODE_UNDEFINED;

    if (!pCdcContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        VmAfdLog(
             VMAFD_DEBUG_ERROR,
             "Invalid CDC context received"
             );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcRegDbGetHAMode(&cdcHAMode);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (cdcHAMode == CDC_DB_ENUM_HA_MODE_DEFAULT)
    {
        (DWORD)CdcWakeupCdcCacheUpdate(
                                pCdcContext->pCdcCacheUpdateContext,
                                TRUE,
                                TRUE
                                );
        (DWORD)CdcWakeupStateMachine(
                                pCdcContext->pCdcStateMachineContext,
                                TRUE
                                );
    }

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
CdcSrvGetDCName(
    PCWSTR pszDomainName,
    DWORD  dwFlags,
    PCDC_DC_INFO_W *ppAffinitizedDC
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W pAffinitizedDC = NULL;
    PCDC_DC_INFO_W pAffinitizedDCToFree = NULL;
    DWORD dwAffinitizedSince = 0;
    CDC_DB_ENUM_HA_MODE cdcHAMode = CDC_DB_ENUM_HA_MODE_UNDEFINED;
    PWSTR pszDomainToUse = NULL;
    UINT64 iStart = 0;
    UINT64 iEnd = 0;
    BOOL bForceRefresh = FALSE;

    if (!ppAffinitizedDC)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    iStart = VmAfdGetTimeInMilliSec();

    bForceRefresh = dwFlags & CDC_FORCE_REFRESH;

    if (bForceRefresh)
    {
        if (!gVmafdGlobals.pCdcContext)
        {
            dwError = ERROR_INVALID_STATE;
        }
        else
        {
           dwError = CdcSrvForceRefreshCache(gVmafdGlobals.pCdcContext);
        }
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

    dwError = CdcRegDbGetHAMode(&cdcHAMode);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcDbGetAffinitizedDC(
                          pszDomainToUse,
                          &dwAffinitizedSince,
                          &pAffinitizedDC
                          );
    if (cdcHAMode == CDC_DB_ENUM_HA_MODE_LEGACY || dwError)
    {
        pAffinitizedDCToFree = pAffinitizedDC;
        dwError = VmAfdAllocateMemory(
                                sizeof(CDC_DC_INFO_W),
                                (PVOID *)&pAffinitizedDC
                                );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfSrvGetDCName(&pAffinitizedDC->pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdTrimFQDNTrailingDot(pAffinitizedDC->pszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfSrvGetDomainName(&pAffinitizedDC->pszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = CdcRegDbGetSiteName(&pAffinitizedDC->pszDcSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    iEnd = VmAfdGetTimeInMilliSec();
    *ppAffinitizedDC = pAffinitizedDC;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDomainToUse);
    if (pAffinitizedDCToFree)
    {
        VmAfdFreeDomainControllerInfoW(pAffinitizedDCToFree);
    }
    return dwError;
error:

    (DWORD)VmAfdAddDCSuperLogEntry(
                            gVmafdGlobals.pLogger,
                            iStart,
                            iEnd,
                            pAffinitizedDC,
                            dwError);

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
    CDC_DC_STATE cdcState = CDC_DC_STATE_UNDEFINED;

    if (!pCdcState)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcDbGetHAClientState(&dwCdcState);
    if (dwError == ERROR_OBJECT_NOT_FOUND)
    {
        cdcState = CDC_DC_STATE_LEGACY;
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

            case CDC_DC_STATE_LEGACY:
              cdcState = CDC_DC_STATE_LEGACY;
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
    UINT64 iStart = 0;
    UINT64 iEnd = 0;

    if (!ppwszDCName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError)
    }

    iStart = VmAfdGetTimeInMilliSec();
    dwError = CdcSrvGetDCName(NULL, 0, &pAffinitizedDC);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringW(
                              pAffinitizedDC->pszDCName,
                              &pwszDCName
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    iEnd = VmAfdGetTimeInMilliSec();

    *ppwszDCName = pwszDCName;

cleanup:

    if (pAffinitizedDC)
    {
        VmAfdFreeDomainControllerInfoW(pAffinitizedDC);
    }
    return dwError;
error:

    (DWORD)VmAfdAddDCSuperLogEntry(
                            gVmafdGlobals.pLogger,
                            iStart,
                            iEnd,
                            pAffinitizedDC,
                            dwError);

    if (ppwszDCName)
    {
        *ppwszDCName = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    goto cleanup;
}

DWORD
CdcSrvEnumDCEntries(
    PWSTR **pppszEntryNames,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    DWORD dwCount = 0;
    DWORD dwIndex = 0;
    PWSTR* ppszEntryNames = NULL;
    PCDC_DB_ENTRY_W pCdcEntries = NULL;
    UINT64 iStart = 0;
    UINT64 iEnd = 0;

    if (!pppszEntryNames || !pdwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    iStart = VmAfdGetTimeInMilliSec();

    dwError = CdcDbEnumDCEntries(&pCdcEntries, &dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwCount)
    {
        dwError = VmAfdAllocateMemory(
                            sizeof(PWSTR)*dwCount,
                            (PVOID)&ppszEntryNames
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        for (; dwIndex < dwCount; ++dwIndex)
        {
            dwError = VmAfdAllocateStringW(
                                  pCdcEntries[dwIndex].pszDCName,
                                  &ppszEntryNames[dwIndex]
                                  );
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    iEnd = VmAfdGetTimeInMilliSec();

    *pppszEntryNames = ppszEntryNames;
    *pdwCount = dwCount;

cleanup:

    if (pCdcEntries)
    {
        VmAfdFreeCdcDbEntriesW(pCdcEntries, dwCount);
    }
    return dwError;

error:

    (DWORD)VmAfdAddDBSuperLogEntry(
                               gVmafdGlobals.pLogger,
                               iStart,
                               iEnd,
                               pCdcEntries,
                               dwError);

    if (pppszEntryNames)
    {
        *pppszEntryNames = NULL;
    }
    if (ppszEntryNames)
    {
        VmAfdFreeStringArrayW(ppszEntryNames, dwCount);
    }
    goto cleanup;
}

DWORD
CdcSrvGetDCStatusInfo(
    PWSTR pwszDCName,
    PWSTR pwszDomainName,
    PCDC_DC_STATUS_INFO_W *ppCdcStatusInfo,
    PVMAFD_HB_STATUS_W    *ppHeartbeatStatus
    )
{
    DWORD dwError = 0;
    PWSTR pwszDomainToUse = NULL;
    PCDC_DC_STATUS_INFO_W pCdcStatusInfo = NULL;
    PVMAFD_HB_STATUS_W    pHeartbeatStatus = NULL;

    if (IsNullOrEmptyString(pwszDCName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!ppCdcStatusInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    
    if (IsNullOrEmptyString(pwszDomainName))
    {
        dwError = VmAfSrvGetDomainName(&pwszDomainToUse);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdAllocateStringW(pwszDomainName, &pwszDomainToUse);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcDbGetDCInfo(
                            pwszDCName,
                            pwszDomainToUse,
                            &pCdcStatusInfo
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (ppHeartbeatStatus)
    {

        dwError = CdcDbGetHeartbeatStatus(
                                    pwszDCName,
                                    pwszDomainToUse,
                                    &pHeartbeatStatus
                                    );
        BAIL_ON_VMAFD_ERROR(dwError);
        
        pHeartbeatStatus->bIsAlive = pCdcStatusInfo->bIsAlive;
        *ppHeartbeatStatus = pHeartbeatStatus;
    }

    *ppCdcStatusInfo = pCdcStatusInfo;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDomainToUse);
    return dwError;
error:

    if (ppCdcStatusInfo)
    {
        *ppCdcStatusInfo = NULL;
    }
    if (ppHeartbeatStatus)
    {
        *ppHeartbeatStatus = NULL;
    }
    if (pCdcStatusInfo)
    {
        VmAfdFreeCdcStatusInfoW(pCdcStatusInfo);
    }
    if (pHeartbeatStatus)
    {
        VmAfdFreeHbStatusW(pHeartbeatStatus);
    }
    goto cleanup;
}

DWORD
CdcSrvWakeupStateMachine(
    PCDC_CONTEXT pCdcContext
    )
{
    DWORD dwError = 0;

    if (!pCdcContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcWakeupStateMachine(
                    pCdcContext->pCdcStateMachineContext,
                    FALSE
                    );
    if (dwError)
    {
        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "Failed to wake up state machine: [%d]",
            dwError);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
CdcRpcAllocateDCStatusInfo(
           PCDC_DC_STATUS_INFO_W pDCStatusInfo,
           PCDC_DC_STATUS_INFO_W *ppRpcDCStatusInfo
           )
{
    DWORD dwError = 0;
    PCDC_DC_STATUS_INFO_W pRpcDCStatusInfo = NULL;

    if (!pDCStatusInfo || !ppRpcDCStatusInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pDCStatusInfo)
    {
        dwError = VmAfdRpcServerAllocateMemory(
                                      sizeof(CDC_DC_STATUS_INFO_W),
                                      (PVOID)&pRpcDCStatusInfo
                                      );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdRpcServerAllocateStringW(
                                      pDCStatusInfo->pwszSiteName,
                                      &pRpcDCStatusInfo->pwszSiteName
                                      );
        BAIL_ON_VMAFD_ERROR(dwError);

        pRpcDCStatusInfo->dwLastPing = pDCStatusInfo->dwLastPing;
        pRpcDCStatusInfo->dwLastResponseTime = pDCStatusInfo->dwLastResponseTime;
        pRpcDCStatusInfo->dwLastError = pDCStatusInfo->dwLastError;
        pRpcDCStatusInfo->bIsAlive = pDCStatusInfo->bIsAlive;
    }

    *ppRpcDCStatusInfo = pRpcDCStatusInfo;

cleanup:

    return dwError;
error:

    if (ppRpcDCStatusInfo)
    {
        *ppRpcDCStatusInfo = NULL;
    }
    if (pRpcDCStatusInfo)
    {
        CdcRpcFreeDCStatuInfo(pRpcDCStatusInfo);
    }
    goto cleanup;
}

VOID
CdcRpcFreeDCStatuInfo(
    PCDC_DC_STATUS_INFO_W  pRpcDCStatusInfo
    )
{
    if (pRpcDCStatusInfo)
    {
       if (pRpcDCStatusInfo->pwszSiteName)
       {
          VmAfdRpcServerFreeMemory(pRpcDCStatusInfo->pwszSiteName);
       }
       VmAfdRpcServerFreeMemory(pRpcDCStatusInfo);
    }
}

static
DWORD
CdcPurgeCache(
    VOID
    )
{
    DWORD dwError = 0;
    PCDC_DB_ENTRY_W pEntries = NULL;
    DWORD dwDCCount = 0;
    DWORD dwIndex = 0;
    UINT64 iStart = 0;
    UINT64 iEnd = 0;
    iStart = VmAfdGetTimeInMilliSec();

    dwError = CdcDbEnumDCEntries(
                        &pEntries,
                        &dwDCCount
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwDCCount)
    {
        for (; dwIndex<dwDCCount; ++dwIndex)
        {
            dwError = CdcDbDeleteDCEntry(pEntries[dwIndex].pszDCName);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    iEnd = VmAfdGetTimeInMilliSec();

cleanup:

    if (pEntries)
    {
        VmAfdFreeCdcDbEntriesW(pEntries, dwDCCount);
    }
    return dwError;
error:

    (DWORD)VmAfdAddDBSuperLogEntry(
                                gVmafdGlobals.pLogger,
                                iStart,
                                iEnd,
                                pEntries,
                                dwError);

    goto cleanup;
}


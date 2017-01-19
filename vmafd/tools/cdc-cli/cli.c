/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : cli.c
 *
 * Abstract :
 *
 */
#include "includes.h"

static
PCSTR
CdcCliMapState(
    CDC_DC_STATE cdcState
    );

DWORD
CdcCliEnableDefaultHA(
    PVMAFD_SERVER pServer
    )
{
    DWORD dwError = 0;

    dwError = CdcEnableClientAffinity(
                    pServer
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf(stdout, "Successfully set to Default client affinity HA mode. \n");

cleanup:

    return dwError;

error:
    if (dwError == ERROR_NOT_SUPPORTED)
    {
        fprintf(
            stdout,
            "Domain function level is not ready for PSC HA. "
            "PSC HA will be available when DFL is ready.");
    }
    goto cleanup;
}

DWORD
CdcCliEnableLegacyHA(
    PVMAFD_SERVER pServer
    )
{
    DWORD dwError = 0;

    dwError = CdcDisableClientAffinity(
                    pServer
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf(stdout, "Successfully set to legacy mode HA. \n");

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
CdcCliGetDCName(
    PVMAFD_SERVER pServer
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_A pDomainControllerInfoA = NULL;

    dwError = CdcGetDCNameA(
                    pServer,
                    NULL,
                    NULL,
                    NULL,
                    0,
                    &pDomainControllerInfoA
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pDomainControllerInfoA->pszDCName)
    {
        fprintf(stdout, "%s\n", pDomainControllerInfoA->pszDCName);
    }
    else
    {
        fprintf(stdout, "No DC found\n");
    }

cleanup:

    if (pDomainControllerInfoA)
    {
        CdcFreeDomainControllerInfoA(pDomainControllerInfoA);
    }

    return dwError;
error:

    goto cleanup;
}

DWORD
CdcCliDcCacheList(
    PVMAFD_SERVER pServer
    )
{
    DWORD dwError = 0;
    PSTR* ppszDCNameArray = NULL;
    DWORD dwCount = 0;

    dwError = CdcEnumDCEntriesA(
                    pServer,
                    &ppszDCNameArray,
                    &dwCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!dwCount)
    {
        fprintf(stderr, "No DCs found\n");
    }
    else
    {
        DWORD idx = 0;

        for (; idx < dwCount; idx++)
        {
            fprintf(stdout, "%s\n", ppszDCNameArray[idx]);
        }
    }

cleanup:

    if (ppszDCNameArray)
    {
        VmAfdFreeStringArrayCountA(ppszDCNameArray, dwCount);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
CdcCliGetStateofClientAffinity(
    PVMAFD_SERVER pServer
    )
{
    DWORD dwError = 0;
    CDC_DC_STATE cdcState = CDC_DC_STATE_UNDEFINED;

    dwError = CdcGetCurrentState(
                            pServer,
                            &cdcState
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf(
          stdout,
          "Client Domain Controller's cache's current state:\t %s \n",
           CdcCliMapState(cdcState)
           );

cleanup:

    return dwError;

error:

    fprintf (stderr,"The call failed with error:%d\n", dwError);
    goto cleanup;
}

DWORD
CdcCliDcCacheRefresh(
    PVMAFD_SERVER pServer
    )
{
    DWORD dwError = 0;

/*
 *    dwError = CdcForceRefreshCacheA(
 *                    NULL);
 *
 */
    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf(stdout, "Successfully refreshed cache \n");

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
CdcCliGetDCStatus(
    PVMAFD_SERVER pServer,
    PCSTR pszPSC
    )
{
    DWORD dwError = 0;
    PCDC_DC_STATUS_INFO_A pCdcStatusInfo = NULL;
    PVMAFD_HB_STATUS_A pHeartbeatStatus = NULL;
    PSTR pszErrorMsg = NULL;

    dwError = CdcGetDCStatusInfoA(
                            pServer,
                            pszPSC,
                            NULL,
                            &pCdcStatusInfo,
                            &pHeartbeatStatus
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf (stdout, "The state of PSC [%s] is:\n", pszPSC);

    fprintf (
            stdout,
            "Last Ping     : %d\n"
            "Response Time : %d\n"
            "Status        : %s\n"
            "Error(if any) : %d",
            pCdcStatusInfo->dwLastPing,
            pCdcStatusInfo->dwLastResponseTime,
            pCdcStatusInfo->bIsAlive? "ALIVE" : "DOWN",
            pCdcStatusInfo->dwLastError
            );

    if (pCdcStatusInfo->dwLastError)
    {
        fprintf (
              stdout,
              " [%s] \n",
              VmAfdGetErrorString(pCdcStatusInfo->dwLastError, &pszErrorMsg)?
              "Unknown Error":
              pszErrorMsg
              );
    }
    else
    {
        fprintf(stdout,"\n");
    }

cleanup:

    if (pCdcStatusInfo)
    {
        CdcFreeDCStatusInfoA(pCdcStatusInfo);
    }
    if (pHeartbeatStatus)
    {
        VmAfdFreeHeartbeatStatusA(pHeartbeatStatus);
    }
    VMAFD_SAFE_FREE_STRINGA(pszErrorMsg);

    return dwError;

error:

    goto cleanup;
}

static
PCSTR
CdcCliMapState(
    CDC_DC_STATE cdcState
    )
{
    PCSTR pszState = NULL;

    switch(cdcState)
    {
      case CDC_DC_STATE_LEGACY:

        pszState = "Legacy Mode";
        break;

      case CDC_DC_STATE_NO_DC_LIST:

        pszState = "No knowledge of any of any DCs";
        break;

      case CDC_DC_STATE_SITE_AFFINITIZED:

        pszState = "Site affinitized";
        break;

      case CDC_DC_STATE_OFF_SITE:

        pszState = "Affinitized to an offsite DC";
        break;

      case CDC_DC_STATE_NO_DCS_ALIVE:

        pszState = "All known DCs are down";
        break;

      default:

        pszState = "Invalid State";
        break;
    }

    return pszState;
}

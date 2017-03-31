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
DWORD
VmAfdCliGetDomainState(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetStatus(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliSetDomainName(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetDomainName(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliSetLDU(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetLDU(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetRHTTPProxyPort(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliSetRHTTPProxyPort(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliSetDCPort(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetCMLocation(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetLSLocation(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliSetPNID(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetPNIDUrl(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetPNID(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliSetDCName(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetDCName(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliPromoteVmDir(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliDemoteVmDir(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliJoinVmDir(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliLeaveVmDir(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliJoinAD(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliLeaveAD(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliQueryAD(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetSiteGUID(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetSiteName(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetMachineID(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliSetMachineID(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetHeartbeatStatus(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliGetDCList(
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
VmAfdCliRefreshSiteName(
    VOID
    );

static
DWORD
VmAfdCliChangePNID(
    PVM_AFD_CLI_CONTEXT pContext
    );

DWORD
VmAfdCliExecute(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    switch (pContext->action)
    {
        case VM_AFD_ACTION_GET_DOMAIN_NAME:

            dwError = VmAfdCliGetDomainName(pContext);

            break;

        case VM_AFD_ACTION_SET_DOMAIN_NAME:

            dwError = VmAfdCliSetDomainName(pContext);

            break;

        case VM_AFD_ACTION_GET_DOMAIN_STATE:

            dwError = VmAfdCliGetDomainState(pContext);

            break;

        case VM_AFD_ACTION_GET_LDU:

            dwError = VmAfdCliGetLDU(pContext);

            break;

        case VM_AFD_ACTION_SET_LDU:

            dwError = VmAfdCliSetLDU(pContext);

            break;

        case VM_AFD_ACTION_GET_RHTTPPROXY_PORT:

            dwError = VmAfdCliGetRHTTPProxyPort(pContext);

            break;

        case VM_AFD_ACTION_SET_RHTTPPROXY_PORT:

            dwError = VmAfdCliSetRHTTPProxyPort(pContext);

            break;

        case VM_AFD_ACTION_SET_DC_PORT:

            dwError = VmAfdCliSetDCPort(pContext);

            break;

        case VM_AFD_ACTION_GET_CM_LOCATION:

            dwError = VmAfdCliGetCMLocation(pContext);

            break;

        case VM_AFD_ACTION_GET_LS_LOCATION:

            dwError = VmAfdCliGetLSLocation(pContext);

            break;

        case VM_AFD_ACTION_GET_PNID:

            dwError = VmAfdCliGetPNID(pContext);

            break;

        case VM_AFD_ACTION_GET_PNID_URL:

            dwError = VmAfdCliGetPNIDUrl(pContext);

            break;

        case VM_AFD_ACTION_SET_PNID:

            dwError = VmAfdCliSetPNID(pContext);

            break;

        case VM_AFD_ACTION_GET_DC_NAME:

            dwError = VmAfdCliGetDCName(pContext);

            break;

        case VM_AFD_ACTION_SET_DC_NAME:

            dwError = VmAfdCliSetDCName(pContext);

            break;

        case VM_AFD_ACTION_PROMOTE_VM_DIR:

            dwError = VmAfdCliPromoteVmDir(pContext);

            break;

        case VM_AFD_ACTION_DEMOTE_VM_DIR:

            dwError = VmAfdCliDemoteVmDir(pContext);

            break;

        case VM_AFD_ACTION_JOIN_VM_DIR:

            dwError = VmAfdCliJoinVmDir(pContext);

            break;

        case VM_AFD_ACTION_LEAVE_VM_DIR:

            dwError = VmAfdCliLeaveVmDir(pContext);

            break;

        case VM_AFD_ACTION_JOIN_AD:

            dwError = VmAfdCliJoinAD(pContext);

            break;

        case VM_AFD_ACTION_LEAVE_AD:

            dwError = VmAfdCliLeaveAD(pContext);

            break;

        case VM_AFD_ACTION_QUERY_AD:

            dwError = VmAfdCliQueryAD(pContext);

            break;

        case VM_AFD_ACTION_GET_STATUS:

            dwError = VmAfdCliGetStatus(pContext);

            break;

        case VM_AFD_ACTION_GET_SITE_GUID:

            dwError = VmAfdCliGetSiteGUID(pContext);

            break;

        case VM_AFD_ACTION_GET_SITE_NAME:

            dwError = VmAfdCliGetSiteName(pContext);

            break;

        case VM_AFD_ACTION_GET_MACHINE_ID:

            dwError = VmAfdCliGetMachineID(pContext);

            break;

        case VM_AFD_ACTION_SET_MACHINE_ID:

            dwError = VmAfdCliSetMachineID(pContext);

            break;

        case VM_AFD_ACTION_GET_HEARTBEAT_STATUS:

            dwError = VmAfdCliGetHeartbeatStatus(pContext);

            break;

        case VM_AFD_ACTION_REFRESH_SITE_NAME:

            dwError = VmAfdCliRefreshSiteName();
            break;

        case VM_AFD_ACTION_CHANGE_PNID:

            dwError = VmAfdCliChangePNID(pContext);
            break;

        case VM_AFD_ACTION_GET_DC_LIST:

            dwError = VmAfdCliGetDCList(pContext);
            break;

        case VM_AFD_ACTION_ADD_PASSWORD_ENTRY:
        case VM_AFD_ACTION_GET_MACHINE_ACCOUNT_INFO:
        case VM_AFD_ACTION_SET_MACHINE_ACCOUNT_INFO:
        case VM_AFD_ACTION_GET_MACHINE_SSL_CERTIFICATES:
        case VM_AFD_ACTION_SET_MACHINE_SSL_CERTIFICATES:

        default:

            dwError = ERROR_INVALID_PARAMETER;

            break;
    }

    return dwError;
}

static
DWORD
VmAfdCliGetDomainName(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetDomainNameA(
                    pContext->pszServerName,
                    &pContext->pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("%s\n", pContext->pszDomainName);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliSetDomainName(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdSetDomainNameA(
                    pContext->pszServerName,
                    pContext->pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliGetDomainState(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PCSTR pszDomainState = NULL;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetDomainStateA(
                    pContext->pszServerName,
                    &pContext->domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    switch (pContext->domainState)
    {
      case VMAFD_DOMAIN_STATE_NONE:
        pszDomainState = "None";
        break;
      case VMAFD_DOMAIN_STATE_CONTROLLER:
        pszDomainState = "Controller";
        break;
      case VMAFD_DOMAIN_STATE_CLIENT:
        pszDomainState = "Client";
        break;
      default:
        pszDomainState = "Unknown";
        break;
    }
    printf("%s\n", pszDomainState);

error:

    return dwError;
}

static
DWORD
VmAfdCliGetStatus(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PCSTR pszStatus = NULL;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetStatusA(
                    pContext->pszServerName,
                    &pContext->status);
    BAIL_ON_VMAFD_ERROR(dwError);

    switch (pContext->status)
    {
      case VMAFD_STATUS_UNKNOWN:
        pszStatus = "Unknown";
        break;
      case VMAFD_STATUS_INITIALIZING:
        pszStatus = "Initializing";
        break;
      case VMAFD_STATUS_PAUSED:
        pszStatus = "Paused";
        break;
      case VMAFD_STATUS_RUNNING:
        pszStatus = "Running";
        break;
      case VMAFD_STATUS_STOPPING:
        pszStatus = "Stopping";
        break;
      case VMAFD_STATUS_STOPPED:
        pszStatus = "Stopped";
        break;
      default:
        pszStatus = "Unknown";
        break;
    }
    printf("%s\n", pszStatus);

error:

    return dwError;
}

static
DWORD
VmAfdCliGetLDU(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetLDUA(
                    pContext->pszServerName,
                    &pContext->pszLDU);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("%s\n", pContext->pszLDU);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliSetLDU(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdSetLDUA(
                    pContext->pszServerName,
                    pContext->pszLDU);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliGetRHTTPProxyPort(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetRHTTPProxyPortA(
                    pContext->pszServerName,
                    &pContext->dwPort);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("%u\n", pContext->dwPort);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliSetRHTTPProxyPort(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdSetRHTTPProxyPortA(
                    pContext->pszServerName,
                    pContext->dwPort);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliSetDCPort(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdSetDCPortA(
                    pContext->pszServerName,
                    pContext->dwPort);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliGetCMLocation(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetCMLocationA(
                    pContext->pszServerName,
                    &pContext->pszCMLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("%s\n", pContext->pszCMLocation);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliGetLSLocation(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PSTR pszLSLocation = NULL;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetLSLocationA(
                    pContext->pszServerName,
                    &pszLSLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("%s\n", pszLSLocation);

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszLSLocation);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliGetPNIDUrl(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetPNIDForUrlA(
                    pContext->pszServerName,
                    &pContext->pszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("%s\n", pContext->pszPNID);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliGetPNID(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetPNIDA(
                    pContext->pszServerName,
                    &pContext->pszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("%s\n", pContext->pszPNID);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliSetPNID(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdSetPNIDA(
                    pContext->pszServerName,
                    pContext->pszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliGetSiteGUID(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetSiteGUIDA(
                    pContext->pszServerName,
                    &pContext->pszSiteGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("%s\n", pContext->pszSiteGUID);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliGetSiteName(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetSiteNameA(
                    pContext->pszServerName,
                    &pContext->pszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("%s\n", pContext->pszSiteName);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliGetMachineID(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PSTR pszMachineID = NULL;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetMachineIDA(
                    pContext->pszServerName,
                    &pszMachineID);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("%s\n", pszMachineID);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszMachineID);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliSetMachineID(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdSetMachineIDA(
                    pContext->pszServerName,
                    pContext->pszMachineId);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("Machine Id [%s] was updated successfully\n", pContext->pszMachineId);

cleanup:

    return dwError;

error:

    printf("Failed to set Machine Id [%s]. Error [%d]\n",
            pContext->pszMachineId,
            dwError);

    goto cleanup;
}

static
DWORD
VmAfdCliGetHeartbeatStatus(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PSTR pszAccount = NULL;
    PSTR pszPassword = NULL;
    PSTR pszDomainName = NULL;
    BOOL bUsingIPC = FALSE;
    PVMAFD_HB_STATUS_A pHeartbeatStatus = NULL;
    PVMAFD_SERVER pServer = NULL;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pContext->pszServerName))
    {
        bUsingIPC = TRUE;
    }

    if (bUsingIPC && !IsNullOrEmptyString(pContext->pszUserName))
    {
        printf ("Getting heartbeat status of local system\n"
                "lotus user-name will not be used\n"
               );
    }

    if (!bUsingIPC)
    {
      //Factor domain name as well into this equation
        if(!pContext->pszUserName)
        {
            dwError = VmAfdGetMachineAccountInfoA(NULL, &pszAccount, &pszPassword);
        }
        else if (pContext->pszUserName && pContext->pszPassword)
        {
            dwError = VmAfdAllocateStringA(pContext->pszUserName, &pszAccount);
            BAIL_ON_VMAFD_ERROR(dwError);
            dwError = VmAfdAllocateStringA(pContext->pszPassword, &pszPassword);
        }
        else if (pContext->pszUserName && !pContext->pszPassword)
        {
            //getPassword
        }
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdOpenServerA(
                          pContext->pszServerName,
                          pszAccount,
                          pszPassword,
                          &pServer
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetHeartbeatStatusA(
                                  pServer,
                                  &pHeartbeatStatus
                                  );
    BAIL_ON_VMAFD_ERROR(dwError);

    printf ("Current Status of the machine is :\t%d\n",pHeartbeatStatus->bIsAlive);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszAccount);
    VMAFD_SAFE_FREE_MEMORY(pszPassword);
    VMAFD_SAFE_FREE_MEMORY(pszDomainName);
    return dwError;

error:

    goto cleanup;

}

static
DWORD
VmAfdCliGetDCName(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetDCNameA(
                    pContext->pszServerName,
                    &pContext->pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("%s\n", pContext->pszDCName);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliSetDCName(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdSetDCNameA(
                    pContext->pszServerName,
                    pContext->pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliPromoteVmDir(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD   dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdPromoteVmDirA(
                    pContext->pszServerName,
                    pContext->pszDomainName,
                    pContext->pszUserName,
                    pContext->pszPassword,
                    pContext->pszSiteName,
                    pContext->pszPartnerName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliDemoteVmDir(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD   dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdDemoteVmDirA(
                    pContext->pszServerName,
                    pContext->pszUserName,
                    pContext->pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliJoinVmDir(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdJoinVmDirA(
                    pContext->pszServerName,
                    pContext->pszUserName,
                    pContext->pszPassword,
                    pContext->pszMachineName,
                    pContext->pszDomainName,
                    pContext->pszOrgUnit);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliLeaveVmDir(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PSTR pszAccount = NULL;
    PSTR pszPassword = NULL;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pContext->pszUserName || !pContext->pszPassword)
    {
        dwError = VmAfdGetMachineAccountInfoA(NULL, &pszAccount, &pszPassword);
    } else
    {
        dwError = VmAfdAllocateStringA(pContext->pszUserName, &pszAccount);
        BAIL_ON_VMAFD_ERROR(dwError);
        dwError = VmAfdAllocateStringA(pContext->pszPassword, &pszPassword);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdLeaveVmDirA(pContext->pszServerName, pszAccount, pszPassword, 0);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszAccount);
    VMAFD_SAFE_FREE_MEMORY(pszPassword);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliJoinAD(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdJoinADA(
                    pContext->pszServerName,
                    pContext->pszUserName,
                    pContext->pszPassword,
                    pContext->pszDomainName,
                    pContext->pszOrgUnit);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliLeaveAD(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdLeaveADA(
                    pContext->pszServerName,
                    pContext->pszUserName,
                    pContext->pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmAfdCliQueryAD(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PSTR pszDistinguishedName = NULL;
    PSTR pszNetbiosName = NULL;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdQueryADA(
                    pContext->pszServerName,
                    &pContext->pszMachineName,
                    &pContext->pszDomainName,
                    &pszDistinguishedName,
                    &pszNetbiosName);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("Name = %s\n", pContext->pszMachineName);
    printf("Domain = %s\n", pContext->pszDomainName);
    printf("DistinguishedName = %s\n", pszDistinguishedName);
    printf("NetbiosName = %s\n", pszNetbiosName);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszDistinguishedName);
    VMAFD_SAFE_FREE_MEMORY(pszNetbiosName);
    return dwError;

error:

    goto cleanup;
}


static
DWORD
VmAfdCliGetDCList(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    DWORD dwServerCount = 0;
    DWORD dwIndex = 0;
    PSTR  pszHostname = NULL;
    PSTR  pszAddress = NULL;
    PVMAFD_DC_INFO_W pVmAfdDCInfoList = NULL;
    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetDCList(
                 pContext->pszServerName,
                 pContext->pszDomainName,
                 &dwServerCount,
                 &pVmAfdDCInfoList
              );
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("Number of Server in the Domain = %d\n",dwServerCount);
    for (; dwIndex < dwServerCount ; dwIndex++)
    {
        PVMAFD_DC_INFO_W pDCInfo = &pVmAfdDCInfoList[dwIndex];

        dwError = VmAfdAllocateStringAFromW(pDCInfo->pwszHostName,&pszHostname);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringAFromW(pDCInfo->pwszAddress,&pszAddress);
        BAIL_ON_VMAFD_ERROR(dwError);

        printf("Hostname = %s, Address = %s\n",pszHostname,pszAddress);

	VMAFD_SAFE_FREE_MEMORY(pszHostname);
        VMAFD_SAFE_FREE_MEMORY(pszAddress);
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pVmAfdDCInfoList);
    return dwError;

error:
     goto cleanup;

}



static
DWORD
VmAfdCliRefreshSiteName(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmAfdRefreshSiteName();
    BAIL_ON_VMAFD_ERROR(dwError);


    printf("Successfully refreshed site name.\n");
cleanup:

    return dwError;

error:

    printf ("Refreshing Site name failed with error: %d\n", dwError);

    goto cleanup;
}

static
DWORD
VmAfdCliChangePNID(
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext ||
        IsNullOrEmptyString(pContext->pszUserName) ||
        IsNullOrEmptyString(pContext->pszPassword) ||
        IsNullOrEmptyString(pContext->pszPNID))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdChangePNIDA(
            pContext->pszUserName,
            pContext->pszPassword,
            pContext->pszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

error:
    return dwError;
}


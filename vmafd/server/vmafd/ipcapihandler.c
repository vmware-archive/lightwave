/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : ipcapihandler.c
 *
 * Abstract :
 *
 */
#include "includes.h"


DWORD
VmAfdLocalAPIHandler(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    DWORD * pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uApiType = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PCSTR pszLogString = NULL;
    time_t start_time = 0;
    time_t total_time = 0;

    if (dwRequestSize < sizeof (UINT32)){
      dwError = ERROR_INVALID_PARAMETER;
      BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!pConnectionContext)
    {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR (dwError);
    }

    start_time = time(NULL);

    uApiType = *((PUINT32)pRequest);

    switch(uApiType)
    {
          case VECS_IPC_CREATE_CERTSTORE:
                pszLogString = "VECS_IPC_CREATE_CERTSTORE";
                dwError = VecsIpcCreateCertStore(
                                        pConnectionContext,
                                        pRequest,
                                        dwRequestSize,
                                        &pResponse,
                                        &dwResponseSize
                                        );
                break;

           case VECS_IPC_DELETE_CERTSTORE:
                pszLogString = "VECS_IPC_DELETE_CERTSTORE";
                dwError = VecsIpcDeleteCertStore(
                                        pConnectionContext,
                                        pRequest,
                                        dwRequestSize,
                                        &pResponse,
                                        &dwResponseSize
                                        );
                break;

           case VECS_IPC_OPEN_CERTSTORE:
                pszLogString = "VECS_IPC_OPEN_CERTSTORE";
                dwError = VecsIpcOpenCertStore (
                                        pConnectionContext,
                                        pRequest,
                                        dwRequestSize,
                                        &pResponse,
                                        &dwResponseSize
                                        );
                break;

           case VECS_IPC_SET_PERMISSION:
                pszLogString = "VECS_IPC_SET_PERMISSION";
                dwError = VecsIpcSetPermission (
                                        pConnectionContext,
                                        pRequest,
                                        dwRequestSize,
                                        &pResponse,
                                        &dwResponseSize
                                        );
                break;

           case VECS_IPC_REVOKE_PERMISSION:
                pszLogString = "VECS_IPC_REVOKE_PERMISSION";
                dwError = VecsIpcRevokePermission (
                                        pConnectionContext,
                                        pRequest,
                                        dwRequestSize,
                                        &pResponse,
                                        &dwResponseSize
                                        );
                break;

           case VECS_IPC_GET_PERMISSIONS:
                pszLogString = "VECS_IPC_GET_PERMISSIONS";
                dwError = VecsIpcGetPermissions (
                                        pConnectionContext,
                                        pRequest,
                                        dwRequestSize,
                                        &pResponse,
                                        &dwResponseSize
                                        );
                break;

           case VECS_IPC_CHANGE_OWNER:
                pszLogString = "VECS_IPC_CHANGE_OWNER";
                dwError = VecsIpcChangeOwner (
                                        pConnectionContext,
                                        pRequest,
                                        dwRequestSize,
                                        &pResponse,
                                        &dwResponseSize
                                        );
                break;

           case VECS_IPC_ADD_ENTRY:
                pszLogString = "VECS_IPC_ADD_ENTRY";
                dwError = VecsIpcAddEntry (
                                          pConnectionContext,
                                          pRequest,
                                          dwRequestSize,
                                          &pResponse,
                                          &dwResponseSize
                                          );
                break;

           case VECS_IPC_DELETE_ENTRY:
                pszLogString = "VECS_IPC_DELETE_ENTRY";
                dwError = VecsIpcDeleteEntry (
                                              pConnectionContext,
                                              pRequest,
                                              dwRequestSize,
                                              &pResponse,
                                              &dwResponseSize
                                              );
                break;

           case VECS_IPC_BEGIN_ENUM_ENTRIES:
                pszLogString = "VECS_IPC_BEGIN_ENUM_ENTRIES";
                dwError = VecsIpcBeginEnumEntries (
                                                   pConnectionContext,
                                                   pRequest,
                                                   dwRequestSize,
                                                   &pResponse,
                                                   &dwResponseSize
                                                  );
                break;

           case VECS_IPC_ENUM_ENTRIES:
                pszLogString = "VECS_IPC_ENUM_ENTRIES";
                dwError = VecsIpcEnumEntries (
                                                pConnectionContext,
                                                pRequest,
                                                dwRequestSize,
                                                &pResponse,
                                                &dwResponseSize
                                             );
                break;

           case VECS_IPC_GET_ENTRY_BY_ALIAS:
                pszLogString = "VECS_IPC_GET_ENTRY_BY_ALIAS";
                dwError = VecsIpcGetEntryByAlias (
                                                  pConnectionContext,
                                                  pRequest,
                                                  dwRequestSize,
                                                  &pResponse,
                                                  &dwResponseSize
                                                 );
                break;

           case VECS_IPC_GET_KEY_BY_ALIAS:
                pszLogString = "VECS_IPC_GET_KEY_BY_ALIAS";
                dwError = VecsIpcGetKeyByAlias (
                                                pConnectionContext,
                                                pRequest,
                                                dwRequestSize,
                                                &pResponse,
                                                &dwResponseSize
                                               );
                break;

           case VECS_IPC_ENUM_STORES:
                pszLogString = "VECS_IPC_ENUM_STORES";
                dwError = VecsIpcEnumStores (
                                             pConnectionContext,
                                             pRequest,
                                             dwRequestSize,
                                             &pResponse,
                                             &dwResponseSize
                                            );
                break;

           case VECS_IPC_GET_ENTRY_COUNT:
                pszLogString = "VECS_IPC_GET_ENTRY_COUNT";
                dwError = VecsIpcGetEntryCount (
                                             pConnectionContext,
                                             pRequest,
                                             dwRequestSize,
                                             &pResponse,
                                             &dwResponseSize
                                            );
                break;

           case VECS_IPC_CLOSE_CERTSTORE:
                pszLogString = "VECS_IPC_CLOSE_CERTSTORE";
                dwError = VecsIpcCloseCertStore (
                                              pConnectionContext,
                                              pRequest,
                                              dwRequestSize,
                                              &pResponse,
                                              &dwResponseSize
                                             );
                break;

            case VECS_IPC_END_ENUM_ENTRIES:
                pszLogString = "VECS_IPC_END_ENUM_ENTRIES";
                dwError = VecsIpcEndEnumEntries (
                                              pConnectionContext,
                                              pRequest,
                                              dwRequestSize,
                                              &pResponse,
                                              &dwResponseSize
                                             );
                break;

                case VMAFD_IPC_GET_STATUS:

                    dwError = VmAfdIpcGetStatus(
                                    pConnectionContext,
                                    pRequest,
                                    dwRequestSize,
                                    &pResponse,
                                    &dwResponseSize
                                    );
                    break;

        case VMAFD_IPC_GET_DOMAIN_NAME:

            dwError = VmAfdIpcGetDomainName(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_SET_DOMAIN_NAME:

            dwError = VmAfdIpcSetDomainName(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_GET_DOMAIN_STATE:

            dwError = VmAfdIpcGetDomainState(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_GET_LDU:

            dwError = VmAfdIpcGetLDU(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_SET_LDU:

            dwError = VmAfdIpcSetLDU(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_GET_RHTTPPROXY_PORT:

            dwError = VmAfdIpcGetRHTTPProxyPort(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_SET_RHTTPPROXY_PORT:

            dwError = VmAfdIpcSetRHTTPProxyPort(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_SET_DC_PORT:

            dwError = VmAfdIpcSetDCPort(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_GET_CM_LOCATION:

            dwError = VmAfdIpcGetCMLocation(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_GET_LS_LOCATION:

            dwError = VmAfdIpcGetLSLocation(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_GET_DC_NAME:

            dwError = VmAfdIpcGetDCName(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_SET_DC_NAME:

            dwError = VmAfdIpcSetDCName(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_GET_MACHINE_ACCOUNT_INFO:

            dwError = VmAfdIpcGetMachineAccountInfo(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_GET_SITE_GUID:

            dwError = VmAfdIpcGetSiteGUID(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_GET_SITE_NAME:

            dwError = VmAfdIpcGetSiteName(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_GET_MACHINE_ID:

            dwError = VmAfdIpcGetMachineID(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_SET_MACHINE_ID:

            dwError = VmAfdIpcSetMachineID(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_PROMOTE_VMDIR:

            dwError = VmAfdIpcPromoteVmDir(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_DEMOTE_VMDIR:

            dwError = VmAfdIpcDemoteVmDir(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_JOIN_VMDIR:

            dwError = VmAfdIpcJoinVmDir(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_JOIN_VMDIR_2:

            dwError = VmAfdIpcJoinVmDir2(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_LEAVE_VMDIR:

            dwError = VmAfdIpcLeaveVmDir(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_CREATE_COMPUTER_ACCOUNT:

            dwError = VmAfdIpcCreateComputerAccount(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_JOIN_AD:

            dwError = VmAfdIpcJoinAD(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_LEAVE_AD:

            dwError = VmAfdIpcLeaveAD(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_QUERY_AD:

            dwError = VmAfdIpcQueryAD(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_FORCE_REPLICATION:

            dwError = VmAfdIpcForceReplication(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_GET_PNID:

            dwError = VmAfdIpcGetPNID(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_SET_PNID:

            dwError = VmAfdIpcSetPNID(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_GET_CA_PATH:

            dwError = VmAfdIpcGetCAPath(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_SET_CA_PATH:

            dwError = VmAfdIpcSetCAPath(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_TRIGGER_ROOT_CERTS_REFRESH:

            dwError = VmAfdIpcTriggerRootCertsRefresh(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_REFRESH_SITE_NAME:

            dwError = VmAfdIpcRefreshSiteName(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_CONFIGURE_DNS:

            dwError = VmAfdIpcConfigureDNS(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case VMAFD_IPC_JOIN_VALIDATE_CREDENTIALS:

            dwError = VmAfdIpcJoinValidateCredentials(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );

            break;
        case VMAFD_IPC_GET_DC_LIST:

            dwError = VmAfdIpcGetDCList(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case CDC_IPC_GET_DC_NAME:
            dwError = CdcIpcGetDCName(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        case CDC_IPC_GET_CDC_STATE:
            dwError = CdcIpcGetCurrentState(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );

            break;

        case CDC_IPC_ENUM_DC_ENTRIES:
            dwError = CdcIpcEnumDCEntries(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );

            break;
       case CDC_IPC_ENABLE_DEFAULT_HA:
            dwError = CdcIpcEnableDefaultHA(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );

            break;
       case CDC_IPC_ENABLE_LEGACY_HA:
            dwError = CdcIpcEnableLegacyModeHA(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );

            break;

       case CDC_IPC_GET_DC_STATUS_INFO:
            dwError = CdcIpcGetDCStatusInfo(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );

            break;

       case VMAFD_IPC_POST_HEARTBEAT:
            dwError = VmAfdIpcPostHeartbeatStatus(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

       case VMAFD_IPC_GET_HEARBEAT_STATUS:
            dwError = VmAfdIpcGetHeartbeatStatus(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

       case VMAFD_IPC_CHANGE_PNID:
            dwError = VmAfdIpcChangePNID(
                            pConnectionContext,
                            pRequest,
                            dwRequestSize,
                            &pResponse,
                            &dwResponseSize
                            );
            break;

        default:

            dwError = ERROR_INVALID_PARAMETER;
            break;

    }

    if (pszLogString)
    {
        total_time = time(NULL) - start_time;
        if (total_time > 1)
        {
            VmAfdLog(VMAFD_DEBUG_ANY,
                     "%s: elasped time %d seconds",
                     pszLogString, total_time);
        }
    }

    BAIL_ON_VMAFD_ERROR(dwError);

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

cleanup:
    return dwError;

error:
        if (ppResponse)
        {
                *ppResponse = NULL;
        }
        if (pdwResponseSize)
        {
                *pdwResponseSize = 0;
        }
        if (pResponse)
        {
                VMAFD_SAFE_FREE_MEMORY (pResponse);
        }
        goto cleanup;
}

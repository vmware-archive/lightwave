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

void VmAfdCloseFileCRLFileHandle(void **ctx);

DWORD
Srv_VmAfdRpcGetStatus(
    handle_t      hBinding, /* IN     */
    PVMAFD_STATUS pStatus   /* IN OUT */
    )
{
    DWORD dwError = 0;
    /* ncalrpc is needed for self-ping operation at startup */
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_NCALRPC
                     | VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    BAIL_ON_VMAFD_INVALID_POINTER(pStatus, dwError);

    *pStatus = VmAfdSrvGetStatus();

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetStatus failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcGetDomainName(
    rpc_binding_handle_t hBinding,          /* IN     */
    PWSTR*   ppwszDomain        /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDomain = NULL;
    PWSTR pwszDomain_rpc = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszDomain, dwError);

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainName(&pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszDomain, &pwszDomain_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszDomain = pwszDomain_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDomain);

    return dwError;

error:

    if (ppwszDomain)
    {
        *ppwszDomain = NULL;
    }

    if (pwszDomain_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszDomain_rpc);
    }

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetDomainName failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcSetDomainName(
    rpc_binding_handle_t hBinding,         /* IN     */
    PWSTR    pwszDomain        /* IN     */
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    if IsNullOrEmptyString(pwszDomain)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainName(pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdRpcSetDomainName succeeded.");

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetDomainName failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcGetDomainState(
    rpc_binding_handle_t hBinding,         /* IN     */
    PVMAFD_DOMAIN_STATE  pDomainState      /*    OUT */
    )
{
    DWORD dwError = 0;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    BAIL_ON_VMAFD_INVALID_POINTER(pDomainState, dwError);

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pDomainState = domainState;

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetDomainState failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcGetLDU(
    rpc_binding_handle_t hBinding,      /* IN     */
    PWSTR*   ppwszLDU       /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszLDU = NULL;
    PWSTR pwszLDU_rpc = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszLDU, dwError);

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetLDU(&pwszLDU);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszLDU, &pwszLDU_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszLDU = pwszLDU_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszLDU);

    return dwError;

error:

    if (ppwszLDU)
    {
        *ppwszLDU = NULL;
    }

    if (pwszLDU_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszLDU_rpc);
    }

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetLDU failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcGetRHTTPProxyPort(
    rpc_binding_handle_t hBinding,
    PDWORD pdwPort
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;
    DWORD dwPort = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pdwPort, dwError);

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetRHTTPProxyPort(&dwPort);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pdwPort = dwPort;

cleanup:

    return dwError;

error:

    if (pdwPort)
    {
        *pdwPort = 0;
    }
    VmAfdLog(VMAFD_DEBUG_ERROR, "RpcVmAfdGetRHTTPProxyPort failed. Error(%u)",
                              dwError);

    goto cleanup;
}

DWORD
Srv_VmAfdRpcSetRHTTPProxyPort(
    rpc_binding_handle_t hBinding,      /* IN     */
    DWORD dwPort                        /* IN     */
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetRHTTPProxyPort(dwPort);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetRHTTPProxyPort failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcSetDCPort(
    rpc_binding_handle_t hBinding,      /* IN     */
    DWORD dwPort                        /* IN     */
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDCPort(dwPort);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetDCPort failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcSetLDU(
    rpc_binding_handle_t hBinding,         /* IN     */
    PWSTR    pwszLDU           /* IN     */
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    if IsNullOrEmptyString(pwszLDU)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetLDU(pwszLDU);
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdRpcSetLDU succeeded.");

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetLDU failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcGetCMLocation(
    rpc_binding_handle_t hBinding,        /* IN     */
    PWSTR*   ppwszCMLocation  /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszCMLocation = NULL;
    PWSTR pwszCMLocation_rpc = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszCMLocation, dwError);

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetCMLocation(&pwszCMLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszCMLocation, &pwszCMLocation_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszCMLocation = pwszCMLocation_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszCMLocation);

    return dwError;

error:

    if (ppwszCMLocation)
    {
        *ppwszCMLocation = NULL;
    }

    if (pwszCMLocation_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszCMLocation_rpc);
    }

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetCMLocation failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcGetLSLocation(
    rpc_binding_handle_t hBinding,        /* IN     */
    PWSTR*   ppwszLSLocation  /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszLSLocation = NULL;
    PWSTR pwszLSLocation_rpc = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszLSLocation, dwError);

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetLSLocation(&pwszLSLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszLSLocation, &pwszLSLocation_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszLSLocation = pwszLSLocation_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszLSLocation);

    return dwError;

error:

    if (ppwszLSLocation)
    {
        *ppwszLSLocation = NULL;
    }

    if (pwszLSLocation_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszLSLocation_rpc);
    }

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetLSLocation failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcGetDCName(
    rpc_binding_handle_t hBinding,      /* IN     */
    PWSTR*   ppwszDCName    /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDCName = NULL;
    PWSTR pwszDCName_rpc = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszDCName, dwError);

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDCName(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszDCName, &pwszDCName_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszDCName = pwszDCName_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDCName);

    return dwError;

error:

    if (ppwszDCName)
    {
        *ppwszDCName = NULL;
    }

    if (pwszDCName_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszDCName_rpc);
    }

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetDCName failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcSetDCName(
    rpc_binding_handle_t hBinding,      /* IN     */
    PWSTR    pwszDCName     /* IN     */
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    if IsNullOrEmptyString(pwszDCName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDCName(pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdRpcSetDCName succeeded.");

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetDCName failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcGetPNID(
    rpc_binding_handle_t hBinding,
    PWSTR* ppwszPNID
    )
{
    DWORD dwError = 0;
    PWSTR pwszPNID = NULL;
    PWSTR pwszPNID_rpc = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszPNID, dwError);

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetPNID(&pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszPNID, &pwszPNID_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszPNID = pwszPNID_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszPNID);

    return dwError;

error:

    if (ppwszPNID)
    {
        *ppwszPNID = NULL;
    }

    if (pwszPNID_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszPNID_rpc);
    }

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetPNID failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcSetPNID(
    rpc_binding_handle_t hBinding,
    PWSTR pwszPNID
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    if IsNullOrEmptyString(pwszPNID)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetPNID(pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdRpcSetPNID succeeded.");

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetPNID failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcGetCAPath(
    rpc_binding_handle_t hBinding,
    PWSTR* ppwszPath
    )
{
    DWORD dwError = 0;
    PWSTR pwszPath = NULL;
    PWSTR pwszPath_rpc = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszPath, dwError);

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetCAPath(&pwszPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszPath, &pwszPath_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszPath = pwszPath_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszPath);

    return dwError;

error:

    if (ppwszPath)
    {
        *ppwszPath = NULL;
    }

    if (pwszPath_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszPath_rpc);
    }

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetCAPath failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcSetCAPath(
    rpc_binding_handle_t hBinding,
    PWSTR pwszPath
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    if IsNullOrEmptyString(pwszPath)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetCAPath(pwszPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdRpcSetCAPath succeeded.");

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetCAPath failed. Error(%u)",
                              dwError);
    goto cleanup;
}

UINT32
Srv_VmAfdRpcGetSiteGUID(
    rpc_binding_handle_t hBinding, /* IN              */
    PWSTR*               ppwszGUID /*    OUT          */
    )
{
    DWORD dwError = 0;
    PWSTR pwszGUID = NULL;
    PWSTR pwszGUID_rpc = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszGUID, dwError);

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetSiteGUID(&pwszGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszGUID, &pwszGUID_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszGUID = pwszGUID_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszGUID);

    return dwError;

error:

    if (ppwszGUID)
    {
        *ppwszGUID = NULL;
    }
    if (pwszGUID_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszGUID_rpc);
    }
    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetSiteGUID failed. Error(%u)",
                              dwError);

    goto cleanup;
}

UINT32
Srv_VmAfdRpcGetMachineID(
    rpc_binding_handle_t hBinding, /* IN              */
    PWSTR*               ppwszGUID /*    OUT          */
    )
{
    DWORD dwError = 0;
    PWSTR pwszGUID = NULL;
    PWSTR pwszGUID_rpc = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszGUID, dwError);

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvCfgGetMachineID(&pwszGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszGUID, &pwszGUID_rpc);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszGUID = pwszGUID_rpc;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszGUID);

    return dwError;

error:

    if (ppwszGUID)
    {
        *ppwszGUID = NULL;
    }
    if (pwszGUID_rpc)
    {
        VmAfdRpcServerFreeMemory(pwszGUID_rpc);
    }
    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcGetMachineID failed. Error(%u)",
                              dwError);

    goto cleanup;
}

UINT32
Srv_VmAfdRpcSetMachineID(
    rpc_binding_handle_t hBinding, /* IN              */
    PWSTR                pwszGUID /*  IN              */
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetMachineID(pwszGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcSetMachineID failed. Error(%u)",
                              dwError);

    goto cleanup;
}

DWORD
Srv_VmAfdRpcQueryAD(
    rpc_binding_handle_t hBinding,  /* IN              */
    PWSTR  *ppwszComputer,          /*    OUT          */
    PWSTR  *ppwszDomain,            /*    OUT          */
    PWSTR  *ppwszDistinguishedName, /*    OUT          */
    PWSTR  *ppwszNetbiosName        /*    OUT          */
    )
{
    DWORD dwError = 0;
    PWSTR  pwszComputer = NULL;
    PWSTR  pwszDomain = NULL;
    PWSTR  pwszDistinguishedName = NULL;
    PWSTR  pwszNetbiosName = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvQueryAD(
                      &pwszComputer,
                      &pwszDomain,
                      &pwszDistinguishedName,
                      &pwszNetbiosName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszComputer = pwszComputer;
    *ppwszDomain = pwszDomain;
    *ppwszDistinguishedName = pwszDistinguishedName;
    *ppwszNetbiosName = pwszNetbiosName;

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdRpcQueryAD succeeded.");

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcQueryAD failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcForceReplication(
    rpc_binding_handle_t hBinding, /* IN              */
    PWSTR   pwszServerName         /* IN              */
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    BAIL_ON_VMAFD_INVALID_POINTER(pwszServerName, dwError);

    dwError = VmAfSrvForceReplication(pwszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdRpcForceReplication succeeded.");

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcForceReplication failed. Error(%u)",
                              dwError);
    goto cleanup;
}

DWORD
Srv_VmAfdRpcTriggerRootCertsRefresh(
    handle_t hBinding
)
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRootFetchTask(TRUE);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ERROR, "VmAfdRpcTriggerRootCertsRefresh failed. Error(%u)",
                              dwError);

    goto cleanup;
}

/*
 * Creates a certificate store
 */
DWORD
Srv_VecsRpcCreateCertStore(
     rpc_binding_handle_t hBinding,
     PWSTR pszStoreName,
     PWSTR pszPassword,
     vecs_store_handle_t *ppStore
     )
{
    DWORD dwError = 0;
    PVECS_SERV_STORE pStore = NULL;
    size_t dwStoreNameLength = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;
    PVM_AFD_CONNECTION_CONTEXT pRootConnectionContext = NULL;
    PVECS_SRV_STORE_HANDLE pStoreHandle = NULL;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszStoreName || !ppStore)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetStringLengthW (
                    pszStoreName,
                    &dwStoreNameLength
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (dwStoreNameLength > STORE_LABEL_MAX_LENGTH)
    {
        dwError = ERROR_LABEL_TOO_LONG;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCreateAnonymousConnectionContext(&pRootConnectionContext);
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsSrvCreateCertStoreWithAuth(
                pszStoreName,
                pszPassword,
                pRootConnectionContext,
                &pStoreHandle);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetStoreFromHandle(
                             pStoreHandle,
                             pRootConnectionContext->pSecurityContext,
                             &pStore
                             );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppStore = pStore;

cleanup:
    if (pStoreHandle)
    {
        VecsSrvCloseCertStoreHandle(pStoreHandle, pRootConnectionContext);
    }
    if (pRootConnectionContext)
    {
        VmAfdFreeConnectionContext(pRootConnectionContext);
    }

    return dwError;
error:
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "VmAfdRpcCreateCertStore failed. Error (%u)",
            dwError);
    if (pStore)
    {
        VecsSrvReleaseCertStore(pStore);
    }
    goto cleanup;
}

DWORD
Srv_VecsRpcOpenCertStore(
        rpc_binding_handle_t hBinding,
        PWSTR pszStoreName,
        PWSTR pszPassword,
        vecs_store_handle_t *ppStore
        )
{
    DWORD dwError = 0;
    PVECS_SERV_STORE pStore = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszStoreName || !ppStore)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsSrvOpenCertStore(
                pszStoreName,
                pszPassword,
                &pStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppStore = pStore;

cleanup:
    return dwError;
error:
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);
    if (pStore)
    {
        VecsSrvReleaseCertStore(pStore);
    }
    goto cleanup;
}

DWORD
Srv_VecsRpcCloseCertStore(
        rpc_binding_handle_t hBinding,
        vecs_store_handle_t *ppStore
        )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!ppStore || !(*ppStore))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsSrvCloseCertStore(*ppStore);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppStore = NULL;

cleanup:

    return dwError;
error:
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);


    goto cleanup;
}

UINT32
Srv_VecsRpcEnumCertStore(
    rpc_binding_handle_t hBinding,
    PVMAFD_CERT_STORE_ARRAY * ppCertStoreArray
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_STORE_ARRAY pCertStoreArray = NULL;
    PWSTR* ppszStoreNameArray = NULL;
    DWORD dwCount = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!ppCertStoreArray)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsSrvEnumCertStore (
                  &ppszStoreNameArray,
                  &dwCount
                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsSrvRpcAllocateCertStoreArray(
                    ppszStoreNameArray,
                    dwCount,
                    &pCertStoreArray);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppCertStoreArray = pCertStoreArray;

cleanup:

    if (ppszStoreNameArray)
    {
        VmAfdFreeStringArrayW(ppszStoreNameArray, dwCount);
    }

    return dwError;

error:

    if (ppCertStoreArray)
    {
        *ppCertStoreArray = NULL;
    }
    if (pCertStoreArray)
    {
        VecsSrvRpcFreeCertStoreArray(pCertStoreArray);
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);


    goto cleanup;
}

UINT32
Srv_VecsRpcDeleteCertStore(
    rpc_binding_handle_t hBinding,
    PWSTR                pwszStoreName
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(pwszStoreName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsSrvDeleteCertStore(pwszStoreName);
    BAIL_ON_VMAFD_ERROR(dwError);

error:
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    return dwError;
}

UINT32
Srv_VecsRpcBeginEnumCerts(
    rpc_binding_handle_t      hBinding,
    vecs_store_handle_t       pStore,
    UINT32                    dwMaxCount,
    UINT32                    dwInfoLevel,
    UINT32*                   pdwLimit,
    vecs_entry_enum_handle_t* ppEnumContext
    )
{
    DWORD dwError = 0;
    PVECS_SRV_ENUM_CONTEXT pEnumContext = NULL;
    PVECS_SERV_STORE pStore2 = (PVECS_SERV_STORE)pStore;
    ENTRY_INFO_LEVEL infoLevel = ENTRY_INFO_LEVEL_UNDEFINED;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pStore || !ppEnumContext || !pdwLimit)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsSrvValidateInfoLevel(dwInfoLevel,&infoLevel);
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsSrvAllocateCertEnumContext(
                    pStore2,
                    dwMaxCount,
                    infoLevel,
                    &pEnumContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppEnumContext = pEnumContext;
    *pdwLimit = pEnumContext->dwLimit;

cleanup:

    return dwError;

error:

    if (ppEnumContext)
    {
        *ppEnumContext = NULL;
    }
    if (pEnumContext)
    {
      VecsSrvReleaseEnumContext(pEnumContext);
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_VecsRpcEnumCerts(
    rpc_binding_handle_t     hBinding,
    vecs_entry_enum_handle_t pEnumContext,
    PVMAFD_CERT_ARRAY*       ppCertContainer
    )
{
    DWORD dwError = 0;
    PVECS_SRV_ENUM_CONTEXT pContext = NULL;
    PVMAFD_CERT_ARRAY pCertContainer = NULL;
    PVMAFD_CERT_ARRAY pCertContainer_rpc = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pEnumContext || !ppCertContainer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pContext = (PVECS_SRV_ENUM_CONTEXT)pEnumContext;

    pContext = VecsSrvAcquireEnumContext(pContext);

    dwError = VecsSrvEnumCerts(pContext, &pCertContainer);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsRpcAllocateCertArray(pCertContainer, &pCertContainer_rpc);
    BAIL_ON_VMAFD_ERROR (dwError);

    pContext->dwIndex += pCertContainer_rpc->dwCount;

    *ppCertContainer = pCertContainer_rpc;

cleanup:
    if (pContext)
    {
        VecsSrvReleaseEnumContext(pContext);
    }
    if (pCertContainer)
    {
      VecsFreeCertArray(pCertContainer);
    }


    return dwError;

error:
    if (ppCertContainer)
    {
      *ppCertContainer = NULL;
    }
    if (pCertContainer_rpc)
    {
      VecsSrvRpcFreeCertArray(pCertContainer_rpc);
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_VecsRpcEndEnumCerts(
    rpc_binding_handle_t      hBinding,
    vecs_entry_enum_handle_t* ppEnumContext
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!ppEnumContext || !*ppEnumContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    VecsSrvReleaseEnumContext((PVECS_SRV_ENUM_CONTEXT)*ppEnumContext);

    *ppEnumContext = NULL;

cleanup:

    return dwError;

error:
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_VecsRpcGetEntryCount(
    rpc_binding_handle_t hBinding,
    vecs_store_handle_t pStore,
    PDWORD pdwSize
    )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;
    PVECS_SERV_STORE pStore2 = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pStore || !pdwSize)
    {
      dwError = ERROR_INVALID_PARAMETER;
      BAIL_ON_VMAFD_ERROR (dwError);
    }

    pStore2 = (PVECS_SERV_STORE)pStore;

    pStore2 = VecsSrvAcquireCertStore(pStore2);

    dwError = VecsSrvGetEntryCount(
                      pStore2,
                      &dwSize);
    BAIL_ON_VMAFD_ERROR (dwError);
    *pdwSize = dwSize;

cleanup:
    if (pStore2)
    {
        VecsSrvReleaseCertStore(pStore2);
    }

    return dwError;
error:
    if (pdwSize)
    {
        *pdwSize = 0;
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_VecsRpcGetCertificateByAlias(
    rpc_binding_handle_t hBinding,
    vecs_store_handle_t pStore,
    PWSTR pszAliasName,
    PWSTR *pszCertificate
    )
{
    DWORD dwError = 0;
    PWSTR pszCert = NULL;
    PWSTR pszCert_rpc = NULL;
    PVECS_SERV_STORE pStore2 = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pStore ||
        IsNullOrEmptyString(pszAliasName) ||
        !pszCertificate
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pStore2 = (PVECS_SERV_STORE)pStore;

    pStore2 = VecsSrvAcquireCertStore(pStore2);

    dwError = VecsSrvGetCertificateByAlias(
                      pStore2,
                      pszAliasName,
                      &pszCert
                      );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdRpcServerAllocateStringW(pszCert, &pszCert_rpc);
    BAIL_ON_VMAFD_ERROR (dwError);

    *pszCertificate = pszCert_rpc;
cleanup:
    VMAFD_SAFE_FREE_MEMORY (pszCert);
    if (pStore2)
    {
      VecsSrvReleaseCertStore(pStore2);
    }

    return dwError;
error:
    if (pszCertificate)
    {
        *pszCertificate = NULL;
    }
    if (pszCert_rpc)
    {
      VmAfdRpcServerFreeMemory(pszCert_rpc);
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_VecsRpcGetPrivateKeyByAlias(
    rpc_binding_handle_t hBinding,
    vecs_store_handle_t pStore,
    PWSTR pszAliasName,
    PWSTR pszPassword,
    PWSTR *ppszPrivateKey
    )
{
    DWORD dwError = 0;
    PWSTR pszPrivateKey = NULL;
    PWSTR pszPrivateKey_Rpc = NULL;
    PVECS_SERV_STORE pStore2 = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pStore ||
        IsNullOrEmptyString(pszAliasName) ||
        !ppszPrivateKey
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pStore2 = (PVECS_SERV_STORE)pStore;

    pStore2 = VecsSrvAcquireCertStore(pStore2);

    dwError = VecsSrvGetPrivateKeyByAlias(
                      pStore2,
                      pszAliasName,
                      pszPassword,
                      &pszPrivateKey
                      );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdRpcServerAllocateStringW(pszPrivateKey, &pszPrivateKey_Rpc);
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszPrivateKey = pszPrivateKey_Rpc;
cleanup:
    VMAFD_SAFE_FREE_MEMORY (pszPrivateKey);
    if (pStore2)
    {
      VecsSrvReleaseCertStore(pStore);
    }

    return dwError;
error:
    if (ppszPrivateKey)
    {
        *ppszPrivateKey = NULL;
    }
    if (pszPrivateKey_Rpc)
    {
        VmAfdRpcServerFreeMemory(pszPrivateKey_Rpc);
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_VecsRpcAddCertificate(
    rpc_binding_handle_t hBinding,
    vecs_store_handle_t pStore,
    UINT32 entryType,
    PWSTR pszAliasName,
    PWSTR pszCertificate,
    PWSTR pszPrivateKey,
    PWSTR pszPassword,
    UINT32 bAutoRefresh
    )
{
    DWORD dwError = 0;
    PVECS_SERV_STORE pStore2 = NULL;
    CERT_ENTRY_TYPE entryType1 = CERT_ENTRY_TYPE_UNKNOWN;
    BOOLEAN bAutoRefresh1 = FALSE;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pStore)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    dwError = VecsSrvValidateEntryType(entryType, &entryType1);
    BAIL_ON_VMAFD_ERROR (dwError);

    if (bAutoRefresh)
    {
        bAutoRefresh1 = TRUE;
    }

    pStore2 = (PVECS_SERV_STORE)pStore;

    pStore2 = VecsSrvAcquireCertStore(pStore2);

    dwError = VecsSrvAddCertificate(
                    pStore2,
                    entryType1,
                    pszAliasName,
                    pszCertificate,
                    pszPrivateKey,
                    pszPassword,
                    bAutoRefresh1
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    if (pStore2)
    {
      VecsSrvReleaseCertStore(pStore2);
    }
    return dwError;
error:
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_VecsRpcGetEntryTypeByAlias(
    rpc_binding_handle_t hBinding,
    vecs_store_handle_t pStore,
    PWSTR pszAliasName,
    UINT32* pEntryType
    )
{
    DWORD dwError = 0;
    CERT_ENTRY_TYPE entryType = CERT_ENTRY_TYPE_UNKNOWN;
    PVECS_SERV_STORE pStore2 = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pStore ||
        IsNullOrEmptyString(pszAliasName) ||
        !pEntryType
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pStore2 = (PVECS_SERV_STORE)pStore;

    pStore2 = VecsSrvAcquireCertStore (pStore2);

    dwError = VecsSrvGetEntryTypeByAlias(
                pStore2,
                pszAliasName,
                &entryType
                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *pEntryType = entryType;

cleanup:
    if (pStore2)
    {
        VecsSrvReleaseCertStore(pStore2);
    }

    return dwError;
error:
    if (pEntryType)
    {
        *pEntryType = 0;
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_VecsRpcGetEntryDateByAlias(
    rpc_binding_handle_t hBinding,
    vecs_store_handle_t pStore,
    PWSTR pszAliasName,
    UINT32 *pdwDate
    )
{
    DWORD dwError = 0;
    DWORD dwDate = 0;
    PVECS_SERV_STORE pStore2 = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString (pszAliasName) ||
        !pStore ||
        !pdwDate
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pStore2 = (PVECS_SERV_STORE)pStore;

    pStore2 = VecsSrvAcquireCertStore(pStore2);

    dwError = VecsSrvGetEntryDateByAlias(
                          pStore2,
                          pszAliasName,
                          &dwDate
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwDate = dwDate;

cleanup:
    VecsSrvReleaseCertStore (pStore2);
    return dwError;
error:
    if (pdwDate)
    {
        *pdwDate = 0;
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_VecsRpcGetEntryByAlias(
    rpc_binding_handle_t hBinding,
    vecs_store_handle_t pStore,
    PWSTR pszAliasName,
    UINT32 dwInfoLevel,
    PVMAFD_CERT_ARRAY *ppCertArray
    )
{
    DWORD dwError = 0;
    PVECS_SERV_STORE pStore2 = NULL;
    ENTRY_INFO_LEVEL infoLevel = ENTRY_INFO_LEVEL_UNDEFINED;
    PVMAFD_CERT_ARRAY pCertContainer_rpc = NULL;
    PVMAFD_CERT_ARRAY pCertContainer = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pStore ||
        IsNullOrEmptyString (pszAliasName) ||
        !ppCertArray
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsSrvValidateInfoLevel (dwInfoLevel, &infoLevel);
    BAIL_ON_VMAFD_ERROR (dwError);

    pStore2 = (PVECS_SERV_STORE)pStore;

    pStore2 = VecsSrvAcquireCertStore(pStore2);

    dwError = VecsSrvGetEntryByAlias(
                    pStore,
                    pszAliasName,
                    infoLevel,
                    &pCertContainer);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsRpcAllocateCertArray(pCertContainer, &pCertContainer_rpc);
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppCertArray = pCertContainer_rpc;

cleanup:
    VecsSrvReleaseCertStore (pStore2);
    if (pCertContainer)
    {
        VecsFreeCertArray (pCertContainer);
    }

    return dwError;
error:
    if (ppCertArray)
    {
        *ppCertArray = NULL;
    }
    if (pCertContainer_rpc)
    {
        VecsSrvRpcFreeCertArray(pCertContainer_rpc);
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_VecsRpcDeleteCertificate(
    rpc_binding_handle_t hBinding,
    vecs_store_handle_t pStore,
    PWSTR pszAliasName
    )
{
    DWORD dwError = 0;
    PVECS_SERV_STORE pStore2 = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString (pszAliasName) ||
        !pStore)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pStore2 = (PVECS_SERV_STORE)pStore;

    pStore2 = VecsSrvAcquireCertStore(pStore2);

    dwError = VecsSrvDeleteCertificate(
                  pStore2,
                  pszAliasName
                  );
    BAIL_ON_VMAFD_ERROR (dwError);
cleanup:
    if (pStore2)
    {
        VecsSrvReleaseCertStore(pStore2);
    }

    return dwError;
error:
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_VmAfdRpcGetHeartbeatStatus(
    rpc_binding_handle_t hBinding,
    PVMAFD_HB_STATUS_W  *ppHeartbeatStatus
    )
{
    DWORD dwError = 0;
    PVMAFD_HB_STATUS_W pHeartbeatStatus = NULL;
    PVMAFD_HB_STATUS_W pRpcHeartbeatStatus = NULL;

    if (!ppHeartbeatStatus)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfSrvGetHeartbeatStatus(
                                    &pHeartbeatStatus
                                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdRpcAllocateHeartbeatStatus(
                                    pHeartbeatStatus,
                                    &pRpcHeartbeatStatus
                                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppHeartbeatStatus = pRpcHeartbeatStatus;

cleanup:

    if (pHeartbeatStatus)
    {
        VmAfdFreeHbStatusW(pHeartbeatStatus);
    }
    return dwError;
error:

    if (ppHeartbeatStatus)
    {
        *ppHeartbeatStatus = NULL;
    }
    if (pRpcHeartbeatStatus)
    {
        VmAfdRpcFreeHeartbeatStatus(pRpcHeartbeatStatus);
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_VmAfdRpcGetSiteName(
    rpc_binding_handle_t hBinding,
    PWSTR*  ppwszSiteName
    )
{
    DWORD dwError = 0;
    PWSTR pwszSiteName = NULL;
    PWSTR pwszRpcSiteName = NULL;

    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetSiteName(&pwszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateStringW(pwszSiteName, &pwszRpcSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszSiteName = pwszRpcSiteName;

cleanup:
    VMAFD_SAFE_FREE_STRINGW(pwszSiteName);
    return dwError;
error:
    if (pwszSiteName)
    {
        VmAfdRpcServerFreeMemory(pwszRpcSiteName);
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_CdcRpcGetCurrentState(
    rpc_binding_handle_t hBinding,
    PCDC_DC_STATE pState
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcSrvGetCurrentState(pState);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;
error:
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_CdcRpcEnableDefaultHA(
    rpc_binding_handle_t hBinding
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcSrvEnableDefaultHA(gVmafdGlobals.pCdcContext);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;
error:
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_CdcRpcEnableLegacyModeHA(
    rpc_binding_handle_t hBinding
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcSrvEnableLegacyModeHA(gVmafdGlobals.pCdcContext);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;
error:
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_CdcRpcGetDCName(
    rpc_binding_handle_t hBinding,
    PWSTR pszDomainName,
    PWSTR pszSiteName,
    UINT32 dwFlags,
    PCDC_DC_INFO_W *ppDomainControllerInfo
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W pAffinitizedDC = NULL;
    PCDC_DC_INFO_W pRpcAffinitizedDC = NULL;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcSrvGetDCName(pszDomainName, dwFlags, &pAffinitizedDC);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcRpcServerAllocateDCInfoW(pAffinitizedDC, &pRpcAffinitizedDC);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppDomainControllerInfo = pRpcAffinitizedDC;

cleanup:
    if (pAffinitizedDC)
    {
        VmAfdFreeDomainControllerInfoW(pAffinitizedDC);
    }
    return dwError;
error:
    if (pRpcAffinitizedDC)
    {
        CdcRpcServerFreeDCInfoW(pRpcAffinitizedDC);
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_CdcRpcEnumDCEntries(
    rpc_binding_handle_t hBinding,
    PCDC_DC_ENTRIES_W *ppDCEntries
    )
{
    DWORD dwError = 0;
    PCDC_DC_ENTRIES_W pDCEntries = NULL;
    PWSTR *pwszDCEntriesArray = NULL;
    PWSTR *pwszRpcDCEntriesArray = NULL;
    DWORD dwDCEntriesCount = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcSrvEnumDCEntries(
                    &pwszDCEntriesArray,
                    &dwDCEntriesCount
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateMemory(
                    sizeof(CDC_DC_ENTRIES_W),
                    (VOID*)&pDCEntries);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwDCEntriesCount > 0)
    {
        dwError = VmAfdRpcServerAllocateStringArrayW(
                        dwDCEntriesCount,
                        (PCWSTR*)pwszDCEntriesArray,
                        &pwszRpcDCEntriesArray);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pDCEntries->ppszEntries = pwszRpcDCEntriesArray;
    pDCEntries->dwCount = dwDCEntriesCount;

    *ppDCEntries = pDCEntries;

cleanup:
    VmAfdFreeStringArrayW(pwszDCEntriesArray, dwDCEntriesCount);
    return dwError;
error:
    VmAfdRpcServerFreeStringArrayW(pwszRpcDCEntriesArray, dwDCEntriesCount);
    VmAfdRpcServerFreeMemory(pDCEntries);
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

UINT32
Srv_CdcRpcGetDCStatusInfo(
    rpc_binding_handle_t hBinding,
    PWSTR pwszDCName,
    PWSTR pwszDomainName,
    PCDC_DC_STATUS_INFO_W *ppDCStatusInfo,
    PVMAFD_HB_STATUS_W    *ppHbStatus
    )
{
    DWORD dwError = 0;
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTHZ;
    PCDC_DC_STATUS_INFO_W pDCStatusInfo = NULL;
    PVMAFD_HB_STATUS_W    pHbStatus = NULL;

    PCDC_DC_STATUS_INFO_W pRpcDCStatusInfo = NULL;
    PVMAFD_HB_STATUS_W    pRpcHbStatus = NULL;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcSrvGetDCStatusInfo(
                    pwszDCName,
                    pwszDomainName,
                    &pDCStatusInfo,
                    &pHbStatus
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pDCStatusInfo)
    {
        dwError = CdcRpcAllocateDCStatusInfo(
                                       pDCStatusInfo,
                                       &pRpcDCStatusInfo
                                       );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pHbStatus)
    {
        dwError = VmAfdRpcAllocateHeartbeatStatus(
                                        pHbStatus,
                                        &pRpcHbStatus
                                        );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppDCStatusInfo = pRpcDCStatusInfo;
    *ppHbStatus = pRpcHbStatus;

cleanup:

    if (pDCStatusInfo)
    {
        VmAfdFreeCdcStatusInfoW(pDCStatusInfo);
    }
    if (pHbStatus)
    {
        VmAfdFreeHbStatusW(pHbStatus);
    }
    return dwError;
error:

    if (ppDCStatusInfo)
    {
        *ppDCStatusInfo = NULL;
    }
    if (ppHbStatus)
    {
        *ppHbStatus = NULL;
    }
    if (pRpcDCStatusInfo)
    {
        CdcRpcFreeDCStatuInfo(pRpcDCStatusInfo);
    }
    if (pRpcHbStatus)
    {
        VmAfdRpcFreeHeartbeatStatus(pRpcHbStatus);
    }
    VmAfdLog (
            VMAFD_DEBUG_ERROR,
            "%s failed. Error (%u)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

void
vecs_store_handle_t_rundown(void *ctx)
{
    if (ctx)
    {
        PVECS_SERV_STORE pStore = (PVECS_SERV_STORE)ctx;

        VecsSrvReleaseCertStore(pStore);
    }
}

void
vecs_entry_enum_handle_t_rundown(void *ctx)
{
    if (ctx)
    {
        PVECS_SRV_ENUM_CONTEXT pContext = (PVECS_SRV_ENUM_CONTEXT)ctx;

        VecsSrvReleaseEnumContext(pContext);
    }
}

//
// Rundown callback that will free the tracking information we use for paged
// log retrieval.
//
void vmafd_superlog_cookie_t_rundown(void *ctx)
{
    if (ctx)
    {
        VmAfdFreeMemory(ctx);
    }
}


UINT32
Srv_RpcVmAfdSuperLogEnable(
    handle_t    hBinding
    )
{
    DWORD  dwError = 0;

    /* ncalrpc is needed for self-ping operation at startup */

    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_NCALRPC
                     | VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdEnableSuperLogging(gVmafdGlobals.pLogger);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "RpcVmAfdSuperLogEnable failed. Error(%u)", dwError);
    goto cleanup;
}

UINT32
Srv_RpcVmAfdSuperLogDisable(
    handle_t    hBinding
    )
{
    DWORD  dwError = 0;

    /* ncalrpc is needed for self-ping operation at startup */
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_NCALRPC
                     | VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdDisableSuperLogging(gVmafdGlobals.pLogger);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "Srv_RpcVmAfdSuperLogDisable failed. Error(%u)", dwError);
    goto cleanup;
}

UINT32
Srv_RpcVmAfdIsSuperLogEnabled(
    handle_t    hBinding,
    UINT32*     pEnabled
    )
{
    DWORD  dwError = 0;

    /* ncalrpc is needed for self-ping operation at startup */

    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_NCALRPC
                     | VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdIsSuperLoggingEnabled(gVmafdGlobals.pLogger);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ERROR, "Srv_RpcVmDirIsSuperLogEnabled failed (%u)", dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmAfdClearSuperLog(
    handle_t    hBinding
    )
{
    DWORD  dwError = 0;

    /* ncalrpc is needed for self-ping operation at startup */
    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_NCALRPC
                     | VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdFlushSuperLogging(gVmafdGlobals.pLogger);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ERROR, "Srv_RpcVmAfdClearSuperLog failed (%u)", dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmAfdSuperLogSetSize(
    handle_t    hBinding,
    UINT32      iSize
    )
{
    DWORD  dwError = 0;

    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_NCALRPC
                     | VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSetSuperLoggingSize(gVmafdGlobals.pLogger, iSize);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ERROR, "Srv_RpcVmAfdSuperLogSetSize failed (%u)", dwError );
    goto cleanup;
}

UINT32
Srv_RpcVmAfdSuperLogGetSize(
    handle_t    hBinding,
    UINT32      *piSize
    )
{
    DWORD  dwError = 0;

    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_NCALRPC
                     | VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetSuperLoggingSize(gVmafdGlobals.pLogger, piSize);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ERROR, "Srv_RpcVmAfdSuperLogGetSize failed (%u)", dwError );
    goto cleanup;
}

DWORD
_VmAfdSuperLoggingInitializeCookie(
    vmafd_superlog_cookie_t *pEnumerationCookie
    )
{
    DWORD dwError = 0;

    if (*pEnumerationCookie == NULL)
    {
        dwError = VmAfdAllocateMemory(sizeof(ULONG64), (PVOID*)pEnumerationCookie);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:
    return dwError;
}

UINT32
Srv_RpcVmAfdSuperLogGetEntries(
    handle_t    hBinding,
    vmafd_superlog_cookie_t *pEnumerationCookie,
    UINT32 dwCountRequested,
    PVMAFD_SUPERLOG_ENTRY_ARRAY *ppRpcEntries
    )
{
    DWORD  dwError = 0;

    DWORD dwRpcFlags = VMAFD_RPC_FLAG_ALLOW_NCALRPC
                     | VMAFD_RPC_FLAG_ALLOW_TCPIP
                     | VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP;

    dwError = VmAfdRpcServerCheckAccess(hBinding, dwRpcFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _VmAfdSuperLoggingInitializeCookie(pEnumerationCookie);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdCircularBufferGetSize(gVmafdGlobals.pLogger->pCircularBuffer, &dwCountRequested);

    dwError = VmAfdSuperLoggingGetEntries(gVmafdGlobals.pLogger, (UINT64 *)*pEnumerationCookie, dwCountRequested, ppRpcEntries);
    BAIL_ON_VMAFD_ERROR(dwError);


cleanup:
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ERROR, "RpcVmAfdSuperLogGetEntries failed. Error(%u)", dwError);

    goto cleanup;
}

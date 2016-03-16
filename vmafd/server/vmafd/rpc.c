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
BOOLEAN
VmAfdRpcCheckServerIsActive(
    VOID
    );

static
VOID
VmAfdRpcIfCallbackFn(
    rpc_if_handle_t InterfaceUuid,
    PVOID           Context,
    unsigned32*     status
    );

static
PVOID
VmAfdRpcListen(
    PVOID pData
    );

static
DWORD
VmAfdAdministratorAccessCheck(
    PCSTR pszUpn
    );

DWORD
VmAfdRpcServerAllocateMemory(
    size_t size,
    PVOID* ppMemory
    )
{
    DWORD dwError = 0;
    PVOID pMemory = NULL;

    if (size <= 0 || !ppMemory)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pMemory = rpc_ss_allocate(size);
    if (!pMemory)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    memset(pMemory,0, size);

    *ppMemory = pMemory;

cleanup:

    return dwError;

error:

    if (ppMemory)
    {
        *ppMemory = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdRpcServerAllocateStringA(
    PCSTR pszSource,
    PSTR* ppszTarget
    )
{
    DWORD dwError = 0;
    PSTR pszTarget = NULL;
    size_t length = 0;

    if (!pszSource)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    length = strlen(pszSource);

    dwError = VmAfdRpcServerAllocateMemory(length + 1, (PVOID*)&pszTarget);
    BAIL_ON_VMAFD_ERROR(dwError);

    memcpy(pszTarget, pszSource, length);

    pszTarget[length] = '\0';

    *ppszTarget = pszTarget;

cleanup:

    return dwError;

error:

    *ppszTarget = NULL;

    if (pszTarget)
    {
        VmAfdRpcServerFreeMemory(pszTarget);
    }

    goto cleanup;
}

DWORD
VmAfdRpcServerAllocateStringW(
    PCWSTR pwszSource,
    PWSTR* ppwszTarget
    )
{
    DWORD  dwError = 0;
    size_t len = 0;
    PWSTR  pwszTarget = NULL;

    if (!pwszSource || !ppwszTarget)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetStringLengthW(pwszSource, &len);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerAllocateMemory(
                    sizeof(WCHAR) * (len + 1),
                    (PVOID*)&pwszTarget);
    BAIL_ON_VMAFD_ERROR(dwError);

    memcpy((PBYTE)pwszTarget, (PBYTE)pwszSource, sizeof(WCHAR) * len);

    *ppwszTarget = pwszTarget;

cleanup:

    return dwError;

error:

    if (ppwszTarget)
    {
        *ppwszTarget = NULL;
    }

    if (pwszTarget)
    {
        VmAfdRpcServerFreeMemory(pwszTarget);
    }

    goto cleanup;
}

DWORD
VmAfdRpcServerAllocateStringArrayW(
    DWORD dwCount,
    PCWSTR *pwszSrc,
    PWSTR ** ppwszDst
    )
{
    DWORD dwError = 0;
    PWSTR *pwszStringArray = NULL;
    DWORD dwIndex = 0;

    if (!pwszSrc || !ppwszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdRpcServerAllocateMemory(
                  sizeof(PWSTR) * dwCount,
                  (PVOID *)&pwszStringArray
                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    for (;dwIndex<dwCount; dwIndex++)
    {
        dwError = VmAfdRpcServerAllocateStringW(
                        pwszSrc[dwIndex],
                        &(pwszStringArray[dwIndex])
                        );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppwszDst = pwszStringArray;
cleanup:
    return dwError;

error:
    if (ppwszDst)
    {
        *ppwszDst = NULL;
    }
    if (pwszStringArray)
    {
        VmAfdRpcServerFreeStringArrayW(
                  pwszStringArray, dwCount);
    }

    goto cleanup;
}

DWORD
CdcRpcServerAllocateDCInfoW(
    PCDC_DC_INFO_W  pCdcInfo,
    PCDC_DC_INFO_W  *ppRpcCdcInfo
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W  pRpcCdcInfo = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pCdcInfo, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(ppRpcCdcInfo, dwError);

    dwError = VmAfdRpcServerAllocateMemory(
                  sizeof(CDC_DC_INFO_W),
                  (PVOID *)&pRpcCdcInfo
                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (pCdcInfo->pszDCAddress)
    {
        dwError = VmAfdRpcServerAllocateStringW(
                        pCdcInfo->pszDCAddress,
                        &pRpcCdcInfo->pszDCAddress
                        );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pCdcInfo->pszDCName)
    {
        dwError = VmAfdRpcServerAllocateStringW(
                        pCdcInfo->pszDCName,
                        &pRpcCdcInfo->pszDCName
                        );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pCdcInfo->pszDcSiteName)
    {
        dwError = VmAfdRpcServerAllocateStringW(
                        pCdcInfo->pszDcSiteName,
                        &pRpcCdcInfo->pszDcSiteName
                        );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pCdcInfo->pszDomainName)
    {
        dwError = VmAfdRpcServerAllocateStringW(
                        pCdcInfo->pszDomainName,
                        &pRpcCdcInfo->pszDomainName
                        );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppRpcCdcInfo = pRpcCdcInfo;

cleanup:
    return dwError;

error:
    if (ppRpcCdcInfo)
    {
        *ppRpcCdcInfo = NULL;
    }
    if (pRpcCdcInfo)
    {
        CdcRpcServerFreeDCInfoW(pRpcCdcInfo);
    }

    goto cleanup;
}


VOID
VmAfdRpcServerFreeStringArrayA(
    PSTR*  ppszStrArray,
    DWORD  dwCount
    )
{
    DWORD iStr = 0;

    for (; iStr < dwCount; iStr++)
    {
        VmAfdRpcServerFreeStringA(ppszStrArray[iStr]);
    }
    VmAfdRpcServerFreeMemory(ppszStrArray);
}

VOID
VmAfdRpcServerFreeStringArrayW(
    PWSTR * ppwszStrArray,
    DWORD dwCount
    )
{
    DWORD iStr = 0;
    if (ppwszStrArray)
    {
        for (; iStr < dwCount; iStr++)
        {
            VmAfdRpcServerFreeMemory(ppwszStrArray[iStr]);
        }
        VmAfdRpcServerFreeMemory(ppwszStrArray);
    }
}

DWORD
CdcRpcServerFreeDCInfoW(
    PCDC_DC_INFO_W  pCdcInfo
    )
{
    DWORD dwError = 0;
    BAIL_ON_VMAFD_INVALID_POINTER(pCdcInfo, dwError);

    VmAfdRpcServerFreeMemory(pCdcInfo->pszDCAddress);
    VmAfdRpcServerFreeMemory(pCdcInfo->pszDCName);
    VmAfdRpcServerFreeMemory(pCdcInfo->pszDcSiteName);
    VmAfdRpcServerFreeMemory(pCdcInfo->pszDomainName);
    VmAfdRpcServerFreeMemory(pCdcInfo);

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VmAfdRpcServerFreeStringA(
    PSTR pszStr
    )
{
    if (pszStr)
    {
        VmAfdRpcServerFreeMemory(pszStr);
    }
}

VOID
VmAfdRpcServerFreeMemory(
    PVOID pMemory
    )
{
    if (pMemory)
    {
        rpc_ss_free(pMemory);
    }
}

VOID
VmAfdRpcClientFreeMemory(
    PVOID pMemory
    )
{
    if (pMemory)
    {
        error_status_t rpcStatus = rpc_s_ok;
        rpc_sm_client_free(pMemory, &rpcStatus);
    }
}

VOID
VmAfdRpcClientFreeStringArrayA(
    PSTR*  ppszStrArray,
    DWORD  dwCount
    )
{
    DWORD iStr = 0;

    for (; iStr < dwCount; iStr++)
    {
        VmAfdRpcClientFreeStringA(ppszStrArray[iStr]);
    }
    VmAfdRpcClientFreeMemory(ppszStrArray);
}

VOID
VmAfdRpcClientFreeStringA(
    PSTR pszStr
    )
{
    if (pszStr)
    {
        VmAfdRpcClientFreeMemory(pszStr);
    }
}

DWORD
VmAfdRpcServerStartListen(
    VOID
    )
{
    DWORD dwError = 0;
    int   status = 0;

    status = dcethread_create(
                            &gVmafdGlobals.pRPCServerThread,
                            NULL,
                            &VmAfdRpcListen,
                            NULL);

#ifndef _WIN32
    dwError = LwErrnoToWin32Error(status);
#else
    dwError = status;
#endif

    BAIL_ON_VMAFD_ERROR(dwError);

    while (!VmAfdRpcCheckServerIsActive())
    {
        // Wait for RPC Server to come up.
        VmAfdSleep(1000);
    }

error:

    return dwError;
}

DWORD
VmAfdRpcServerStopListen(
    VOID
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
        rpc_mgmt_stop_server_listening(NULL, (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (!dwError)
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
        }
        if(!dwError)
        {
            dwError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdRpcServerRegisterIf(
    rpc_if_handle_t pInterfaceSpec
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
       rpc_server_register_if_ex(
               pInterfaceSpec,
               NULL,
               NULL,
               rpc_if_allow_secure_only,
               rpc_c_listen_max_calls_default,
               VmAfdRpcIfCallbackFn,
               &dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if ( dwError == rpc_s_ok )
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
            if (!dwError)
            {
                dwError = RPC_S_INTERNAL_ERROR;
            }
        }
    }
    DCETHREAD_ENDTRY;

    return dwError;
}

DWORD
VmAfdRpcServerUseProtSeq(
    PCSTR pszProtSeq
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
        rpc_server_use_protseq(
                (unsigned char*) pszProtSeq,
                rpc_c_protseq_max_calls_default,
                (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if ( dwError == rpc_s_ok )
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
            if (!dwError)
            {
                dwError = RPC_S_INTERNAL_ERROR;
            }
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAfdRpcServerUseProtSeqEp(
    PCSTR pszProtSeq,
    PCSTR pszEndpoint
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
        rpc_server_use_protseq_ep(
                (unsigned char*) pszProtSeq,
                rpc_c_protseq_max_calls_default,
                (unsigned char*) pszEndpoint,
                (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if ( dwError == rpc_s_ok )
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
            if (!dwError)
            {
                dwError = RPC_S_INTERNAL_ERROR;
            }
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAfdRpcServerInqBindings(
    rpc_binding_vector_p_t* ppServerBindings
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
        rpc_server_inq_bindings(ppServerBindings, (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if ( dwError == rpc_s_ok )
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
            if (!dwError)
            {
                dwError = RPC_S_INTERNAL_ERROR;
            }
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAFD_ERROR(dwError);

error:

     return dwError;
}

DWORD
VmAfdRpcEpRegister(
    rpc_binding_vector_p_t pServerBinding,
    rpc_if_handle_t        pInterfaceSpec,
    PCSTR                  pszAnnotation
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
       rpc_ep_register(
                pInterfaceSpec,
                pServerBinding,
                NULL,
                (idl_char*)pszAnnotation,
                (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (dwError == rpc_s_ok )
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
            if (!dwError)
            {
                dwError = RPC_S_INTERNAL_ERROR;
            }
        }
    }
    DCETHREAD_ENDTRY;

    return dwError;
}

DWORD
VmAfdRpcServerRegisterAuthInfo(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszHostName = NULL;
    PSTR pszCanonicalHostName = NULL;
    PSTR pszServerPrincipalName = NULL;

    dwError = VmAfdGetHostName(&pszHostName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetCanonicalHostName(
                      pszHostName,
                      &pszCanonicalHostName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                      &pszServerPrincipalName,
                      "host/%s",
                      pszCanonicalHostName);
    BAIL_ON_VMAFD_ERROR(dwError);

    DCETHREAD_TRY
    {
       rpc_server_register_auth_info(
                  pszServerPrincipalName, // Server principal name
                  rpc_c_authn_gss_negotiate, // Authentication service
                  NULL, // Use default key function
                  NULL,
                  &dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (dwError == rpc_s_ok )
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
            if (dwError == rpc_s_ok)
            {
                dwError = RPC_S_INTERNAL_ERROR;
            }
        }
    }
    DCETHREAD_ENDTRY;

error:

    VMAFD_SAFE_FREE_STRINGA(pszHostName);
    VMAFD_SAFE_FREE_STRINGA(pszCanonicalHostName);
    VMAFD_SAFE_FREE_STRINGA(pszServerPrincipalName);

    return dwError;
}

DWORD
VmAfdRpcBindingInqAuthClient(
    rpc_binding_handle_t hClientBinding,
    rpc_authz_handle_t*  pPrivs,
    PSTR*                ppServerPrincName,
    DWORD*               pAuthnLevel,
    DWORD*               pAuthnSvc,
    DWORD*               pAuthzSvc
    )
{
    error_status_t rpcStatus = rpc_s_ok;

    rpc_binding_inq_auth_client(
            hClientBinding,
            pPrivs, // The data referenced by this parameter is read-only,
                    // and therefore should not be modified/freed.
            (unsigned_char_p_t*)ppServerPrincName,
            pAuthnLevel,
            pAuthnSvc,
            pAuthzSvc,
            &rpcStatus);

    BAIL_ON_VMAFD_ERROR(rpcStatus);

error:

    return rpcStatus;
}

DWORD
VmAfdRpcStringBindingCompose(
    PCSTR pszProtSeq,
    PCSTR pszNetworkAddr,
    PCSTR pszEndPoint,
    PSTR* ppszStringBinding
    )
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_string_binding_compose(
        NULL,
        (PBYTE)pszProtSeq,
        (PBYTE)pszNetworkAddr,
        (PBYTE)pszEndPoint,
        NULL,
        (PBYTE*)ppszStringBinding,
        &dwError);

    return dwError;
}

DWORD
VmAfdRpcBindingFromStringBinding(
    PCSTR pszStringBinding,
    handle_t* pBinding
    )
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_binding_from_string_binding(
        (PBYTE)pszStringBinding,
        pBinding,
        &dwError);

    return dwError;
}

DWORD
VmAfdRpcStringFree(
    PSTR* ppszString
    )
{
    DWORD dwError = 0;

    if (ppszString)
    {
        rpc_string_free((PBYTE*)ppszString, &dwError);
    }

    return dwError;
}

DWORD
VmAfdRpcBindingFree(
    handle_t* pBinding
    )
{
    DWORD dwError = 0;

    if (pBinding)
    {
        rpc_binding_free(pBinding, &dwError);
    }

    return dwError;
}

DWORD
VmAfdRpcBindingVectorFree(
    rpc_binding_vector_p_t* ppServerBindings
    )
{
    error_status_t rpcStatus = rpc_s_ok;

    if ( (ppServerBindings != NULL) && ((*ppServerBindings) != NULL) )
    {
        rpc_binding_vector_free(ppServerBindings, &rpcStatus);
    }
    BAIL_ON_VMAFD_ERROR(rpcStatus);

error:

    return rpcStatus;
}

DWORD
VmAfdRpcGetErrorCode(
    dcethread_exc* pException
    )
{
    DWORD dwError = 0;

    dwError = dcethread_exc_getstatus(pException);

    return dwError;
}

static
BOOLEAN
VmAfdRpcCheckServerIsActive(
    VOID
    )
{
    volatile DWORD dwError = 0;
    BOOLEAN bIsActive = FALSE;

    DCETHREAD_TRY
    {
        bIsActive = rpc_mgmt_is_server_listening(NULL, (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (!dwError)
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
        }
        if (!dwError)
        {
            dwError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return bIsActive;

error:

    bIsActive = FALSE;

    goto cleanup;
}

static
VOID
VmAfdRpcIfCallbackFn(
    rpc_if_handle_t InterfaceUuid,
    PVOID           Context,
    unsigned32*     status
    )
{
    *status = VmAfdRpcAuthCallback(Context);
}

static
PVOID
VmAfdRpcListen(
    PVOID pData
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
        rpc_server_listen(
            rpc_c_listen_max_calls_default, (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (!dwError)
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
        }
        if(!dwError)
        {
            dwError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
#ifndef _WIN32
    raise(SIGTERM); // indicate that process must terminate
#endif
    return NULL;

error:

    goto cleanup;
}

DWORD
VmAfdRpcServerCheckAccess(
    rpc_binding_handle_t hBinding,
    DWORD dwRpcFlags)
{
    DWORD dwError = 0;
    rpc_authz_cred_handle_t hPriv = { 0 };
    unsigned char *authPrinc = NULL;
    DWORD dwProtectLevel = 0;
    ULONG rpc_status = rpc_s_ok;
    PSTR pszRpcHandle = NULL;

    rpc_binding_to_string_binding(
        hBinding,
        (unsigned_char_p_t *) &pszRpcHandle,
        &rpc_status);
    if (rpc_status != rpc_s_ok)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "VmAfdRpcServerCheckAccess: rpc_binding_to_string_binding failed");
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    VmAfdLog(VMAFD_DEBUG_ANY,
             "VmAfdRpcServerCheckAccess: request from %s", pszRpcHandle);

    if (strncmp(pszRpcHandle, "ncalrpc:", 8) == 0)
    {
        if ( !(dwRpcFlags & VMAFD_RPC_FLAG_ALLOW_NCALRPC) )
        {
            dwError = ERROR_ACCESS_DENIED;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        if ( !(dwRpcFlags & VMAFD_RPC_FLAG_REQUIRE_AUTH_NCALRPC) )
        {
            goto cleanup;
        }
    }
    else
    {
        if ( !(dwRpcFlags & VMAFD_RPC_FLAG_ALLOW_TCPIP) )
        {
            dwError = ERROR_ACCESS_DENIED;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        if ( !(dwRpcFlags & VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP) )
        {
            goto cleanup;
        }
    }

    rpc_binding_inq_auth_caller(
        hBinding,
        &hPriv,
        &authPrinc,
        &dwProtectLevel,
        NULL, /* unsigned32 *authn_svc, */
        NULL, /* unsigned32 *authz_svc, */
        &rpc_status);
    if (rpc_status != rpc_s_ok)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "VmAfdRpcServerCheckAccess: rpc_binding_inq_auth_caller failed");
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    /* Deny if connection is not encrypted */
    if (dwProtectLevel < rpc_c_protect_level_pkt_privacy)
    {
        /* TBD - map error code */
        VmAfdLog(VMAFD_DEBUG_ANY,
                 "VmAfdRpcServerCheckAccess: protection level %lu", dwProtectLevel);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    /* Deny if no auth identity is provided.  */
    if (rpc_status == rpc_s_binding_has_no_auth || !authPrinc || !*authPrinc)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "VmAfdRpcServerCheckAccess: failed: %s",
                 authPrinc ? (PSTR)authPrinc : "NULL");

        /* TBD - map error code */
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    VmAfdLog(VMAFD_DEBUG_ANY,
             "VmAfdRpcServerCheckAccess: Authenticated principal %s",
             (PSTR)authPrinc);

    if ( dwRpcFlags & VMAFD_RPC_FLAG_REQUIRE_AUTHZ )
    {
        dwError = VmAfdAdministratorAccessCheck(authPrinc);
        BAIL_ON_VMAFD_ERROR(dwError);

        VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdRPCCheckAccess: Authorized user %s", authPrinc);
    }

cleanup:
    if (authPrinc)
    {
        rpc_string_free((unsigned_char_p_t *)&authPrinc, &rpc_status);
    }
    if (pszRpcHandle)
    {
        rpc_string_free((unsigned_char_p_t *)&pszRpcHandle, &rpc_status);
    }
    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR,
             "VmAfdRpcServerCheckAccess failed (%lu)", dwError);

    goto cleanup;
}

static
DWORD
VmAfdAdministratorAccessCheck(
    PCSTR pszUpn
    )
{
    DWORD dwError = 0;
    PSTR pszDomainName = NULL;
    PSTR pszDomainNameDN = NULL;
    PSTR pszGroupName = NULL;
    PSTR *ppszMemberships = NULL;
    DWORD dwMemberships = 0;

    if (IsNullOrEmptyString(pszUpn))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvDirGetMemberships(
                      pszUpn,
                      &ppszMemberships,
                      &dwMemberships);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainNameA(&pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdFQDNToDN(
                     pszDomainName,
                     &pszDomainNameDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                     &pszGroupName,
                     "%s,%s",
                     "cn=Administrators,cn=Builtin",
                     pszDomainNameDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!VmAfSrvDirIsMember(
                    ppszMemberships,
                    dwMemberships,
                    pszGroupName))
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (ppszMemberships)
    {
        VmAfSrvDirFreeMemberships(ppszMemberships, dwMemberships);
        ppszMemberships = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pszDomainNameDN);
    VMAFD_SAFE_FREE_MEMORY(pszGroupName);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "VmDirAdministratorAccessCheck failed (%u)", dwError);
    goto cleanup;
}

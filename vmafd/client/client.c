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
VOID
VmAfdFreeServer(
    PVMAFD_SERVER pServer
    );

static
PVOID
VmAfdHeartbeatWorker(
    PVOID pThreadArgs
    );

DWORD
VmAfdOpenServerA(
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PVMAFD_SERVER *ppServer)
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppServer, dwError);

    if ((pszServerName && !pszPassword) ||
        (!pszServerName && pszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdOpenServerWithTimeoutA(
                                pszServerName,
                                pszUserName,
                                pszPassword,
                                0,
                                &pServer
                                );
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppServer = pServer;

cleanup:

    return dwError;

error:

    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }

    goto cleanup;
}

DWORD
VmAfdOpenServerW(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PVMAFD_SERVER *ppServer)
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppServer, dwError);

    if ((pwszServerName && !pwszPassword) ||
        (!pwszServerName && pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdOpenServerWithTimeoutW(
                  pwszServerName,
                  pwszUserName,
                  pwszPassword,
                  0,
                  &pServer);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppServer = pServer;

cleanup:

    return dwError;
error:

    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }
    goto cleanup;
}

DWORD
VmAfdOpenServerWithTimeoutA(
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    DWORD dwTimeout,
    PVMAFD_SERVER *ppServer)
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;
    BOOLEAN bIsLocalHost = TRUE;

    BAIL_ON_VMAFD_INVALID_POINTER(ppServer, dwError);

    if ((pszServerName && !pszPassword) ||
        (!pszServerName && pszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                  sizeof(VMAFD_SERVER),
                  (PVOID *)&pServer);
    BAIL_ON_VMAFD_ERROR(dwError);

    bIsLocalHost = VmAfdIsLocalHost(pszServerName);
    if (!bIsLocalHost && pszUserName && pszPassword)
    {
        dwError = VmAfdCreateBindingHandleAuthA(
                      pszServerName,
                      NULL,
                      pszUserName,
                      NULL,
                      pszPassword,
                      &pServer->hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringA(
                      pszServerName,
                      &pServer->pszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringA(
                      pszUserName,
                      &pServer->pszUserName);
        BAIL_ON_VMAFD_ERROR(dwError);

        rpc_mgmt_set_com_timeout(pServer->hBinding, dwTimeout, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pServer->refCount = 1;

    *ppServer = pServer;

cleanup:

    return dwError;

error:

    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }

    goto cleanup;

}

DWORD
VmAfdOpenServerWithTimeoutW(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    DWORD  dwTimeout,
    PVMAFD_SERVER *ppServer)

{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;
    PSTR pszServerName = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppServer, dwError);

    if ((pwszServerName && !pwszPassword) ||
        (!pwszServerName && pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pwszServerName)
    {
        dwError = VmAfdAllocateStringAFromW(pwszServerName, &pszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pwszUserName)
    {
        dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pwszPassword)
    {
        dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdOpenServerWithTimeoutA(
                  pszServerName,
                  pszUserName,
                  pszPassword,
                  dwTimeout,
                  &pServer);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppServer = pServer;

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszServerName);
    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);

    return dwError;

error:

    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }

    goto cleanup;

}

VOID
VmAfdCloseServer(
    PVMAFD_SERVER pServer)
{
    if (pServer)
    {
        VmAfdReleaseServer(pServer);
    }
}

DWORD
VmAfdRpcGetErrorCode(
    dcethread_exc* pDceException
)
{
    DWORD dwError = 0;

    dwError = dcethread_exc_getstatus (pDceException);

    return dwError;
}

DWORD
VmAfdGetStatusA(
    PCSTR         pszServerName,  /* IN     OPTIONAL */
    PVMAFD_STATUS pStatus         /* IN OUT          */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    VMAFD_STATUS status = VMAFD_STATUS_UNKNOWN;

    BAIL_ON_VMAFD_INVALID_POINTER(pStatus, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetStatusW(pwszServerName, &status);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pStatus = status;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    if (pStatus)
    {
        *pStatus = VMAFD_STATUS_UNKNOWN;
    }

    goto cleanup;
}

DWORD
VmAfdGetStatusW(
    PCWSTR        pwszServerName, /* IN     */
    PVMAFD_STATUS pStatus         /* IN OUT */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR   pwszServerEndpoint = NULL;
    VMAFD_STATUS status = VMAFD_STATUS_UNKNOWN;

    BAIL_ON_VMAFD_INVALID_POINTER(pStatus, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetStatus(
                                      &status
                                     );
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                    pwszServerName,
                    pwszServerEndpoint,
                    &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetStatus(hBinding, &status);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    *pStatus = status;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;

error:

    if (pStatus)
    {
        *pStatus = VMAFD_STATUS_UNKNOWN;
    }

    goto cleanup;
}

DWORD
VmAfdGetStatusRPCA(
    PCSTR         pszServerName,  /* IN     OPTIONAL */
    PVMAFD_STATUS pStatus         /* IN OUT          */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    VMAFD_STATUS status = VMAFD_STATUS_UNKNOWN;

    BAIL_ON_VMAFD_INVALID_POINTER(pStatus, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetStatusRPCW(pwszServerName, &status);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pStatus = status;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    if (pStatus)
    {
        *pStatus = VMAFD_STATUS_UNKNOWN;
    }

    goto cleanup;
}

DWORD
VmAfdGetStatusRPCW(
    PCWSTR        pwszServerName, /* IN     */
    PVMAFD_STATUS pStatus         /* IN OUT */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR   pwszServerEndpoint = NULL;
    VMAFD_STATUS status = VMAFD_STATUS_UNKNOWN;

    BAIL_ON_VMAFD_INVALID_POINTER(pStatus, dwError);

    dwError = VmAfdCreateBindingHandleW(
                    pwszServerName,
                    pwszServerEndpoint,
                    &hBinding);
    BAIL_ON_VMAFD_ERROR(dwError);

    DCETHREAD_TRY
    {
        dwError = VmAfdRpcGetStatus(hBinding, &status);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAFD_ERROR(dwError);

    *pStatus = status;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;

error:

    if (pStatus)
    {
        *pStatus = VMAFD_STATUS_UNKNOWN;
    }

    goto cleanup;
}

DWORD
VmAfdGetDomainNameA(
    PCSTR pszServerName,    /* IN     OPTIONAL */
    PSTR* ppszDomain        /*    OUT          */
)
{
    DWORD  dwError = 0;
    PWSTR  pwszDomain = NULL;
    PSTR   pszDomain = NULL;
    PWSTR  pwszServerName = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszDomain, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetDomainNameW(pwszServerName, &pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDomain, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszDomain = pszDomain;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);

    return dwError;

error:

    if (ppszDomain)
    {
        *ppszDomain = NULL;
    }

    // VMAFD_SAFE_FREE_MEMORY(pszDomain);

    goto cleanup;
}

DWORD
VmAfdGetDomainNameW(
    PCWSTR pwszServerName,  /* IN     OPTIONAL */
    PWSTR* ppwszDomain      /*    OUT          */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;
    PWSTR  pwszDomain = NULL;
    PWSTR  pwszDomainRet = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszDomain, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetDomainName(&pwszDomainRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetDomainName(hBinding, &pwszDomain);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringW(pwszDomain, &pwszDomainRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszDomain = pwszDomainRet;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    VMAFD_RPC_SAFE_FREE_MEMORY(pwszDomain);

    return dwError;

error:

    if (ppwszDomain)
    {
        *ppwszDomain = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdSetDomainNameA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    PCSTR pszDomain      /* IN              */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszDomain     = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pszDomain, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszDomain, &pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSetDomainNameW(pwszServerName, pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdSetDomainNameW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PCWSTR pwszDomain      /* IN              */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszDomain, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalSetDomainName(pwszDomain);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcSetDomainName(hBinding, (PWSTR)pwszDomain);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdGetDomainStateA(
    PCSTR pszServerName,              /* IN     OPTIONAL */
    VMAFD_DOMAIN_STATE* pDomainState  /*    OUT          */
)
{
    DWORD  dwError = 0;
    VMAFD_DOMAIN_STATE state = VMAFD_DOMAIN_STATE_NONE;
    PWSTR  pwszServerName = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pDomainState, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetDomainStateW(pwszServerName, &state);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pDomainState = state;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;
}

DWORD
VmAfdGetDomainStateW(
    PCWSTR pwszServerName,            /* IN     OPTIONAL */
    VMAFD_DOMAIN_STATE* pDomainState  /*    OUT          */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;
    VMAFD_DOMAIN_STATE state = VMAFD_DOMAIN_STATE_NONE;

    BAIL_ON_VMAFD_INVALID_POINTER(pDomainState, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetDomainState(&state);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetDomainState(hBinding, &state);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *pDomainState = state;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdGetDomainStateW: failed, Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdGetLDUA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    PSTR* ppszLDU        /*    OUT          */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszLDU = NULL;
    PSTR  pszLDU = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszLDU, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetLDUW(pwszServerName, &pwszLDU);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszLDU, &pszLDU);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszLDU = pszLDU;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszLDU);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    if (ppszLDU)
    {
        *ppszLDU = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdGetLDUW(
    PCWSTR pwszServerName,  /* IN     OPTIONAL */
    PWSTR* ppwszLDU         /*    OUT          */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;
    PWSTR  pwszLDU = NULL;
    PWSTR  pwszLDURet = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszLDU, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetLDU(&pwszLDURet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetLDU(hBinding, &pwszLDU);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringW(pwszLDU, &pwszLDURet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszLDU = pwszLDURet;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    VMAFD_RPC_SAFE_FREE_MEMORY(pwszLDU);

    return dwError;

error:

    if (ppwszLDU)
    {
        *ppwszLDU = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdSetLDUA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    PCSTR pszLDU         /*    OUT          */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszLDU     = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pszLDU, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszLDU, &pwszLDU);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSetLDUW(pwszServerName, pwszLDU);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszLDU);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdSetLDUW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PCWSTR pwszLDU         /* IN              */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszLDU, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalSetLDU(pwszLDU);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcSetLDU(hBinding, (PWSTR)pwszLDU);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdGetRHTTPProxyPortA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    PDWORD pdwPort       /* IN OUT          */
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    DWORD dwPort = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pdwPort, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetRHTTPProxyPortW(pwszServerName, &dwPort);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pdwPort = dwPort;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    if (pdwPort)
    {
        *pdwPort = 0;
    }

    goto cleanup;
}

DWORD
VmAfdGetRHTTPProxyPortW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PDWORD pdwPort         /* IN OUT          */
    )
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;
    DWORD dwPort = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pdwPort, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetRHTTPProxyPort(&dwPort);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetRHTTPProxyPort(hBinding, &dwPort);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *pdwPort = dwPort;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;

error:

    if (pdwPort)
    {
        *pdwPort = 0;
    }

    goto cleanup;
}

DWORD
VmAfdSetRHTTPProxyPortA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    DWORD dwPort         /* IN              */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdSetRHTTPProxyPortW(pwszServerName, dwPort);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdSetRHTTPProxyPortW(
    PCWSTR pwszServerName,   /* IN     OPTIONAL */
    DWORD dwPort             /* IN              */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalSetRHTTPProxyPort(dwPort);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcSetRHTTPProxyPort(hBinding, dwPort);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdSetDCPortA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    DWORD dwPort         /* IN              */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdSetDCPortW(pwszServerName, dwPort);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdSetDCPortW(
    PCWSTR pwszServerName,   /* IN     OPTIONAL */
    DWORD dwPort             /* IN              */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalSetDCPort(dwPort);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcSetDCPort(hBinding, dwPort);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdGetCMLocationA(
    PCSTR pszServerName,    /* IN     OPTIONAL */
    PSTR* ppszCMLocation    /*    OUT          */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszCMLocation = NULL;
    PSTR  pszCMLocation = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszCMLocation, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetCMLocationW(pwszServerName, &pwszCMLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszCMLocation, &pszCMLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszCMLocation = pszCMLocation;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszCMLocation);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    if (ppszCMLocation)
    {
        *ppszCMLocation = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdGetCMLocationW(
    PCWSTR pwszServerName,  /* IN     OPTIONAL */
    PWSTR* ppwszCMLocation  /*    OUT          */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;
    PWSTR  pwszCMLocation = NULL;
    PWSTR  pwszCMLocationRet = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszCMLocation, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetCMLocation(&pwszCMLocationRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetCMLocation(hBinding, &pwszCMLocation);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringW(pwszCMLocation, &pwszCMLocationRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszCMLocation = pwszCMLocationRet;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    VMAFD_RPC_SAFE_FREE_MEMORY(pwszCMLocation);

    return dwError;

error:

    if (ppwszCMLocation)
    {
        *ppwszCMLocation = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdGetLSLocationA(
    PCSTR pszServerName,    /* IN     OPTIONAL */
    PSTR* ppszLSLocation    /*    OUT          */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszLSLocation = NULL;
    PSTR  pszLSLocation = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszLSLocation, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetLSLocationW(pwszServerName, &pwszLSLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszLSLocation, &pszLSLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszLSLocation = pszLSLocation;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszLSLocation);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    if (ppszLSLocation)
    {
        *ppszLSLocation = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdGetLSLocationW(
    PCWSTR pwszServerName,  /* IN     OPTIONAL */
    PWSTR* ppwszLSLocation  /*    OUT          */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;
    PWSTR  pwszLSLocation = NULL;
    PWSTR  pwszLSLocationRet = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszLSLocation, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetLSLocation(&pwszLSLocationRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetLSLocation(hBinding, &pwszLSLocation);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringW(pwszLSLocation, &pwszLSLocationRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszLSLocation = pwszLSLocationRet;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    VMAFD_RPC_SAFE_FREE_MEMORY(pwszLSLocation);

    return dwError;

error:

    if (ppwszLSLocation)
    {
        *ppwszLSLocation = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdGetDCNameA(
    PCSTR pszServerName,    /* IN     OPTIONAL */
    PSTR* ppszDCName        /*    OUT          */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszDCName = NULL;
    PSTR  pszDCName = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszDCName, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetDCNameW(pwszServerName, &pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDCName, &pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszDCName = pszDCName;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    if (ppszDCName)
    {
        *ppszDCName = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY (pszDCName);

    goto cleanup;
}

DWORD
VmAfdGetDCNameW(
    PCWSTR pwszServerName,  /* IN     OPTIONAL */
    PWSTR* ppwszDCName      /*    OUT          */
)
{
    DWORD dwError = 0;
    PWSTR  pwszDCNameRet = NULL;
    PCDC_DC_INFO_W pDomainControllerInfo = NULL;
    PVMAFD_SERVER pServer = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszDCName, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdOpenServerW(NULL, NULL, NULL, &pServer);
    }
    else
    {
        dwError = ERROR_NOT_SUPPORTED;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcGetDCNameW(
                          pServer,
                          NULL,
                          NULL,
                          NULL,
                          0,
                          &pDomainControllerInfo
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringW(
                              pDomainControllerInfo->pszDCName,
                              &pwszDCNameRet
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszDCName = pwszDCNameRet;

cleanup:

    if (pDomainControllerInfo)
    {
        VmAfdFreeDomainControllerInfoW(pDomainControllerInfo);
    }

    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }

    return dwError;

error:

    if (ppwszDCName)
    {
        *ppwszDCName = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pwszDCNameRet);

    goto cleanup;
}

DWORD
VmAfdGetDCNameExA(
    PCSTR pszServerName,    /* IN     OPTIONAL */
    PSTR* ppszDCName        /*    OUT          */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszDCName = NULL;
    PSTR  pszDCName = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszDCName, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetDCNameW(pwszServerName, &pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDCName, &pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszDCName = pszDCName;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    if (ppszDCName)
    {
        *ppszDCName = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszDCName);

    goto cleanup;
}

DWORD
VmAfdGetDCNameExW(
    PCWSTR pwszServerName,  /* IN     OPTIONAL */
    PWSTR* ppwszDCName      /*    OUT          */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;
    PWSTR  pwszDCName = NULL;
    PWSTR  pwszDCNameRet = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszDCName, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetDCName(&pwszDCNameRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetDCName(hBinding, &pwszDCName);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringW(pwszDCName, &pwszDCNameRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszDCName = pwszDCNameRet;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    VMAFD_RPC_SAFE_FREE_MEMORY(pwszDCName);

    return dwError;

error:

    if (ppwszDCName)
    {
        *ppwszDCName = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdSetDCNameA(
    PCSTR pszServerName,    /* IN      OPTIONAL */
    PCSTR pszDCName         /* IN               */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszDCName = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pszDCName, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszDCName, &pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSetDCNameW(pwszServerName, pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdSetDCNameW(
    PCWSTR pwszServerName,  /* IN      OPTIONAL */
    PCWSTR pwszDCName       /* IN               */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszDCName, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalSetDCName(pwszDCName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcSetDCName(hBinding, (PWSTR)pwszDCName);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdGetMachineAccountInfoA(
    PCSTR pszServerName,    /* IN     OPTIONAL */
    PSTR* ppszAccount,      /*    OUT          */
    PSTR* ppszPassword      /*    OUT          */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszAccount  = NULL;
    PWSTR pwszPassword = NULL;
    PSTR  pszAccount  = NULL;
    PSTR  pszPassword = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszAccount, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(ppszPassword, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetMachineAccountInfoW(
                  pwszServerName,
                  &pwszAccount,
                  &pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszAccount, &pszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszAccount = pszAccount;
    *ppszPassword = pszPassword;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    if (ppszAccount)
    {
        *ppszAccount = NULL;
    }

    if (ppszPassword)
    {
        *ppszPassword = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszAccount);
    VMAFD_SAFE_FREE_MEMORY(pszPassword);

    goto cleanup;
}

DWORD
VmAfdGetMachineAccountInfoW(
    PCWSTR pwszServerName,  /* IN     OPTIONAL */
    PWSTR* ppwszAccount,    /*    OUT          */
    PWSTR* ppwszPassword    /*    OUT          */
)
{
    DWORD dwError = 0;
    PWSTR pwszAccount = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountRet = NULL;
    PWSTR pwszPasswordRet = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszAccount, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(ppwszPassword, dwError);

    dwError = VmAfdLocalGetMachineAccountInfo(
                    &pwszAccountRet,
                    &pwszPasswordRet);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszAccount = pwszAccountRet;
    *ppwszPassword = pwszPasswordRet;

cleanup:

    VMAFD_RPC_SAFE_FREE_MEMORY(pwszAccount);
    VMAFD_RPC_SAFE_FREE_MEMORY(pwszPassword);

    return dwError;

error:

    if (ppwszAccount)
    {
        *ppwszAccount = NULL;
    }
    if (ppwszPassword)
    {
        *ppwszPassword = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pwszAccountRet);
    VMAFD_SAFE_FREE_MEMORY(pwszPasswordRet);

    goto cleanup;
}

DWORD
VmAfdGetSiteGUIDA(
    PCSTR  pszServerName,  /* IN     OPTIONAL */
    PSTR*  ppszSiteGUID    /*    OUT          */
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszSiteGUID  = NULL;
    PSTR  pszSiteGUID = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszSiteGUID, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetSiteGUIDW(pwszServerName, &pwszSiteGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszSiteGUID, &pszSiteGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszSiteGUID = pszSiteGUID;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszSiteGUID);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    if (ppszSiteGUID)
    {
        *ppszSiteGUID = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszSiteGUID);

    goto cleanup;
}

DWORD
VmAfdGetSiteGUIDW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PWSTR* ppwszSiteGUID   /*    OUT          */
    )
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;
    PWSTR  pwszSiteGUID = NULL;
    PWSTR  pwszSiteGUIDRet = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszSiteGUID, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetSiteGUID(&pwszSiteGUIDRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetSiteGUID(hBinding, &pwszSiteGUID);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringW(pwszSiteGUID, &pwszSiteGUIDRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszSiteGUID = pwszSiteGUIDRet;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    VMAFD_RPC_SAFE_FREE_MEMORY(pwszSiteGUID);

    return dwError;

error:

    if (ppwszSiteGUID)
    {
        *ppwszSiteGUID = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pwszSiteGUIDRet);

    goto cleanup;
}

DWORD
VmAfdGetSiteNameA(
    PCSTR  pszServerName,  /* IN     OPTIONAL */
    PSTR*  ppszSiteName    /*    OUT          */
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszSiteName  = NULL;
    PSTR  pszSiteName = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszSiteName, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetSiteNameW(pwszServerName, &pwszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszSiteName, &pszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszSiteName = pszSiteName;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    if (ppszSiteName)
    {
        *ppszSiteName = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszSiteName);

    goto cleanup;
}

DWORD
VmAfdGetSiteNameW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PWSTR* ppwszSiteName   /*    OUT          */
    )
{
    DWORD dwError = 0;
    PWSTR  pwszSiteName = NULL;
    PWSTR  pwszSiteNameRet = NULL;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszSiteName, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetSiteName(&pwszSiteNameRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetSiteName(hBinding, &pwszSiteName);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringW(pwszSiteName, &pwszSiteNameRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszSiteName = pwszSiteNameRet;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    VMAFD_RPC_SAFE_FREE_MEMORY(pwszSiteName);
    return dwError;

error:

    if (ppwszSiteName)
    {
        *ppwszSiteName = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pwszSiteNameRet);

    goto cleanup;
}

DWORD
VmAfdGetSiteNameHA(
    PVMAFD_SERVER pServer,
    PSTR*  ppszSiteName
    )
{
    DWORD dwError = 0;
    PWSTR pwszSiteName  = NULL;
    PSTR  pszSiteName = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pServer, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(ppszSiteName, dwError);

    dwError = VmAfdGetSiteNameHW(pServer, &pwszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszSiteName, &pszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszSiteName = pszSiteName;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);

    return dwError;

error:

    if (ppszSiteName)
    {
        *ppszSiteName = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszSiteName);

    goto cleanup;
}

DWORD
VmAfdGetSiteNameHW(
    PVMAFD_SERVER pServer,
    PWSTR* ppwszSiteName
    )
{
    DWORD dwError = 0;
    PWSTR  pwszSiteName = NULL;
    PWSTR  pwszSiteNameRet = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pServer, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(ppwszSiteName, dwError);

    if (!pServer->hBinding)
    {
        dwError = VmAfdLocalGetSiteName(&pwszSiteNameRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetSiteName(pServer->hBinding, &pwszSiteName);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringW(pwszSiteName, &pwszSiteNameRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszSiteName = pwszSiteNameRet;

cleanup:

    VMAFD_RPC_SAFE_FREE_MEMORY(pwszSiteName);
    return dwError;

error:

    if (ppwszSiteName)
    {
        *ppwszSiteName = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pwszSiteNameRet);

    goto cleanup;
}

DWORD
VmAfdGetMachineIDA(
    PCSTR  pszServerName,  /* IN     OPTIONAL */
    PSTR*  ppszMachineID    /*    OUT          */
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszMachineID  = NULL;
    PSTR  pszMachineID = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszMachineID, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetMachineIDW(pwszServerName, &pwszMachineID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszMachineID, &pszMachineID);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszMachineID = pszMachineID;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszMachineID);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    if (ppszMachineID)
    {
        *ppszMachineID = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszMachineID);

    goto cleanup;
}

DWORD
VmAfdGetMachineIDW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PWSTR* ppwszMachineID   /*    OUT          */
    )
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;
    PWSTR  pwszMachineID = NULL;
    PWSTR  pwszMachineIDRet = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszMachineID, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetMachineID(&pwszMachineIDRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetMachineID(hBinding, &pwszMachineID);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringW(pwszMachineID, &pwszMachineIDRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszMachineID = pwszMachineIDRet;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    VMAFD_RPC_SAFE_FREE_MEMORY(pwszMachineID);

    return dwError;

error:

    if (ppwszMachineID)
    {
        *ppwszMachineID = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pwszMachineIDRet);

    goto cleanup;
}

DWORD
VmAfdSetMachineIDA(
    PCSTR  pszServerName,  /* IN     OPTIONAL */
    PCSTR  pszMachineID    /* IN              */
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszMachineID  = NULL;

    if (IsNullOrEmptyString(pszMachineID))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszMachineID, &pwszMachineID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSetMachineIDW(pwszServerName, pwszMachineID);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszMachineID);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdSetMachineIDW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PCWSTR pwszMachineID   /* IN              */
    )
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    if (IsNullOrEmptyString(pwszMachineID))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalSetMachineID(pwszMachineID);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcSetMachineID(hBinding, (PWSTR)pwszMachineID);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdPromoteVmDirA(
    PCSTR pszServerName,     /* IN              */
    PCSTR pszDomainName,     /* IN              */
    PCSTR pszUserName,       /* IN              */
    PCSTR pszPassword,       /* IN              */
    PCSTR pszSiteName,       /* IN     OPTIONAL */
    PCSTR pszPartnerHostName /* IN     OPTIONAL */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszSiteName = NULL;
    PWSTR pwszPartnerHostName = NULL;

    if (IsNullOrEmptyString(pszServerName) ||
        IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszUserName, &pwszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszDomainName)
    {
        dwError = VmAfdAllocateStringWFromA(pszDomainName, &pwszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszSiteName)
    {
        dwError = VmAfdAllocateStringWFromA(pszSiteName, &pwszSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszPartnerHostName)
    {
        dwError = VmAfdAllocateStringWFromA(
                      pszPartnerHostName,
                      &pwszPartnerHostName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdPromoteVmDirW(
                  pwszServerName,
                  pwszDomainName,
                  pwszUserName,
                  pwszPassword,
                  pwszSiteName,
                  pwszPartnerHostName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pwszUserName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);
    VMAFD_SAFE_FREE_MEMORY(pwszPartnerHostName);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdPromoteVmDirW(
    PCWSTR pwszServerName,     /* IN              */
    PCWSTR pwszDomainName,     /* IN     OPTIONAL */
    PCWSTR pwszUserName,       /* IN              */
    PCWSTR pwszPassword,       /* IN              */
    PCWSTR pwszSiteName,       /* IN     OPTIONAL */
    PCWSTR pwszPartnerHostName /* IN     OPTIONAL */
)
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pwszServerName) ||
            IsNullOrEmptyString(pwszUserName) ||
            IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdPromoteVmDirW: promoting vmdir instance");

    dwError = VmAfdLocalPromoteVmDir(
                      pwszServerName,
                      pwszDomainName,
                      pwszUserName,
                      pwszPassword,
                      pwszSiteName,
                      pwszPartnerHostName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdPromoteVmDirW failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdDemoteVmDirA(
    PCSTR pszServerName,    /* IN     OPTIONAL */
    PCSTR pszUserName,      /* IN              */
    PCSTR pszPassword       /* IN              */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;

    if ( IsNullOrEmptyString(pszUserName) ||
	 IsNullOrEmptyString(pszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszServerName)
    {
	dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
	BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszUserName, &pwszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdDemoteVmDirW(
                  pwszServerName,
                  pwszUserName,
                  pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszUserName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdDemoteVmDirW(
    PCWSTR pwszServerName,  /* IN     OPTIONAL */
    PCWSTR pwszUserName,    /* IN              */
    PCWSTR pwszPassword     /* IN              */
)
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pwszUserName) ||
	IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdLocalDemoteVmDir(
                      pwszServerName,
                      pwszUserName,
                      pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdDemoteVmDirW failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdJoinValidateDomainCredentialsA(
    PCSTR pszDomainName,     /* IN              */
    PCSTR pszUserName,       /* IN              */
    PCSTR pszPassword        /* IN              */
    )
{
    DWORD dwError = 0;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszDomainName = NULL;

    if (IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszDomainName, &pwszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszUserName, &pwszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdJoinValidateDomainCredentialsW(
                   pwszDomainName,
                   pwszUserName,
                   pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszUserName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszDomainName);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdJoinValidateDomainCredentialsW(
    PCWSTR pwszDomainName,     /* IN              */
    PCWSTR pwszUserName,       /* IN              */
    PCWSTR pwszPassword        /* IN              */
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pwszDomainName) ||
        IsNullOrEmptyString(pwszUserName) ||
        IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdLocalJoinValidateDomainCredentials(
                      pwszDomainName,
                      pwszUserName,
                      pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAfdJoinVmDirA(
    PCSTR pszServerName,      /* IN              */
    PCSTR pszUserName,        /* IN              */
    PCSTR pszPassword,        /* IN              */
    PCSTR pszMachineName,     /* IN              */
    PCSTR pszDomainName,      /* IN              */
    PCSTR pszOrgUnit          /* IN     OPTIONAL */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszMachineName = NULL;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszOrgUnit = NULL;

    if (IsNullOrEmptyString(pszServerName) ||
            IsNullOrEmptyString(pszUserName) ||
            IsNullOrEmptyString(pszPassword) ||
            IsNullOrEmptyString(pszMachineName) ||
            IsNullOrEmptyString(pszDomainName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszUserName, &pwszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszMachineName, &pwszMachineName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszDomainName, &pwszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszOrgUnit)
    {
        dwError = VmAfdAllocateStringWFromA(pszOrgUnit, &pwszOrgUnit);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdJoinVmDirW(
                  pwszServerName,
                  pwszUserName,
                  pwszPassword,
                  pwszMachineName,
                  pwszDomainName,
                  pwszOrgUnit);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszUserName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszMachineName);
    VMAFD_SAFE_FREE_MEMORY(pwszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pwszOrgUnit);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdJoinVmDirW(
    PCWSTR pwszServerName,  /* IN              */
    PCWSTR pwszUserName,    /* IN              */
    PCWSTR pwszPassword,    /* IN              */
    PCWSTR pwszMachineName, /* IN              */
    PCWSTR pwszDomainName,  /* IN              */
    PCWSTR pwszOrgUnit      /* IN     OPTIONAL */
)
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pwszServerName) ||
            IsNullOrEmptyString(pwszUserName) ||
            IsNullOrEmptyString(pwszPassword) ||
            IsNullOrEmptyString(pwszMachineName) ||
            IsNullOrEmptyString(pwszDomainName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdLocalJoinVmDir(
                      pwszServerName,
                      pwszUserName,
                      pwszPassword,
                      pwszMachineName,
                      pwszDomainName,
                      pwszOrgUnit);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdJoinVmDirW failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdJoinVmDir2A(
    PCSTR            pszDomainName,  /* IN              */
    PCSTR            pszUserName,    /* IN              */
    PCSTR            pszPassword,    /* IN              */
    PCSTR            pszMachineName, /* IN     OPTIONAL */
    PCSTR            pszOrgUnit,     /* IN     OPTIONAL */
    VMAFD_JOIN_FLAGS dwFlags         /* IN              */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszMachineName = NULL;
    PWSTR pwszOrgUnit = NULL;

    if (IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszDomainName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszUserName, &pwszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszMachineName)
    {
        dwError = VmAfdAllocateStringWFromA(pszMachineName, &pwszMachineName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszDomainName, &pwszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszOrgUnit)
    {
        dwError = VmAfdAllocateStringWFromA(pszOrgUnit, &pwszOrgUnit);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdJoinVmDir2W(
                  pwszDomainName,
                  pwszUserName,
                  pwszPassword,
                  pwszMachineName,
                  pwszOrgUnit,
                  dwFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pwszUserName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszMachineName);
    VMAFD_SAFE_FREE_MEMORY(pwszOrgUnit);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdJoinVmDir2A failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdJoinVmDir2W(
    PCWSTR           pwszDomainName,  /* IN            */
    PCWSTR           pwszUserName,    /* IN            */
    PCWSTR           pwszPassword,    /* IN            */
    PCWSTR           pwszMachineName, /* IN   OPTIONAL */
    PCWSTR           pwszOrgUnit,     /* IN   OPTIONAL */
    VMAFD_JOIN_FLAGS dwFlags          /* IN            */
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pwszUserName) ||
            IsNullOrEmptyString(pwszPassword) ||
            IsNullOrEmptyString(pwszDomainName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdLocalJoinVmDir2(
                      pwszDomainName,
                      pwszUserName,
                      pwszPassword,
                      pwszMachineName,
                      pwszOrgUnit,
                      dwFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdJoinVmDir2W failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLeaveVmDirA(
    PCSTR pszServerName,    /* IN              */
    PCSTR pszUserName,      /* IN              */
    PCSTR pszPassword,      /* IN              */
    DWORD dwLeaveFlags      /* IN              */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszUserName)
    {
        dwError = VmAfdAllocateStringWFromA(pszUserName, &pwszUserName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszPassword)
    {
        dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // Machine credentials will be used if the user name and password are NULL.
    dwError = VmAfdLeaveVmDirW(
                  pwszServerName,
                  pwszUserName,
                  pwszPassword,
                  dwLeaveFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszUserName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdLeaveVmDirW(
    PCWSTR pwszServerName,  /* IN              */
    PCWSTR pwszUserName,    /* IN              */
    PCWSTR pwszPassword,    /* IN              */
    DWORD dwLeaveFlags      /* IN              */
)
{
    DWORD dwError = 0;

    dwError = VmAfdLocalLeaveVmDir(
                      pwszServerName,
                      pwszUserName,
                      pwszPassword,
                      dwLeaveFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLeaveVmDirW failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdCreateComputerAccountA(
    PCSTR            pszUserName,       /* IN              */
    PCSTR            pszPassword,       /* IN              */
    PCSTR            pszMachineName,    /* IN              */
    PCSTR            pszOrgUnit,        /* IN     OPTIONAL */
    PSTR*            ppszOutPassword    /* OUT             */
    )
{
    DWORD dwError = 0;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszMachineName = NULL;
    PWSTR pwszOrgUnit = NULL;
    PWSTR pwszOutPassword = NULL;
    PSTR  pszOutPassword = NULL;

    if (IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszMachineName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszUserName, &pwszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszMachineName, &pwszMachineName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszOrgUnit)
    {
        dwError = VmAfdAllocateStringWFromA(pszOrgUnit, &pwszOrgUnit);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCreateComputerAccountW(
                  pwszUserName,
                  pwszPassword,
                  pwszMachineName,
                  pwszOrgUnit,
                  &pwszOutPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                  pwszOutPassword,
                  &pszOutPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (ppszOutPassword)
    {
        *ppszOutPassword = pszOutPassword;
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszUserName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszMachineName);
    VMAFD_SAFE_FREE_MEMORY(pwszOrgUnit);
    VMAFD_SAFE_FREE_MEMORY(pwszOutPassword);

    return dwError;

error:

    VMAFD_SAFE_FREE_STRINGA(pszOutPassword);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdCreateComputerAccountA failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdCreateComputerAccountW(
    PCWSTR           pwszUserName,      /* IN            */
    PCWSTR           pwszPassword,      /* IN            */
    PCWSTR           pwszMachineName,   /* IN            */
    PCWSTR           pwszOrgUnit,       /* IN   OPTIONAL */
    PWSTR*           ppwszOutPassword   /* OUT           */
    )
{
    DWORD dwError = 0;
    PWSTR pwszOutPassword = NULL;

    if (IsNullOrEmptyString(pwszUserName) ||
        IsNullOrEmptyString(pwszPassword) ||
        IsNullOrEmptyString(pwszMachineName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdLocalCreateComputerAccount(
                  pwszUserName,
                  pwszPassword,
                  pwszMachineName,
                  pwszOrgUnit,
                  &pwszOutPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (ppwszOutPassword)
    {
        *ppwszOutPassword = pwszOutPassword;
    }

cleanup:

    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszOutPassword);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdCreateComputerAccountW failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdJoinADA(
    PCSTR pszServerName,      /* IN     OPTIONAL */
    PCSTR pszUserName,        /* IN              */
    PCSTR pszPassword,        /* IN              */
    PCSTR pszDomainName,      /* IN              */
    PCSTR pszOrgUnit          /* IN     OPTIONAL */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszOrgUnit = NULL;

    if (IsNullOrEmptyString(pszUserName) ||
            IsNullOrEmptyString(pszPassword) ||
            IsNullOrEmptyString(pszDomainName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszUserName, &pwszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszDomainName, &pwszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszOrgUnit)
    {
        dwError = VmAfdAllocateStringWFromA(pszOrgUnit, &pwszOrgUnit);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdJoinADW(
                  pwszServerName,
                  pwszUserName,
                  pwszPassword,
                  pwszDomainName,
                  pwszOrgUnit);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszUserName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pwszOrgUnit);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdJoinADW(
    PCWSTR pwszServerName,  /* IN              */
    PCWSTR pwszUserName,    /* IN              */
    PCWSTR pwszPassword,    /* IN              */
    PCWSTR pwszDomainName,  /* IN              */
    PCWSTR pwszOrgUnit      /* IN     OPTIONAL */
)
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pwszUserName) ||
            IsNullOrEmptyString(pwszPassword) ||
            IsNullOrEmptyString(pwszDomainName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdLocalJoinAD(
                    pwszUserName,
                    pwszPassword,
                    pwszDomainName,
                    pwszOrgUnit);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdJoinADW failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdLeaveADA(
    PCSTR pszServerName,    /* IN              */
    PCSTR pszUserName,      /* IN              */
    PCSTR pszPassword       /* IN              */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;

    if (IsNullOrEmptyString(pszUserName) ||
            IsNullOrEmptyString(pszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszUserName, &pwszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdLeaveADW(
                  pwszServerName,
                  pwszUserName,
                  pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszUserName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdLeaveADW(
    PCWSTR pwszServerName,  /* IN              */
    PCWSTR pwszUserName,    /* IN              */
    PCWSTR pwszPassword     /* IN              */
)
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pwszUserName) ||
            IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdLocalLeaveAD(
                      pwszUserName,
                      pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdLeaveADW failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdQueryADA(
    PCSTR pszServerName,   /* IN           OPTIONAL   */
    PSTR *ppszComputer,   /*          OUT    */
    PSTR *ppszDomain,     /*          OUT    */
    PSTR *ppszDistinguishedName, /*        OUT   OPTIONAL */
    PSTR *ppszNetbiosName        /*        OUT   OPTIONAL */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszComputer = NULL;
    PWSTR pwszDomain = NULL;
    PWSTR pwszDistinguishedName = NULL;
    PWSTR pwszNetbiosName = NULL;
    PSTR pszComputer = NULL;
    PSTR pszDomain = NULL;
    PSTR pszDistinguishedName = NULL;
    PSTR pszNetbiosName = NULL;

    if (!ppszComputer || !ppszDomain)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdQueryADW(
                  pwszServerName,
                  &pwszComputer,
                  &pwszDomain,
                  ppszDistinguishedName ? &pwszDistinguishedName : NULL,
                  ppszNetbiosName ? &pwszNetbiosName : NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pwszComputer)
    {
        dwError = VmAfdAllocateStringAFromW(pwszComputer, &pszComputer);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pwszDomain)
    {
        dwError = VmAfdAllocateStringAFromW(pwszDomain, &pszDomain);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pwszDistinguishedName)
    {
        dwError = VmAfdAllocateStringAFromW(pwszDistinguishedName, &pszDistinguishedName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pwszNetbiosName)
    {
        dwError = VmAfdAllocateStringAFromW(pwszNetbiosName, &pszNetbiosName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszComputer = pszComputer;
    *ppszDomain = pszDomain;
    if (ppszDistinguishedName)
    {
        *ppszDistinguishedName = pszDistinguishedName;
    }
    if (ppszNetbiosName)
    {
        *ppszNetbiosName = pszNetbiosName;
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszComputer);
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszDistinguishedName);
    VMAFD_SAFE_FREE_MEMORY(pwszNetbiosName);

    return dwError;

error:

    VMAFD_SAFE_FREE_STRINGA(pszComputer);
    VMAFD_SAFE_FREE_STRINGA(pszDomain);
    VMAFD_SAFE_FREE_STRINGA(pszDistinguishedName);
    VMAFD_SAFE_FREE_STRINGA(pszNetbiosName);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdQueryADA failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdGetDCList (
    PCSTR  pszServerName,
    PCSTR  pszDomain,
    PDWORD pdwServerCount,
    PVMAFD_DC_INFO_W *ppVmAfdDCInfoList
    )
{
   DWORD dwError = 0;

   if (!pdwServerCount || !ppVmAfdDCInfoList
          || !pszDomain )
   {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_VMAFD_ERROR(dwError);
   }
   if (pszServerName == NULL || (strlen(pszServerName) == 0)
            || VmAfdIsLocalHost(pszServerName))
   {
       dwError = VmAfdLocalGetDCList(
                           pszDomain,
                           pdwServerCount,
                           ppVmAfdDCInfoList
                           );
       BAIL_ON_VMAFD_ERROR(dwError);
   }
   else
   {
       dwError = ERROR_INVALID_PARAMETER;
       BAIL_ON_VMAFD_ERROR(dwError);
   }
cleanup:
    return dwError;
error:
    goto cleanup;
}


DWORD
VmAfdQueryADW(
    PCWSTR pwszServerName,         /* IN       OPTIONAL  */
    PWSTR *ppwszComputer,          /*    OUT             */
    PWSTR *ppwszDomain,            /*    OUT             */
    PWSTR *ppwszDistinguishedName, /*    OUT             */
    PWSTR *ppwszNetbiosName        /*    OUT             */
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;
    PWSTR pwszComputerRpc = NULL;
    PWSTR pwszDomainRpc = NULL;
    PWSTR pwszDistinguishedNameRpc = NULL;
    PWSTR pwszNetbiosNameRpc = NULL;
    PWSTR pwszComputer = NULL;
    PWSTR pwszDomain = NULL;
    PWSTR pwszDistinguishedName = NULL;
    PWSTR pwszNetbiosName = NULL;

    if (!ppwszComputer ||
        !ppwszDomain ||
        !ppwszDistinguishedName ||
        !ppwszNetbiosName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalQueryAD(
                          &pwszComputer,
                          &pwszDomain,
                          &pwszDistinguishedName,
                          &pwszNetbiosName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcQueryAD(
                          hBinding,
                          &pwszComputerRpc,
                          &pwszDomainRpc,
                          &pwszDistinguishedNameRpc,
                          &pwszNetbiosNameRpc);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);

        if (pwszComputerRpc)
        {
            dwError = VmAfdAllocateStringW(pwszComputerRpc, &pwszComputer);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (pwszDomainRpc)
        {
            dwError = VmAfdAllocateStringW(pwszDomainRpc, &pwszDomain);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (pwszDistinguishedNameRpc)
        {
            dwError = VmAfdAllocateStringW(pwszDistinguishedNameRpc, &pwszDistinguishedName);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (pwszNetbiosNameRpc)
        {
            dwError = VmAfdAllocateStringW(pwszNetbiosNameRpc, &pwszNetbiosName);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    *ppwszComputer = pwszComputer;
    *ppwszDomain = pwszDomain;
    *ppwszDistinguishedName = pwszDistinguishedName;
    *ppwszNetbiosName = pwszNetbiosName;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    if (pwszComputerRpc)
    {
        VmAfdRpcClientFreeMemory(pwszComputerRpc);
    }

    if (pwszDomainRpc)
    {
        VmAfdRpcClientFreeMemory(pwszDomainRpc);
    }

    if (pwszDistinguishedNameRpc)
    {
        VmAfdRpcClientFreeMemory(pwszDistinguishedNameRpc);
    }

    if (pwszNetbiosNameRpc)
    {
        VmAfdRpcClientFreeMemory(pwszNetbiosNameRpc);
    }

    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszComputer);
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszDistinguishedName);
    VMAFD_SAFE_FREE_MEMORY(pwszNetbiosName);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdQueryADW failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdForceReplicationA(
    PCSTR pszServerName       /* IN              */
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;

    if (IsNullOrEmptyString(pszServerName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdForceReplicationW(
                  pwszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    goto cleanup;

}

DWORD
VmAfdForceReplicationW(
    PCWSTR pwszServerName       /* IN              */
)
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pwszServerName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdLocalForceReplication(pwszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdForceReplicationW failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmAfdGetPNIDForUrlA(
    PCSTR pszServerName,
    PSTR* ppszPNIDUrl
    )
{
    DWORD  dwError = 0;
    PWSTR  pwszServerName = NULL;
    PWSTR  pwszPNIDUrl = NULL;
    PSTR   pszPNIDUrl = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszPNIDUrl, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetPNIDForUrlW(
                    pwszServerName,
                    &pwszPNIDUrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPNIDUrl, &pszPNIDUrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszPNIDUrl = pszPNIDUrl;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszPNIDUrl);

    return dwError;

error:

    if (ppszPNIDUrl)
    {
        *ppszPNIDUrl = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszPNIDUrl);

    goto cleanup;
}

DWORD
VmAfdGetPNIDForUrlW(
    PCWSTR pwszServerName,
    PWSTR* ppwszPNIDUrl
    )
{
    DWORD  dwError = 0;
    PWSTR  pwszPNID = NULL;
    PWSTR  pwszPNIDUrl = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszPNIDUrl, dwError);

    dwError = VmAfdGetPNIDW(pwszServerName, &pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (VmAfdCheckIfIPV6AddressW(pwszPNID))
    {
        SIZE_T length = 0;
        WCHAR  delimiters[] = { '[', ']', 0 };
        PWSTR  pwszWCursor = NULL;
        PWSTR  pwszRCursor = pwszPNID;

        dwError = VmAfdGetStringLengthW(pwszPNID, &length);
        BAIL_ON_VMAFD_ERROR(dwError);

        length += 2;

        dwError = VmAfdAllocateMemory(
                      (length + 1) * sizeof(WCHAR),
                      (PVOID*)&pwszPNIDUrl);
        BAIL_ON_VMAFD_ERROR(dwError);

        pwszWCursor = pwszPNIDUrl;

        *pwszWCursor++ = delimiters[0];

        for (; pwszRCursor && *pwszRCursor; pwszRCursor++)
        {
            *pwszWCursor++ = *pwszRCursor;
        }

        *pwszWCursor++ = delimiters[1];
    }
    else
    {
        pwszPNIDUrl = pwszPNID;
        pwszPNID = NULL;
    }

    *ppwszPNIDUrl = pwszPNIDUrl;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszPNID);

    return dwError;

error:

    if (ppwszPNIDUrl)
    {
        *ppwszPNIDUrl = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pwszPNIDUrl);

    goto cleanup;
}

DWORD
VmAfdGetPNIDA(
    PCSTR pszServerName,
    PSTR* ppszPNID
)
{
    DWORD  dwError = 0;
    PWSTR  pwszPNID = NULL;
    PSTR   pszPNID = NULL;
    PWSTR  pwszServerName = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszPNID, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetPNIDW(pwszServerName, &pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPNID, &pszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszPNID = pszPNID;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszPNID);

    return dwError;

error:

    if (ppszPNID)
    {
        *ppszPNID = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdGetPNIDW(
    PCWSTR pwszServerName,
    PWSTR* ppwszPNID
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;
    PWSTR  pwszPNID = NULL;
    PWSTR  pwszPNIDRet = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszPNID, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetPNID(&pwszPNIDRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetPNID(hBinding, &pwszPNID);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringW(pwszPNID, &pwszPNIDRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszPNID = pwszPNIDRet;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    VMAFD_RPC_SAFE_FREE_MEMORY(pwszPNID);

    return dwError;

error:

    if (ppwszPNID)
    {
        *ppwszPNID = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdSetPNIDA(
    PCSTR pszServerName,
    PCSTR pszPNID
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszPNID = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pszPNID, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszPNID, &pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSetPNIDW(pwszServerName, pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszPNID);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdSetPNIDW(
    PCWSTR pwszServerName,
    PCWSTR pwszPNID
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszPNID, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalSetPNID(pwszPNID);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcSetPNID(hBinding, (PWSTR)pwszPNID);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdGetCAPathA(
    PCSTR pszServerName,
    PSTR* ppszPath
)
{
    DWORD  dwError = 0;
    PWSTR  pwszPath = NULL;
    PSTR   pszPath = NULL;
    PWSTR  pwszServerName = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszPath, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetCAPathW(pwszServerName, &pwszPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPath, &pszPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszPath = pszPath;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszPath);

    return dwError;

error:

    if (ppszPath)
    {
        *ppszPath = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdGetCAPathW(
    PCWSTR pwszServerName,
    PWSTR* ppwszPath
)
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;
    PWSTR  pwszPath = NULL;
    PWSTR  pwszPathRet = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszPath, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalGetCAPath(&pwszPathRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetCAPath(hBinding, &pwszPath);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringW(pwszPath, &pwszPathRet);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszPath = pwszPathRet;

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    VMAFD_RPC_SAFE_FREE_MEMORY(pwszPath);

    return dwError;

error:

    if (ppwszPath)
    {
        *ppwszPath = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdSetCAPathA(
    PCSTR pszServerName,
    PCSTR pszPath
)
{
    DWORD dwError = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszPath = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pszPath, dwError);

    if (pszServerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszServerName, &pwszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszPath, &pwszPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSetCAPathW(pwszServerName, pwszPath);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszPath);
    VMAFD_SAFE_FREE_MEMORY(pwszServerName);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdSetCAPathW(
    PCWSTR pwszServerName,
    PCWSTR pwszPath
    )
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszPath, dwError);

    if (VmAfdIsLocalHostW(pwszServerName))
    {
        dwError = VmAfdLocalSetCAPath(pwszPath);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCreateBindingHandleW(
                        pwszServerName,
                        pwszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMAFD_ERROR(dwError);

        DCETHREAD_TRY
        {
            dwError = VmAfdRpcSetCAPath(hBinding, (PWSTR)pwszPath);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    if (hBinding)
    {
        VmAfdFreeBindingHandle(&hBinding);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdStartHeartbeatA(
    PCSTR            pszServiceName,
    DWORD            dwServicePort,
    PVMAFD_HB_HANDLE *ppHandle
    )
{
    DWORD dwError = 0;
    PWSTR pwszServiceName = NULL;
    PVMAFD_HB_HANDLE pHandle = NULL;

    if (IsNullOrEmptyString(pszServiceName) ||
        !ppHandle
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(
                                  pszServiceName,
                                  &pwszServiceName
                                  );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdStartHeartbeatW(
                                pwszServiceName,
                                dwServicePort,
                                &pHandle
                                );
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppHandle = pHandle;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServiceName);
    return dwError;
error:

    if (ppHandle)
    {
        *ppHandle = NULL;
    }
    if (pHandle)
    {
        VmAfdStopHeartbeat(pHandle);
    }

    goto cleanup;
}

DWORD
VmAfdStartHeartbeatW(
    PCWSTR           pwszServiceName,
    DWORD            dwServicePort,
    PVMAFD_HB_HANDLE *ppHandle
    )
{
    DWORD dwError = 0;
    PVMAFD_HB_HANDLE pHandle = NULL;

    if (IsNullOrEmptyString(pwszServiceName) ||
        !ppHandle
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                          sizeof(VMAFD_HB_HANDLE),
                          (PVOID*)&pHandle
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdLocalPostHeartbeat(pwszServiceName, dwServicePort);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringW(
                              pwszServiceName,
                              &pHandle->pszServiceName
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    pHandle->dwPort = dwServicePort;
    pHandle->condStopHeartbeat = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    pHandle->mutStopHeartbeat = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;

    dwError = pthread_create(
                      &pHandle->threadHandle,
                      NULL,
                      &VmAfdHeartbeatWorker,
                      (PVOID) pHandle
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    pHandle->pThreadHandle = &pHandle->threadHandle;

    *ppHandle = pHandle;

cleanup:

    return dwError;
error:

    if (ppHandle)
    {
        *ppHandle = NULL;
    }
    if (pHandle)
    {
        VmAfdStopHeartbeat(pHandle);
    }
    goto cleanup;
}

DWORD
VmAfdGetHeartbeatStatusA(
    PVMAFD_SERVER       pServer,
    PVMAFD_HB_STATUS_A* ppHeartbeatStatus
    )
{
    DWORD dwError = 0;
    PVMAFD_HB_STATUS_A pHeartbeatStatus = NULL;
    PVMAFD_HB_STATUS_W pwHeartbeatStatus = NULL;

    if (!ppHeartbeatStatus || !pServer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetHeartbeatStatusW(
                                  pServer,
                                  &pwHeartbeatStatus
                                  );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdHeartbeatStatusAFromW(
                                  pwHeartbeatStatus,
                                  &pHeartbeatStatus
                                  );
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppHeartbeatStatus = pHeartbeatStatus;

cleanup:

    if (pwHeartbeatStatus)
    {
        VmAfdFreeHeartbeatStatusW(pwHeartbeatStatus);
    }
    return dwError;
error:

    if (ppHeartbeatStatus)
    {
        *ppHeartbeatStatus = NULL;
    }
    if (pHeartbeatStatus)
    {
        VmAfdFreeHeartbeatStatusA(pHeartbeatStatus);
    }
    goto cleanup;
}

DWORD
VmAfdGetHeartbeatStatusW(
    PVMAFD_SERVER       pServer,
    PVMAFD_HB_STATUS_W* ppHeartbeatStatus
    )
{
    DWORD dwError = 0;
    PVMAFD_HB_STATUS_W pHeartbeatStatus = NULL;
    PVMAFD_HB_STATUS_W pRpcHeartbeatStatus = NULL;

    if (!pServer || !ppHeartbeatStatus)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pServer->hBinding)
    {
        dwError = VmAfdLocalGetHeartbeatStatus(&pHeartbeatStatus);
    }
    else
    {
        DCETHREAD_TRY
        {
            dwError = VmAfdRpcGetHeartbeatStatus(
                                          pServer->hBinding,
                                          &pRpcHeartbeatStatus
                                          );
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
        }
        DCETHREAD_ENDTRY;

        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateFromRpcHeartbeatStatus(
                                          pRpcHeartbeatStatus,
                                          &pHeartbeatStatus
                                          );
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppHeartbeatStatus = pHeartbeatStatus;

cleanup:

    if (pRpcHeartbeatStatus)
    {
        VmAfdRpcClientFreeHeartbeatStatus(pRpcHeartbeatStatus);
    }
    return dwError;
error:

    if (ppHeartbeatStatus)
    {
        *ppHeartbeatStatus = NULL;
    }
    if (pHeartbeatStatus)
    {
        VmAfdFreeHeartbeatStatusW(pHeartbeatStatus);
    }
    goto cleanup;
}

VOID
VmAfdFreeHeartbeatStatusA(
    PVMAFD_HB_STATUS_A pHeartbeatStatus
    )
{
    if (pHeartbeatStatus)
    {
        VmAfdFreeHbStatusA(pHeartbeatStatus);
    }
}

VOID
VmAfdFreeHeartbeatStatusW(
    PVMAFD_HB_STATUS_W pHeartbeatStatus
    )
{
    if (pHeartbeatStatus)
    {
        VmAfdFreeHbStatusW(pHeartbeatStatus);
    }
}

VOID
VmAfdStopHeartbeat(
    PVMAFD_HB_HANDLE pHandle
    )
{
    if (pHandle)
    {
        pthread_cond_signal(&pHandle->condStopHeartbeat);
        if (pHandle->pThreadHandle)
        {
            pthread_join(*pHandle->pThreadHandle,NULL);

            pHandle->pThreadHandle = NULL;

        }
        pthread_cond_destroy(&pHandle->condStopHeartbeat);
        pthread_mutex_destroy(&pHandle->mutStopHeartbeat);
        VMAFD_SAFE_FREE_MEMORY(pHandle->pszServiceName);
        VMAFD_SAFE_FREE_MEMORY(pHandle);
    }
}

DWORD
VmAfdRefreshSiteName(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmAfdLocalRefreshSiteName();
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdConfigureDNSA(
    PCSTR pszUserName,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;

    BAIL_ON_VMAFD_EMPTY_STRING(pszUserName, dwError);
    BAIL_ON_VMAFD_EMPTY_STRING(pszPassword, dwError);

    dwError = VmAfdAllocateStringWFromA(
                pszUserName,
                &pwszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(
                pszPassword,
                &pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdConfigureDNSW(pwszUserName, pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    VMAFD_SAFE_FREE_MEMORY(pwszUserName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);

    return dwError;
}

DWORD
VmAfdConfigureDNSW(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_EMPTY_STRING(pwszUserName, dwError);
    BAIL_ON_VMAFD_EMPTY_STRING(pwszPassword, dwError);

    dwError = VmAfdLocalConfigureDNSW(pwszUserName, pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

error:
    return dwError;
}

PVMAFD_SERVER
VmAfdAcquireServer(
    PVMAFD_SERVER pServer
    )
{
    if (pServer)
    {
        InterlockedIncrement(&pServer->refCount);
    }

    return pServer;
}

VOID
VmAfdReleaseServer(
    PVMAFD_SERVER pServer
    )
{
    if (pServer && InterlockedDecrement(&pServer->refCount) == 0)
    {
        VmAfdFreeServer(pServer);
    }
}


DWORD
VmAfdGetErrorMsgByCode(
    DWORD dwErrorCode,
    PSTR *pszErrMsg
    )
{
    return VmAfdGetErrorString(
                      dwErrorCode,
                      pszErrMsg);
}

static
VOID
VmAfdFreeServer(
    PVMAFD_SERVER pServer
    )
{
    if (pServer)
    {
        if (pServer->hBinding)
        {
            VmAfdFreeBindingHandle(&pServer->hBinding);
        }
        VMAFD_SAFE_FREE_STRINGA(pServer->pszServerName);
        VMAFD_SAFE_FREE_STRINGA(pServer->pszUserName);
        VMAFD_SAFE_FREE_MEMORY(pServer);
    }
}

static
PVOID
VmAfdHeartbeatWorker(
    PVOID pThreadArgs
    )
{
    DWORD dwError = 0;
    PVMAFD_HB_HANDLE pInfo = NULL;
    BOOL bCanceled = FALSE;
    struct timespec ts = {0};

    pInfo = (PVMAFD_HB_HANDLE)pThreadArgs;

    do
    {
        dwError = VmAfdLocalPostHeartbeat(
                                      pInfo->pszServiceName,
                                      pInfo->dwPort
                                      );
        dwError = 0;
        BAIL_ON_VMAFD_ERROR(dwError);

        pthread_mutex_lock(&pInfo->mutStopHeartbeat);

        ts.tv_sec = time(NULL) + VMAFD_HEARTBEAT_INTERVAL;
        dwError = pthread_cond_timedwait(
                                     &pInfo->condStopHeartbeat,
                                     &pInfo->mutStopHeartbeat,
                                     &ts
                                     );

        if (dwError != ETIMEDOUT)
        {
            bCanceled = TRUE;
        }
        pthread_mutex_unlock(&pInfo->mutStopHeartbeat);
    } while(!bCanceled);


cleanup:

    return NULL;
error:

    goto cleanup;
}

DWORD
VmAfdAllocateFromRpcHeartbeatStatus(
   PVMAFD_HB_STATUS_W   pHeartbeatStatusSrc,
   PVMAFD_HB_STATUS_W *ppHeartbeatStatusDest
   )
{
    DWORD dwError = 0;
    PVMAFD_HB_STATUS_W pHeartbeatStatusDest = NULL;

    if (!pHeartbeatStatusSrc ||
        !ppHeartbeatStatusDest
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                            sizeof(VMAFD_HB_STATUS_W),
                            (PVOID *)&pHeartbeatStatusDest
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    pHeartbeatStatusDest->bIsAlive = pHeartbeatStatusSrc->bIsAlive;

    if (pHeartbeatStatusSrc->pHeartbeatInfoArr)
    {
        DWORD dwIndex = 0;

        dwError = VmAfdAllocateMemory(
                         sizeof(VMAFD_HB_INFO_W)*pHeartbeatStatusSrc->dwCount,
                         (PVOID *)&pHeartbeatStatusDest->pHeartbeatInfoArr
                         );
        BAIL_ON_VMAFD_ERROR(dwError);

        pHeartbeatStatusDest->dwCount = pHeartbeatStatusSrc->dwCount;

        for (; dwIndex < pHeartbeatStatusSrc->dwCount; ++dwIndex)
        {
            PVMAFD_HB_INFO_W pSrc =
                        &pHeartbeatStatusSrc->pHeartbeatInfoArr[dwIndex];
            PVMAFD_HB_INFO_W pDst=
                        &pHeartbeatStatusDest->pHeartbeatInfoArr[dwIndex];

            pDst->dwPort = pSrc->dwPort;
            pDst->dwLastHeartbeat = pSrc->dwLastHeartbeat;
            pDst->bIsAlive = pSrc->bIsAlive;

            if (pSrc->pszServiceName)
            {
                dwError = VmAfdAllocateStringW(
                                      pSrc->pszServiceName,
                                      &pDst->pszServiceName
                                      );
                BAIL_ON_VMAFD_ERROR(dwError);
            }
         }
    }

    *ppHeartbeatStatusDest = pHeartbeatStatusDest;

cleanup:

    return dwError;
error:

    if (ppHeartbeatStatusDest)
    {
        *ppHeartbeatStatusDest = NULL;
    }
    if (pHeartbeatStatusDest)
    {
        VmAfdFreeHeartbeatStatusW(pHeartbeatStatusDest);
    }
    goto cleanup;
}

VOID
VmAfdRpcClientFreeHeartbeatStatus(
   PVMAFD_HB_STATUS_W pHeartbeatStatus
   )
{
    if (pHeartbeatStatus)
    {
        if (pHeartbeatStatus->pHeartbeatInfoArr)
        {
            DWORD dwIndex = 0;

            for (; dwIndex < pHeartbeatStatus->dwCount; ++dwIndex)
            {
                PVMAFD_HB_INFO_W pCursor=
                              &pHeartbeatStatus->pHeartbeatInfoArr[dwIndex];

                if (pCursor->pszServiceName)
                {
                    VmAfdRpcClientFreeMemory(pCursor->pszServiceName);
                }
            }

            VmAfdRpcClientFreeMemory(pHeartbeatStatus->pHeartbeatInfoArr);
        }

        VmAfdRpcClientFreeMemory(pHeartbeatStatus);
    }
}

DWORD
VmAfdHeartbeatStatusAFromW(
    PVMAFD_HB_STATUS_W  pwHeartbeatStatus,
    PVMAFD_HB_STATUS_A* ppHeartbeatStatus
    )
{
    DWORD dwError = 0;
    PVMAFD_HB_STATUS_A pHeartbeatStatus = NULL;

    if (!pwHeartbeatStatus || !ppHeartbeatStatus)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                            sizeof(VMAFD_HB_STATUS_A),
                            (PVOID *)&pHeartbeatStatus
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pwHeartbeatStatus->dwCount)
    {
        DWORD dwIndex = 0;

        dwError = VmAfdAllocateMemory(
                            sizeof(VMAFD_HB_INFO_A)*pwHeartbeatStatus->dwCount,
                            (PVOID*)&pHeartbeatStatus->pHeartbeatInfoArr
                            );
        BAIL_ON_VMAFD_ERROR(dwError);

        for (; dwIndex < pwHeartbeatStatus->dwCount; ++dwIndex)
        {
            PVMAFD_HB_INFO_W pCurrHeartbeatInfoW =
                              &pwHeartbeatStatus->pHeartbeatInfoArr[dwIndex];
            PVMAFD_HB_INFO_A pCurrHeartbeatInfoA =
                              &pHeartbeatStatus->pHeartbeatInfoArr[dwIndex];

            dwError = VmAfdAllocateStringAFromW(
                              pCurrHeartbeatInfoW->pszServiceName,
                              &pCurrHeartbeatInfoA->pszServiceName
                              );
            BAIL_ON_VMAFD_ERROR(dwError);

            pCurrHeartbeatInfoA->dwPort = pCurrHeartbeatInfoW->dwPort;
            pCurrHeartbeatInfoA->dwLastHeartbeat =
                                 pCurrHeartbeatInfoW->dwLastHeartbeat;
            pCurrHeartbeatInfoA->bIsAlive = pCurrHeartbeatInfoW->bIsAlive;
        }
    }

    pHeartbeatStatus->dwCount = pwHeartbeatStatus->dwCount;
    pHeartbeatStatus->bIsAlive = pwHeartbeatStatus->bIsAlive;

    *ppHeartbeatStatus = pHeartbeatStatus;

cleanup:

    return dwError;
error:

    if (ppHeartbeatStatus)
    {
        *ppHeartbeatStatus = NULL;
    }
    if (pHeartbeatStatus)
    {
        VmAfdFreeHeartbeatStatusA(pHeartbeatStatus);
    }
    goto cleanup;
}

DWORD
VmAfdChangePNIDA(
    PCSTR pszUserName,
    PCSTR pszPassword,
    PCSTR pszPNID
    )
{
    DWORD dwError = 0;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszPNID = NULL;

    if (IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszPNID))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringWFromA(pszUserName, &pwszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszPassword, &pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszPNID, &pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdChangePNIDW(pwszUserName, pwszPassword, pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pwszPNID);
    VMAFD_SAFE_FREE_MEMORY(pwszUserName);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdChangePNIDW(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszPNID
    )
{
    DWORD dwError = 0;

    dwError = VmAfdLocalChangePNID(pwszUserName, pwszPassword, pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ANY, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmAfdFreeString(
    PSTR pszString)
{
    VMAFD_SAFE_FREE_STRINGA(pszString);
}

VOID
VmAfdFreeWString(
    PWSTR pwszString)
{
    VMAFD_SAFE_FREE_STRINGW(pwszString);
}



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

// macros use this function
// static
DWORD
VmAuthsvcRpcGetErrorCode(
    dcethread_exc* pDceException
    )
{
    DWORD dwError = 0;

    dwError = dcethread_exc_getstatus(pDceException);

#ifndef _WIN32
    dwError = LwNtStatusToWin32Error(LwRpcStatusToNtStatus(dwError));
#endif

    return dwError;
}

// Public client interfaces for RPC calls
DWORD
VmAuthsvcJoinRealmW(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszMachineName,
    PCWSTR pwszDomainName,
    PCWSTR pwszOrgUnit)
{
    DWORD dwError = 0;
    rpc_binding_handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    dwError = VmAuthsvcCreateBindingHandleW(
                    pwszServerName,
                    pwszServerEndpoint,
                    &hBinding);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    DCETHREAD_TRY
    {
        dwError = VmAuthsvcRpcJoinRealm(
                          hBinding,
                          (PWSTR)pwszServerName,
                          (PWSTR)pwszUserName,
                          (PWSTR)pwszPassword,
                          (PWSTR)pwszMachineName,
                          (PWSTR)pwszDomainName,
                          (PWSTR)pwszOrgUnit);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        VmAuthsvcRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    if (hBinding)
    {
        VmAuthsvcFreeBindingHandle(&hBinding);
    }

    return dwError;
}

DWORD
VmAuthsvcLeavRealmW(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszDomainName)
{
    DWORD dwError = 0;
    rpc_binding_handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    dwError = VmAuthsvcCreateBindingHandleW(
                    pwszServerName,
                    pwszServerEndpoint,
                    &hBinding);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    DCETHREAD_TRY
    {
        dwError = VmAuthsvcRpcLeaveRealm(
                          hBinding,
                          (PWSTR)pwszServerName,
                          (PWSTR)pwszUserName,
                          (PWSTR)pwszDomainName);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        VmAuthsvcRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    if (hBinding)
    {
        VmAuthsvcFreeBindingHandle(&hBinding);
    }

    return dwError;
}

DWORD
VmAuthsvcIsJoinedToVMDirectoryW(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszDomainName)
{
    DWORD dwError = 0;
    rpc_binding_handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    dwError = VmAuthsvcCreateBindingHandleW(
                    pwszServerName,
                    pwszServerEndpoint,
                    &hBinding);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    DCETHREAD_TRY
    {
        dwError = VmAuthsvcRpcIsJoinedToVMDirectory(
                          hBinding,
                          (PWSTR)pwszServerName,
                          (PWSTR)pwszUserName,
                          (PWSTR)pwszDomainName);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        VmAuthsvcRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    if (hBinding)
    {
        VmAuthsvcFreeBindingHandle(&hBinding);
    }

    return dwError;
}

// Public client interfaces for RPC calls
DWORD
VmAuthsvcJoinADDomainW(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszMachineName,
    PCWSTR pwszDomainName,
    PCWSTR pwszOrgUnit)
{
    DWORD dwError = 0;
    rpc_binding_handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    dwError = VmAuthsvcCreateBindingHandleW(
                    pwszServerName,
                    pwszServerEndpoint,
                    &hBinding);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    DCETHREAD_TRY
    {
        dwError = VmAuthsvcRpcJoinADDomain(
                          hBinding,
                          (PWSTR)pwszServerName,
                          (PWSTR)pwszUserName,
                          (PWSTR)pwszPassword,
                          (PWSTR)pwszMachineName,
                          (PWSTR)pwszDomainName,
                          (PWSTR)pwszOrgUnit);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        VmAuthsvcRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    if (hBinding)
    {
        VmAuthsvcFreeBindingHandle(&hBinding);
    }

    return dwError;
}

DWORD
VmAuthsvcLeavADDomainW(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszDomainName)
{
    DWORD dwError = 0;
    rpc_binding_handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    dwError = VmAuthsvcCreateBindingHandleW(
                    pwszServerName,
                    pwszServerEndpoint,
                    &hBinding);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    DCETHREAD_TRY
    {
        dwError = VmAuthsvcRpcLeaveADDomain(
                          hBinding,
                          (PWSTR)pwszServerName,
                          (PWSTR)pwszUserName,
                          (PWSTR)pwszDomainName);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        VmAuthsvcRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    if (hBinding)
    {
        VmAuthsvcFreeBindingHandle(&hBinding);
    }

    return dwError;
}

DWORD
VmAuthsvcIsJoinedToADDomainW(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszDomainName)
{
    DWORD dwError = 0;
    rpc_binding_handle_t hBinding = NULL;
    PCWSTR pwszServerEndpoint = NULL;

    dwError = VmAuthsvcCreateBindingHandleW(
                    pwszServerName,
                    pwszServerEndpoint,
                    &hBinding);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    DCETHREAD_TRY
    {
        dwError = VmAuthsvcRpcIsJoinedToADDomain(
                          hBinding,
                          (PWSTR)pwszServerName,
                          (PWSTR)pwszUserName,
                          (PWSTR)pwszDomainName);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        VmAuthsvcRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    if (hBinding)
    {
        VmAuthsvcFreeBindingHandle(&hBinding);
    }

    return dwError;
}

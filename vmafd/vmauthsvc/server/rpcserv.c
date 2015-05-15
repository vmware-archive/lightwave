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

UINT32
VmAuthsvcRpcJoinRealm(
    rpc_binding_handle_t hBinding,
    PWSTR pwszServerName,
    PWSTR pwszUserName,
    PWSTR pwszPassword,
    PWSTR pwszMachineName,
    PWSTR pwszDomainName,
    PWSTR pwszOrgUnit)
{
    UINT32 dwError = 0;

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return dwError;
}

UINT32
VmAuthsvcRpcLeaveRealm(
    rpc_binding_handle_t hBinding,
    PWSTR pwszServerName,
    PWSTR pwszUserName,
    PWSTR pwszDomainName)
{
    UINT32 dwError = 0;

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return dwError;
}

UINT32
VmAuthsvcRpcIsJoinedToVMDirectory(
    rpc_binding_handle_t hBinding,
    PWSTR pwszServerName,
    PWSTR pwszUserName,
    PWSTR pwszDomainName)
{
    UINT32 dwError = 0;

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return dwError;
}

UINT32
VmAuthsvcRpcJoinADDomain(
    rpc_binding_handle_t hBinding,
    PWSTR pwszServerName,
    PWSTR pwszUserName,
    PWSTR pwszPassword,
    PWSTR pwszMachineName,
    PWSTR pwszDomainName,
    PWSTR pwszOrgUnit)
{
    UINT32 dwError = 0;

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return dwError;
}

UINT32
VmAuthsvcRpcLeaveADDomain(
    rpc_binding_handle_t hBinding,
    PWSTR pwszServerName,
    PWSTR pwszUserName,
    PWSTR pwszDomainName)
{
    UINT32 dwError = 0;

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return dwError;
}

UINT32
VmAuthsvcRpcIsJoinedToADDomain(
    rpc_binding_handle_t hBinding,
    PWSTR pwszServerName,
    PWSTR pwszUserName,
    PWSTR pwszDomainName)
{
    UINT32 dwError = 0;

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return dwError;
}

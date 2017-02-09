/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
_MapErrorCode(
    DWORD dwCode)
{
    DWORD dwError = 0;

    if (dwCode == NERR_SetupAlreadyJoined)
    {
        dwError = ERROR_IS_JOINED;
    }
    else if (dwCode == NERR_SetupNotJoined)
    {
        dwError = ERROR_NOT_JOINED;
    }
    else
    {
        dwError = dwCode;
    }

    return dwError;
}

DWORD
VmAfSrvJoinAD(
    PWSTR    pwszUserName,       /* IN            */
    PWSTR    pwszPassword,       /* IN            */
    PWSTR    pwszDomainName,     /* IN            */
    PWSTR    pwszOrgUnit         /* IN   OPTIONAL */
    )
{
    DWORD dwError = 0;
    NET_API_STATUS status = NERR_Success;
    PWSTR pwszUserNameUpn = pwszUserName;
    PWSTR pwszTemp = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszDomainName, dwError);

    if ((VmAfdStringChrW(pwszUserName, L'@') == NULL) && (VmAfdStringChrW(pwszUserName, L'\\') == NULL))
    {
	size_t usernameLen = 0;
	size_t domainLen = 0;
	size_t len = 0;

	VmAfdGetStringLengthW(pwszUserName, &usernameLen);
	VmAfdGetStringLengthW(pwszDomainName, &domainLen);

        len = usernameLen + domainLen + 1;

	dwError = VmAfdAllocateMemory( (len + 1)*sizeof(WCHAR), (PVOID *)&pwszTemp);
	BAIL_ON_VMAFD_ERROR(dwError);

	dwError = VmAfdStringCpyW(pwszTemp, len, pwszUserName);
	BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdStringCatW(pwszTemp, len+1, L"@");
        BAIL_ON_VMAFD_ERROR(dwError);

	dwError = VmAfdStringCatW(pwszTemp, len+1, pwszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);

	pwszUserNameUpn = pwszTemp;
    }

    status = NetJoinDomain(
                  NULL,
                  pwszDomainName,
                  pwszOrgUnit,
                  pwszUserNameUpn,
                  pwszPassword,
                  NETSETUP_JOIN_DOMAIN | NETSETUP_ACCT_CREATE | NETSETUP_DOMAIN_JOIN_IF_JOINED);
    dwError = _MapErrorCode(status);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_STRINGW(pwszTemp);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "Failed to join AD. Error(%u)",
                              dwError);

    goto cleanup;
}

DWORD
VmAfSrvLeaveAD(
    PWSTR    pwszUserName,      /* IN              */
    PWSTR    pwszPassword       /* IN              */
    )
{
    DWORD dwError = 0;
    NET_API_STATUS status = NERR_Success;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);

    status = NetUnjoinDomain(
                  NULL,
                  pwszUserName,
                  pwszPassword,
                  0);
    dwError = _MapErrorCode(status);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "Failed to leave AD. Error(%u)",
                              dwError);

    goto cleanup;
}

DWORD
VmAfSrvQueryAD(
    PWSTR   *ppwszComputer,     /*             OUT */
    PWSTR   *ppwszDomain,       /*             OUT */
    PWSTR   *ppwszDistinguishedName, /*             OUT OPTIONAL */
    PWSTR   *ppwszNetbiosName        /*             OUT OPTIONAL */
    )
{
    DWORD dwError = 0;
    PWSTR pwszComputerName = NULL;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszDistinguishedName = NULL;
    PWSTR pwszNetbiosNameBuffer = NULL;
    PWSTR pwszNetbiosName = NULL;
    NET_API_STATUS status = NERR_Success;
    NETSETUP_JOIN_STATUS joinStatus = NetSetupUnknownStatus;
    PWSTR *ppwszComputerNameArray = NULL;
    DWORD dwCount = 0;
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC *pDomainInfo = NULL;

    status = NetGetJoinInformation(
                 NULL,
                 &pwszNetbiosNameBuffer,
                 &joinStatus);
    dwError = _MapErrorCode(status);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (joinStatus != NetSetupDomainName)
    {
        dwError = ERROR_NOT_JOINED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(pwszNetbiosNameBuffer, &pwszNetbiosName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DsRoleGetPrimaryDomainInformation(
                   NULL,
                   DsRolePrimaryDomainInfoBasic,
                   (PBYTE *)&pDomainInfo);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pDomainInfo->DomainNameDns)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringW(pDomainInfo->DomainNameDns, &pwszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    status = NetEnumerateComputerNames(
                 NULL,
                 NetPrimaryComputerName,
                 0,
                 &dwCount,
                 &ppwszComputerNameArray);
    dwError = _MapErrorCode(status);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringW(ppwszComputerNameArray[0], &pwszComputerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszComputer = pwszComputerName;
    *ppwszDomain = pwszDomainName;
    *ppwszDistinguishedName = pwszDistinguishedName;
    *ppwszNetbiosName = pwszNetbiosName;

cleanup:

    if (pDomainInfo)
    {
        DsRoleFreeMemory((PBYTE)pDomainInfo);
    }

    if (pwszNetbiosNameBuffer)
    {
        NetApiBufferFree(pwszNetbiosNameBuffer);
    }

    if (ppwszComputerNameArray)
    {
        NetApiBufferFree(ppwszComputerNameArray);
    }

    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pwszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pwszNetbiosName);
    VMAFD_SAFE_FREE_MEMORY(pwszComputerName);

    VmAfdLog(VMAFD_DEBUG_ANY, "Failed to query AD. Error(%u)",
                              dwError);

    goto cleanup;
}

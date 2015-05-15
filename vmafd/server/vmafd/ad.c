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

DWORD
VmAfSrvJoinAD(
    PWSTR    pwszUserName,       /* IN            */
    PWSTR    pwszPassword,       /* IN            */
    PWSTR    pwszDomainName,     /* IN            */
    PWSTR    pwszOrgUnit         /* IN   OPTIONAL */
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszOrgUnit = NULL;
    PSTR pszDefaultRealm = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszDomainName, dwError);

    dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDomainName, &pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pwszOrgUnit)
    {
        dwError = VmAfdAllocateStringAFromW(pwszOrgUnit, &pszOrgUnit);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(pszDomainName, &pszDefaultRealm);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdUpperCaseStringA(pszDefaultRealm);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DJJoinDomain(pszDomainName,
                           pszOrgUnit,
                           pszUserName,
                           pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);
    VMAFD_SAFE_FREE_STRINGA(pszDomainName);
    VMAFD_SAFE_FREE_STRINGA(pszOrgUnit);
    VMAFD_SAFE_FREE_STRINGA(pszDefaultRealm);

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
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);

    dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DJUnjoinDomain(pszUserName, pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "Failed to leave AD. Error(%u)",
                              dwError);

    goto cleanup;
}

DWORD
VmAfSrvQueryAD(
    PWSTR   *ppwszComputer,          /*    OUT          */
    PWSTR   *ppwszDomain,            /*    OUT          */
    PWSTR   *ppwszDistinguishedName, /*    OUT          */
    PWSTR   *ppwszNetbiosName        /*    OUT          */
    )
{
    DWORD dwError = 0;
    PSTR pszComputerName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszComputerDN = NULL;
    PSTR pszDistinguishedName = NULL;
    PWSTR pwszComputerName = NULL;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszDistinguishedName = NULL;
    PWSTR pwszNetbiosName = NULL;
    PLWNET_DC_INFO pDcInfo = NULL;

    dwError = DJQueryJoinInformation(&pszComputerName,
                                     &pszDomainName,
                                     &pszComputerDN);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszComputerName)
    {
        dwError = VmAfdAllocateStringWFromA(pszComputerName, &pwszComputerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszDomainName)
    {
        dwError = VmAfdAllocateStringWFromA(pszDomainName, &pwszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = LWNetGetDCName(
                        NULL,
                        pszDomainName,
                        NULL,
                        0,
                        &pDcInfo);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (pDcInfo->pszNetBIOSDomainName)
        {
            dwError = VmAfdAllocateStringWFromA(
                              pDcInfo->pszNetBIOSDomainName,
                              &pwszNetbiosName);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    if (pszDistinguishedName)
    {
        dwError = VmAfdAllocateStringWFromA(pszDistinguishedName, &pwszDistinguishedName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszComputer = pwszComputerName;
    *ppwszDomain = pwszDomainName;
    *ppwszDistinguishedName = pwszDistinguishedName;
    *ppwszNetbiosName = pwszNetbiosName;

cleanup:

    if (pszComputerName)
    {
        DJFreeMemory(pszComputerName);
    }
    if (pszDomainName)
    {
        DJFreeMemory(pszDomainName);
    }
    if (pszComputerDN)
    {
        DJFreeMemory(pszComputerDN);
    }
    if (pszDistinguishedName)
    {
        VMAFD_SAFE_FREE_STRINGA(pszDistinguishedName);
    }
    LWNET_SAFE_FREE_DC_INFO(pDcInfo);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "Failed to query AD. Error(%u)",
                              dwError);

    goto cleanup;
}

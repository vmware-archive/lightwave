/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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
VmDirCreateComputerAccountInternal(
    PVDIR_CONNECTION pConn,
    PCSTR pszMachineName,
    PCSTR pszDomainName,
    PCSTR pszOrgUnit,
    PVMDIR_MACHINE_INFO_A *ppMachineInfo
    )
{
    DWORD dwError = 0;
    PVMDIR_MACHINE_INFO_A pMachineInfo = NULL;
    PSTR pszDomain = NULL;

    if (!pConn ||
        IsNullOrEmptyString(pConn->AccessInfo.pszNormBindedDn) ||
        IsNullOrEmptyString(pszMachineName) ||
        !ppMachineInfo
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pszDomainName))
    {
        dwError = VmDirDomainDNToName(
                      pConn->AccessInfo.pszNormBindedDn,
                      &pszDomain);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszDomainName = pszDomain;
    }

    if (IsNullOrEmptyString(pszOrgUnit))
    {
        pszOrgUnit = VMDIR_COMPUTERS_RDN_VAL;
    }

    dwError = VmDirSrvSetupComputerAccount(
                  pConn,
                  pszDomainName,
                  pszOrgUnit,
                  pszMachineName,
                  &pMachineInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMachineInfo = pMachineInfo;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s error (%u)", __FUNCTION__, dwError);
    if (pMachineInfo)
    {
        VmDirFreeMachineInfoA(pMachineInfo);
    }
    goto cleanup;
}

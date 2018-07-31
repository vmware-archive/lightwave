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
VmDirJoinAtomic(
    PVDIR_CONNECTION pConn,
    PCSTR pszMachineName,
    PCSTR pszOrgUnit,
    BOOLEAN bPreJoined,
    PVMDIR_MACHINE_INFO_A *ppMachineInfo,
    PVMDIR_KRB_INFO *ppKrbInfo
    )
{
    DWORD dwError = 0;
    PVMDIR_MACHINE_INFO_A  pMachineInfo = NULL;
    PVMDIR_KRB_INFO pKrbInfo = NULL;
    PSTR pszDomainName = NULL;

    if (!pConn ||
        IsNullOrEmptyString(pConn->AccessInfo.pszNormBindedDn) ||
        IsNullOrEmptyString(pszMachineName) ||
        IsNullOrEmptyString(pszOrgUnit) ||
        !ppMachineInfo ||
        !ppKrbInfo ||
        !bPreJoined  /* prejoined is the only supported path now */
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirDomainDNToName(
                  pConn->AccessInfo.pszNormBindedDn,
                  &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
                  sizeof(VMDIR_MACHINE_INFO_A),
                  (PVOID*)&pMachineInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvGetComputerAccountInfo(
                  pConn,
                  pszDomainName,
                  pszMachineName,
                  &pMachineInfo->pszComputerDN,
                  &pMachineInfo->pszMachineGUID,
                  &pMachineInfo->pszSiteName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvGetKeyTabInfoClient(
                  pConn,
                  pszDomainName,
                  pszMachineName,
                  &pKrbInfo);
    /* still investigating if this is right but this is how the current code is setup */
    if (dwError == VMDIR_ERROR_ENTRY_NOT_FOUND)
    {
        dwError = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMachineInfo = pMachineInfo;
    *ppKrbInfo = pKrbInfo;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s error (%u)", __FUNCTION__, dwError);
    VmDirFreeMachineInfoA(pMachineInfo);
    VmDirFreeKrbInfo(pKrbInfo);
    goto cleanup;
}

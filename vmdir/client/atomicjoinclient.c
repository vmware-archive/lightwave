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


static
DWORD
_VmDirCopyFromRpcKrbInfo(
    PVMDIR_KRB_INFO  pKrbInfoIn,
    PVMDIR_KRB_INFO* ppKrbInfoOut
    );

static
DWORD
_VmDirCopyFromRpcMachineInfo(
    PVMDIR_MACHINE_INFO_W  pMachineInfoIn,
    PVMDIR_MACHINE_INFO_A* ppMachineInfoOut
    );


/*
 * APIs for HA Topology Management ends here
 */
DWORD
VmDirClientJoinAtomic(
    PCSTR                   pszServerName,
    PCSTR                   pszUserName,
    PCSTR                   pszPassword,
    PCSTR                   pszDomainName,
    PCSTR                   pszMachineName,
    PCSTR                   pszOrgUnit,
    DWORD                   dwFlags,
    PVMDIR_MACHINE_INFO_A   *ppMachineInfo
    )
{
    DWORD dwError = 0;
    PVMDIR_MACHINE_INFO_A  pMachineInfo = NULL;
    PVMDIR_MACHINE_INFO_W  pRpcMachineInfo = NULL;
    PVMDIR_KRB_INFO      pKrbInfo = NULL;
    PVMDIR_KRB_INFO      pRpcKrbInfo = NULL;
    handle_t hBinding = NULL;
    PSTR pszSRPUPN = NULL;
    PSTR pszSRPUPNAlloc = NULL;
    PWSTR pwszMachineName = NULL;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszOrgUnit = NULL;
    PWSTR pwszPassword = NULL;

    if (IsNullOrEmptyString(pszServerName) ||
        IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszMachineName) ||
        !ppMachineInfo
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringWFromA(
                              pszDomainName,
                              &pwszDomainName
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                              pszMachineName,
                              &pwszMachineName
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!IsNullOrEmptyString(pszOrgUnit))
    {
        dwError = VmDirAllocateStringWFromA(
                              pszOrgUnit,
                              &pwszOrgUnit
                              );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringWFromA(
                              pszPassword,
                              &pwszPassword
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!VmDirStringChrA(pszUserName, '@'))
    {
        dwError = VmDirAllocateStringPrintf(
                    &pszSRPUPNAlloc,
                    "%s@%s",
                    pszUserName,
                    pszDomainName
                    );
        BAIL_ON_VMDIR_ERROR(dwError);

        pszSRPUPN = pszSRPUPNAlloc;
    }
    else
    {
        pszSRPUPN = (PSTR)pszUserName;
    }

    dwError = VmDirCreateBindingHandleAuthA(
                    pszServerName,
                    NULL,
                    pszSRPUPN,
                    NULL,
                    pszPassword,
                    &hBinding);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RPC_TRY
    {

        if (dwFlags & VMDIR_CLIENT_JOIN_FLAGS_PREJOINED)
        {
            dwError = RpcVmDirGetComputerAccountInfo(
                        hBinding,
                        pwszDomainName,
                        pwszPassword,
                        pwszMachineName,
                        &pRpcMachineInfo,
                        &pRpcKrbInfo
                        );
        }
        else
        {
            dwError = RpcVmDirClientJoin(
                        hBinding,
                        pwszDomainName,
                        pwszMachineName,
                        pwszOrgUnit,
                        &pRpcMachineInfo,
                        &pRpcKrbInfo
                        );
        }
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pRpcKrbInfo)
    {
        dwError = _VmDirCopyFromRpcKrbInfo(
                          pRpcKrbInfo,
                          &pKrbInfo
                          );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirWriteToKeyTabFile(pKrbInfo);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirCopyFromRpcMachineInfo(
                              pRpcMachineInfo,
                              &pMachineInfo
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConfigSetDCAccountInfo(
                                    pszMachineName,
                                    pMachineInfo->pszComputerDN,
                                    pMachineInfo->pszPassword,
                                    strlen(pMachineInfo->pszPassword),
                                    pMachineInfo->pszMachineGUID
                                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMachineInfo = pMachineInfo;

cleanup:

    if (hBinding)
    {
        VmDirFreeBindingHandle(&hBinding);
    }
    if (pRpcKrbInfo)
    {
        VmDirClientRpcFreeKrbInfo(pRpcKrbInfo);
    }
    if (pRpcMachineInfo)
    {
        VmDirClientRpcFreeMachineInfoW(pRpcMachineInfo);
    }
    VMDIR_SAFE_FREE_MEMORY(pszSRPUPNAlloc);
    VMDIR_SAFE_FREE_MEMORY(pwszMachineName);
    VMDIR_SAFE_FREE_MEMORY(pwszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pwszOrgUnit);
    VMDIR_SAFE_FREE_MEMORY(pwszPassword);
    return dwError;
error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s error (%u)", __FUNCTION__, dwError);

    if (ppMachineInfo)
    {
        *ppMachineInfo = NULL;
    }
    if (pMachineInfo)
    {
        VmDirFreeMachineInfoA(pMachineInfo);
    }
    goto cleanup;
}

DWORD
VmDirCreateComputerAccountAtomic(
    PCSTR                   pszServerName,
    PCSTR                   pszSRPUPN,
    PCSTR                   pszPassword,
    PCSTR                   pszDomainName,
    PCSTR                   pszMachineName,
    PCSTR                   pszOrgUnit,
    PVMDIR_MACHINE_INFO_A   *ppMachineInfo
    )
{
    DWORD dwError = 0;
    PVMDIR_MACHINE_INFO_A  pMachineInfo = NULL;
    PVMDIR_MACHINE_INFO_W  pRpcMachineInfo = NULL;
    handle_t hBinding = NULL;
    PWSTR pwszMachineName = NULL;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszOrgUnit = NULL;
    PWSTR pwszPassword = NULL;

    if (IsNullOrEmptyString(pszServerName) ||
        IsNullOrEmptyString(pszSRPUPN) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszMachineName) ||
        !ppMachineInfo
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringWFromA(
                              pszDomainName,
                              &pwszDomainName
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                              pszMachineName,
                              &pwszMachineName
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!IsNullOrEmptyString(pszOrgUnit))
    {
        dwError = VmDirAllocateStringWFromA(
                              pszOrgUnit,
                              &pwszOrgUnit
                              );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringWFromA(
                              pszPassword,
                              &pwszPassword
                              );
    BAIL_ON_VMDIR_ERROR(dwError);


    dwError = VmDirCreateBindingHandleAuthA(
                    pszServerName,
                    NULL,
                    pszSRPUPN,
                    NULL,
                    pszPassword,
                    &hBinding);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RPC_TRY
    {

        dwError = RpcVmDirCreateComputerAccount(
                                            hBinding,
                                            pwszDomainName,
                                            pwszMachineName,
                                            pwszOrgUnit,
                                            &pRpcMachineInfo
                                            );
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirCopyFromRpcMachineInfo(
                              pRpcMachineInfo,
                              &pMachineInfo
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMachineInfo = pMachineInfo;

cleanup:

    if (hBinding)
    {
        VmDirFreeBindingHandle(&hBinding);
    }
    if (pRpcMachineInfo)
    {
        VmDirClientRpcFreeMachineInfoW(pRpcMachineInfo);
    }
    VMDIR_SAFE_FREE_MEMORY(pwszMachineName);
    VMDIR_SAFE_FREE_MEMORY(pwszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pwszOrgUnit);
    VMDIR_SAFE_FREE_MEMORY(pwszPassword);
    return dwError;
error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s error (%u)", __FUNCTION__, dwError);
    if (ppMachineInfo)
    {
        *ppMachineInfo = NULL;
    }
    if (pMachineInfo)
    {
        VmDirFreeMachineInfoA(pMachineInfo);
    }
    goto cleanup;
}

VOID
VmDirClientFreeMachineInfo(
    PVMDIR_MACHINE_INFO_A pMachineInfo
    )
{
    if (pMachineInfo)
    {
        VmDirFreeMachineInfoA(pMachineInfo);
    }
}

static
DWORD
_VmDirCopyFromRpcMachineInfo(
    PVMDIR_MACHINE_INFO_W  pMachineInfoIn,
    PVMDIR_MACHINE_INFO_A* ppMachineInfoOut
    )
{
    DWORD dwError = 0;
    PVMDIR_MACHINE_INFO_A pMachineInfo = NULL;

    if (!pMachineInfoIn || !ppMachineInfoOut)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                          sizeof(VMDIR_MACHINE_INFO_A),
                          (PVOID*)&pMachineInfo
                          );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                          pMachineInfoIn->pwszPassword,
                          &pMachineInfo->pszPassword
                          );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                          pMachineInfoIn->pwszSiteName,
                          &pMachineInfo->pszSiteName
                          );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                          pMachineInfoIn->pwszComputerDN,
                          &pMachineInfo->pszComputerDN
                          );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                          pMachineInfoIn->pwszMachineGUID,
                          &pMachineInfo->pszMachineGUID
                          );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMachineInfoOut = pMachineInfo;

cleanup:

    return dwError;
error:

    if (ppMachineInfoOut)
    {
        *ppMachineInfoOut = NULL;
    }
    if (pMachineInfo)
    {
        VmDirFreeMachineInfoA(pMachineInfo);
    }
    goto cleanup;
}

static
DWORD
_VmDirCopyFromRpcKrbInfo(
    PVMDIR_KRB_INFO  pKrbInfoIn,
    PVMDIR_KRB_INFO* ppKrbInfoOut
    )
{
    DWORD dwError = 0;
    PVMDIR_KRB_INFO pKrbInfo = NULL;

    if (!pKrbInfoIn || !ppKrbInfoOut)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                              sizeof(VMDIR_KRB_INFO),
                              (PVOID*)&pKrbInfo
                              );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pKrbInfoIn->dwCount)
    {
        DWORD dwIndex = 0;
        dwError = VmDirAllocateMemory(
                                  sizeof(VMDIR_KRB_BLOB)*pKrbInfoIn->dwCount,
                                  (PVOID)&pKrbInfo->pKrbBlobs
                                  );
        BAIL_ON_VMDIR_ERROR(dwError);


        for (; dwIndex < pKrbInfoIn->dwCount; ++dwIndex)
        {
            VMDIR_KRB_BLOB pCursorIn = pKrbInfoIn->pKrbBlobs[dwIndex];
            VMDIR_KRB_BLOB pCursor = pKrbInfo->pKrbBlobs[dwIndex];

            if (pCursorIn.dwCount)
            {
                dwError = VmDirAllocateMemory(
                                      pCursorIn.dwCount,
                                      (PVOID*)&pCursor.krbBlob
                                      );
                BAIL_ON_VMDIR_ERROR(dwError);

                pCursor.dwCount = pCursorIn.dwCount;

                dwError = VmDirCopyMemory(
                                      pCursor.krbBlob,
                                      pCursor.dwCount,
                                      pCursorIn.krbBlob,
                                      pCursor.dwCount
                                      );
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        pKrbInfo->dwCount = dwIndex;
    }

    *ppKrbInfoOut = pKrbInfo;
cleanup:

    return dwError;
error:

    if (ppKrbInfoOut)
    {
        *ppKrbInfoOut = NULL;
    }
    if (pKrbInfo)
    {
        VmDirFreeKrbInfo(pKrbInfo);
    }
    goto cleanup;
}

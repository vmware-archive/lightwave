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
VmDirCreateDomainTrust(
    PCSTR pszServerName,
    PCSTR pszTrustName,
    PCSTR pszDomainName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PCSTR pszTrustPasswordIn,
    PCSTR pszTrustPasswordOut
    )
{
    DWORD dwError = 0;
    PCSTR pszServerEndpoint = NULL;
    PWSTR pwszTrustName = NULL;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszTrustPasswordIn = NULL;
    PWSTR pwszTrustPasswordOut = NULL;
    PWSTR pwszUPNName = NULL;
    handle_t hBinding = NULL;

    if (
         IsNullOrEmptyString(pszServerName)
      || IsNullOrEmptyString(pszTrustName)
      || IsNullOrEmptyString(pszDomainName)
      || IsNullOrEmptyString(pszUserName)
      || IsNullOrEmptyString(pszPassword)
      || IsNullOrEmptyString(pszTrustPasswordIn)
      || IsNullOrEmptyString(pszTrustPasswordOut)
       )
    {
        dwError =  ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirCreateBindingHandleAuthA(
                    pszServerName,
                    pszServerEndpoint,
                    pszUserName,
                    pszDomainName,
                    pszPassword,
                    &hBinding
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                    pszTrustName,
                    &pwszTrustName
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                    pszDomainName,
                    &pwszDomainName
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                    pszTrustPasswordIn,
                    &pwszTrustPasswordIn
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                    pszTrustPasswordOut,
                    &pwszTrustPasswordOut
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirCreateDomainTrust(
                        hBinding,
                        pwszTrustName,
                        pwszDomainName,
                        pwszTrustPasswordIn,
                        pwszTrustPasswordOut
                        );
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pwszTrustName);
    VMDIR_SAFE_FREE_MEMORY(pwszPassword);
    VMDIR_SAFE_FREE_MEMORY(pwszUPNName);
    VMDIR_SAFE_FREE_MEMORY(pwszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pwszTrustPasswordIn);
    VMDIR_SAFE_FREE_MEMORY(pwszTrustPasswordOut);
    if (hBinding)
    {
        VmDirFreeBindingHandle(&hBinding);
    }
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirAllocateTrustInfoA(
    PCSTR   pszName,
    PCSTR   pszDC,
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    PVMDIR_TRUST_INFO_A *ppTrustInfoA)
{
    DWORD dwError = 0;
    PVMDIR_TRUST_INFO_A pTrustInfoA = NULL;

    if (!pszName ||
        !pszDC ||
        !pszUserName ||
        !pszPassword)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                  sizeof(VMDIR_TRUST_INFO_A),
                  (PVOID*)&pTrustInfoA);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                  pszName,
                  &pTrustInfoA->pszName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                  pszDC,
                  &pTrustInfoA->pszDC);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                  pszUserName,
                  &pTrustInfoA->pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                  pszPassword,
                  &pTrustInfoA->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppTrustInfoA = pTrustInfoA;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_TRUST_INFO_A(pTrustInfoA);
    goto cleanup;
}

DWORD
VmDirAllocateTrustInfoW(
    PCWSTR   pwszName,
    PCWSTR   pwszDC,
    PCWSTR   pwszUserName,
    PCWSTR   pwszPassword,
    PVMDIR_TRUST_INFO_W *ppTrustInfoW)
{
    DWORD dwError = 0;
    PVMDIR_TRUST_INFO_W pTrustInfoW = NULL;

    if (!pwszName ||
        !pwszDC ||
        !pwszUserName ||
        !pwszPassword)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                  sizeof(VMDIR_TRUST_INFO_W),
                  (PVOID*)&pTrustInfoW);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringW(
                  pwszName,
                  &pTrustInfoW->pwszName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringW(
                  pwszDC,
                  &pTrustInfoW->pwszDC);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringW(
                  pwszUserName,
                  &pTrustInfoW->pwszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringW(
                  pwszPassword,
                  &pTrustInfoW->pwszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppTrustInfoW = pTrustInfoW;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_TRUST_INFO_W(pTrustInfoW);
    goto cleanup;
}

DWORD
VmDirAllocateTrustInfoAFromW(
    PVMDIR_TRUST_INFO_W pTrustInfoW,
    PVMDIR_TRUST_INFO_A *ppTrustInfoA
    )
{
    DWORD dwError = 0;
    PVMDIR_TRUST_INFO_A pTrustInfoA = NULL;

    dwError = VmDirAllocateMemory(
                  sizeof(VMDIR_TRUST_INFO_A),
                  (PVOID*)&pTrustInfoA);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                  pTrustInfoW->pwszName,
                  &pTrustInfoA->pszName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                  pTrustInfoW->pwszDC,
                  &pTrustInfoA->pszDC);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                  pTrustInfoW->pwszUserName,
                  &pTrustInfoA->pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(
                  pTrustInfoW->pwszPassword,
                  &pTrustInfoA->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppTrustInfoA = pTrustInfoA;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_TRUST_INFO_A(pTrustInfoA);
    goto cleanup;
}

DWORD
VmDirAllocateTrustInfoWFromA(
    PVMDIR_TRUST_INFO_A pTrustInfoA,
    PVMDIR_TRUST_INFO_W *ppTrustInfoW
    )
{
    DWORD dwError = 0;
    PVMDIR_TRUST_INFO_W pTrustInfoW = NULL;

    dwError = VmDirAllocateMemory(
                  sizeof(VMDIR_TRUST_INFO_W),
                  (PVOID*)&pTrustInfoW);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                  pTrustInfoA->pszName,
                  &pTrustInfoW->pwszName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                  pTrustInfoA->pszDC,
                  &pTrustInfoW->pwszDC);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                  pTrustInfoA->pszUserName,
                  &pTrustInfoW->pwszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                  pTrustInfoA->pszPassword,
                  &pTrustInfoW->pwszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppTrustInfoW = pTrustInfoW;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_TRUST_INFO_W(pTrustInfoW);
    goto cleanup;
}

VOID
VmDirFreeTrustInfoA(
    PVMDIR_TRUST_INFO_A pTrustInfoA)
{
    if (pTrustInfoA)
    {
        VMDIR_SAFE_FREE_STRINGA(pTrustInfoA->pszName);
        VMDIR_SAFE_FREE_STRINGA(pTrustInfoA->pszDC);
        VMDIR_SAFE_FREE_STRINGA(pTrustInfoA->pszUserName);
        VMDIR_SAFE_FREE_STRINGA(pTrustInfoA->pszPassword);
        VMDIR_SAFE_FREE_MEMORY(pTrustInfoA);
    }
}

VOID
VmDirFreeTrustInfoW(
    PVMDIR_TRUST_INFO_W pTrustInfoW)
{
    if (pTrustInfoW)
    {
        VMDIR_SAFE_FREE_MEMORY(pTrustInfoW->pwszName);
        VMDIR_SAFE_FREE_MEMORY(pTrustInfoW->pwszDC);
        VMDIR_SAFE_FREE_MEMORY(pTrustInfoW->pwszUserName);
        VMDIR_SAFE_FREE_MEMORY(pTrustInfoW->pwszPassword);
        VMDIR_SAFE_FREE_MEMORY(pTrustInfoW);
    }
}

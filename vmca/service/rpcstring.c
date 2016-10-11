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



/*
 * Module Name: ThinAppVMCAService
 *
 * Filename: rpcstring.c
 *
 * Abstract:
 *
 * RPC Server Side String Allocation
 *
 */

#include "includes.h"

DWORD
VMCARpcAllocateStringW(
    RP_PWSTR  pwszSrc,
    RP_PWSTR* ppwszDst
    )
{
    DWORD dwError = 0;
    size_t len = 0;
    RP_PWSTR pwszDst = NULL;

    if (!pwszSrc || !ppwszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = LwWc16sLen(pwszSrc, &len);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARpcAllocateMemory(
                    sizeof(RP_WSTR) * (len + 1),
                    (PVOID*)&pwszDst);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = LwWc16snCpy(pwszDst, pwszSrc, len);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppwszDst = pwszDst;

cleanup:

    return dwError;

error:

    if (ppwszDst)
    {
        *ppwszDst = NULL;
    }

    if (pwszDst)
    {
        VMCARpcFreeMemory(pwszDst);
    }

    goto cleanup;
}

DWORD
VMCARpcAllocateUnicodeStringFromAnsi(
    RP_PCSTR  pszSrc,
    RP_PWSTR* ppwszDst
    )
{
    DWORD dwError = 0;
    RP_PWSTR pwszTmp = NULL;
    RP_PWSTR pwszDst = NULL;

    dwError = LwMbsToWc16s(pszSrc, &pwszTmp);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARpcAllocateStringW(pwszTmp, &pwszDst);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppwszDst = pwszDst;

cleanup:

    if (pwszTmp)
    {
        LwFreeMemory(pwszTmp);
    }

    return dwError;

error:

    *ppwszDst = NULL;

    if (pwszDst)
    {
        VMCARpcFreeMemory(pwszDst);
    }

    goto cleanup;
}

/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : rpcstring.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Service
 *
 *            RPC Server String Management
 *
 * Authors  : Sriram Nambakam (snambakam@vmware.com)
 *
 *
 */

#include "includes.h"

DWORD
VmDnsRpcAllocateStringW(
    PWSTR  pwszSrc,
    PWSTR* ppwszDst
    )
{
    DWORD  dwError = 0;
    size_t len = 0;
    PWSTR  pwszDst = NULL;

    if (!pwszSrc || !ppwszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsGetStringLengthW(pwszSrc, &len);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcAllocateMemory(
                    sizeof(WCHAR) * (len + 1),
                    (PVOID*)&pwszDst);
    BAIL_ON_VMDNS_ERROR(dwError);

    memcpy((PBYTE)pwszDst, (PBYTE)pwszSrc, sizeof(WCHAR) * len);

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
        VmDnsRpcFreeMemory(pwszDst);
    }

    goto cleanup;
}


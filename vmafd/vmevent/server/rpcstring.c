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



/*
 * Module Name: EventLogService
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
EventLogRpcAllocateStringW(
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
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

    dwError = EventLogGetStringLengthW(pwszSrc, &len);
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = EventLogRpcAllocateMemory(
                    sizeof(RP_WSTR) * (len + 1),
                    (PVOID*)&pwszDst);
    BAIL_ON_VMEVENT_ERROR(dwError);

    memcpy((PBYTE)pwszDst, (PBYTE)pwszSrc, sizeof(RP_WSTR) * len);

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
        EventLogRpcFreeMemory(pwszDst);
    }

    goto cleanup;
}


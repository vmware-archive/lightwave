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

static
DWORD
_VmAfdAllocateSuperLogEntryArray(
        PVMAFD_SUPERLOG_ENTRY_ARRAY pSrcEntries,
        PVMAFD_SUPERLOG_ENTRY_ARRAY *ppDstEntries
        );

// Super Logger Client API

DWORD
VmAfdSuperLogEnable(
    PVMAFD_SERVER pServer
    )
{
    DWORD dwError = 0;

    if(!pServer || !pServer->hBinding)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
       dwError = RpcVmAfdSuperLogEnable(pServer->hBinding);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdIsSuperLogEnabled(
    PVMAFD_SERVER       pServer,
    PBOOLEAN pbEnabled
    )
{
    DWORD           dwError = 0;
    UINT32          dwEnable = 0;

    if(!pServer || !pServer->hBinding)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
       dwError = RpcVmAfdIsSuperLogEnabled(pServer->hBinding, &dwEnable);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAFD_ERROR(dwError);

    *pbEnabled = (dwEnable == 1 ? TRUE : FALSE);

cleanup:
    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdSuperLogGetSize(
    PVMAFD_SERVER    pServer,
    PDWORD    pdwSize
    )
{
    DWORD           dwError = 0;

    if(!pServer || !pServer->hBinding)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
       dwError = RpcVmAfdSuperLogGetSize(pServer->hBinding, pdwSize);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}


DWORD
VmAfdSuperLogSetSize(
    PVMAFD_SERVER    pServer,
    DWORD    dwSize
    )
{
    DWORD dwError = 0;

    if(!pServer || !pServer->hBinding)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
       dwError = RpcVmAfdSuperLogSetSize(pServer->hBinding, dwSize);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdSuperLogDisable(
    PVMAFD_SERVER    pServer
    )
{
    DWORD           dwError = 0;

    if(!pServer || !pServer->hBinding)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
       dwError = RpcVmAfdSuperLogDisable(pServer->hBinding);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdSuperLogGetEntries(
    PVMAFD_SERVER       pServer,
    UINT32 **ppEnumerationCookie,
    DWORD  dwCount, // 0 ==> all
    PVMAFD_SUPERLOG_ENTRY_ARRAY *ppEntries
    )
{
    DWORD           dwError = 0;
    PVMAFD_SUPERLOG_ENTRY_ARRAY pEntries = NULL;
    PVMAFD_SUPERLOG_ENTRY_ARRAY pRpcEntries = NULL;

    if(!pServer || !pServer->hBinding)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
        dwError = RpcVmAfdSuperLogGetEntries(
                          pServer->hBinding,
                          (vmafd_superlog_cookie_t *)ppEnumerationCookie,
                          dwCount,
                          &pRpcEntries);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _VmAfdAllocateSuperLogEntryArray(pRpcEntries, &pEntries);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppEntries = pEntries;

cleanup:
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pEntries);
    goto cleanup;
}

DWORD
VmAfdClearSuperLog(
    PVMAFD_SERVER       pServer
    )
{
    DWORD           dwError = 0;

    if(!pServer || !pServer->hBinding)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
       dwError = RpcVmAfdClearSuperLog(pServer->hBinding);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmAfdRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:

    goto cleanup;
}
static
DWORD
_VmAfdAllocateSuperLogEntryArray(
        PVMAFD_SUPERLOG_ENTRY_ARRAY pSrcEntries,
        PVMAFD_SUPERLOG_ENTRY_ARRAY *ppDstEntries
        )
{
    DWORD   dwError = 0;
    PVMAFD_SUPERLOG_ENTRY srcEntries = NULL;
    PVMAFD_SUPERLOG_ENTRY dstEntries = NULL;
    PVMAFD_SUPERLOG_ENTRY_ARRAY pDstEntries = NULL;
    unsigned int i;

    if (!pSrcEntries || !ppDstEntries)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
            sizeof(VMAFD_SUPERLOG_ENTRY_ARRAY),
            (PVOID*)&pDstEntries
            );
    BAIL_ON_VMAFD_ERROR(dwError);

    pDstEntries->dwCount = 0;
    pDstEntries->entries = NULL;

    if (pSrcEntries->dwCount > 0)
    {
        dwError = VmAfdAllocateMemory(
                sizeof(VMAFD_SUPERLOG_ENTRY_ARRAY)*pSrcEntries->dwCount,
                (PVOID*)&pDstEntries->entries
                );
        BAIL_ON_VMAFD_ERROR(dwError);

        pDstEntries->dwCount = pSrcEntries->dwCount;
        srcEntries = pSrcEntries->entries;
        dstEntries = pDstEntries->entries;

        for (i = 0; i < pDstEntries->dwCount; i++)
        {
            dwError = VmAfdStringCpyA(dstEntries[i].pszDomainName, VMAFD_MAX_DN_LEN, srcEntries[i].pszDomainName);
            BAIL_ON_VMAFD_ERROR(dwError);
            dwError = VmAfdStringCpyA(dstEntries[i].pszSiteName, VMAFD_MAX_DN_LEN, srcEntries[i].pszSiteName);
            BAIL_ON_VMAFD_ERROR(dwError);
            dwError = VmAfdStringCpyA(dstEntries[i].pszDCName, VMAFD_MAX_DN_LEN, srcEntries[i].pszDCName);
            BAIL_ON_VMAFD_ERROR(dwError);

            dstEntries[i].dwState = srcEntries[i].dwState;
            dstEntries[i].dwCDCPingTime = srcEntries[i].dwCDCPingTime;
            dstEntries[i].dwCDCLastPing = srcEntries[i].dwCDCLastPing;
            dstEntries[i].bCDCIsAlive = srcEntries[i].bCDCIsAlive;
            dstEntries[i].bHBIsAlive = srcEntries[i].bHBIsAlive;
            dstEntries[i].dwHBCount = srcEntries[i].dwHBCount;
            dstEntries[i].dwErrorCode = srcEntries[i].dwErrorCode;
            dstEntries[i].iStartTime = srcEntries[i].iStartTime;
            dstEntries[i].iEndTime = srcEntries[i].iEndTime;
        }
    }

    *ppDstEntries = pDstEntries;

cleanup:
    return dwError;

error:
    goto cleanup;
}

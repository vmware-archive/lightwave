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
CreateSuperlogWrapper(
        PSTR pszNetworkAddress,
        PSTR pszDomain,
        PSTR pszUserName,
        PSTR pszPassword,
        PSUPERLOG_WRAPPER *ppSuperlogWrapper
        )
{
    DWORD dwError = 0;
    PSUPERLOG_WRAPPER pSuperlogWrapper = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(SUPERLOG_WRAPPER),
            (PVOID*)&pSuperlogWrapper
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    pSuperlogWrapper->pszNetworkAddress = NULL;
    pSuperlogWrapper->pszDomain = NULL;
    pSuperlogWrapper->pServerContext = NULL;

    // If network address and domain are not provided, leave
    // the server context NULL so that rpc client will connect
    // to the localhost
    if (pszNetworkAddress != NULL && pszDomain != NULL)
    {
        dwError = VmDirAllocateStringA(pszNetworkAddress, &pSuperlogWrapper->pszNetworkAddress);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pszDomain, &pSuperlogWrapper->pszDomain);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirOpenServerA(
                pszNetworkAddress,
                pszUserName,
                pszDomain,
                pszPassword,
                0,
                NULL,
                &pSuperlogWrapper->pServerContext
                );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppSuperlogWrapper = pSuperlogWrapper;

cleanup:
    return dwError;

error:
    FreeSuperlogWrapper(pSuperlogWrapper);
    goto cleanup;
}

DWORD
GetSuperlogServerData(
        PSUPERLOG_WRAPPER pSuperlogWrapper,
        PVMDIR_SUPERLOG_SERVER_DATA *ppServerData
        )
{
    DWORD dwError = 0;
    PVMDIR_SUPERLOG_SERVER_DATA pServerData = NULL;

    dwError = VmDirSuperLogQueryServerData(
            pSuperlogWrapper->pServerContext,
            &pServerData
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppServerData = pServerData;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pServerData);
    goto cleanup;
}

DWORD
EnableSuperlog(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        )
{
    DWORD dwError = 0;

    dwError = VmDirSuperLogEnable(pSuperlogWrapper->pServerContext);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
IsSuperlogEnabled(
        PSUPERLOG_WRAPPER pSuperlogWrapper,
        PBOOLEAN pbEnabled
        )
{
    DWORD dwError = 0;
    BOOLEAN bEnabled = FALSE;

    dwError = VmDirIsSuperLogEnabled(pSuperlogWrapper->pServerContext, &bEnabled);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pbEnabled = bEnabled;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
DisableSuperlog(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        )
{
    DWORD dwError = 0;

    dwError = VmDirSuperLogDisable(pSuperlogWrapper->pServerContext);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
SetSuperlogBufferSize(
        PSUPERLOG_WRAPPER pSuperlogWrapper,
        DWORD dwSize
        )
{
    DWORD dwError = 0;

    dwError = VmDirSuperLogSetSize(pSuperlogWrapper->pServerContext, dwSize);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
GetSuperlogBufferSize(
        PSUPERLOG_WRAPPER pSuperlogWrapper,
        PDWORD pdwSize
        )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;

    dwError = VmDirSuperLogGetSize(pSuperlogWrapper->pServerContext, &dwSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwSize = dwSize;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
RetrieveSuperlogEntries(
        PSUPERLOG_WRAPPER pSuperlogWrapper,
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY *ppEntries,
        UINT64 **ppEnumerationCookie,
        DWORD dwCount,
        BOOLEAN flush
        )
{
    DWORD dwError = 0;
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pEntries = NULL;

    dwError = VmDirSuperLogGetEntriesLdapOperation(
            pSuperlogWrapper->pServerContext,
            ppEnumerationCookie,
            dwCount,
            &pEntries
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (flush)
    {
        dwError = FlushSuperlogBuffer(pSuperlogWrapper);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppEntries = pEntries;

cleanup:
    return dwError;

error:
    VmDirFreeSuperLogEntryLdapOperationArray(pEntries);
    goto cleanup;
}

DWORD
FlushSuperlogBuffer(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        )
{
    DWORD dwError = 0;

    dwError = VmDirSuperLogFlush(pSuperlogWrapper->pServerContext);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
FreeSuperlogWrapper(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        )
{
    if (pSuperlogWrapper)
    {
        if (pSuperlogWrapper->pszNetworkAddress)
        {
            VMDIR_SAFE_FREE_MEMORY(pSuperlogWrapper->pszNetworkAddress);
            pSuperlogWrapper->pszNetworkAddress = NULL;
        }
        if (pSuperlogWrapper->pszDomain)
        {
            VMDIR_SAFE_FREE_MEMORY(pSuperlogWrapper->pszDomain);
            pSuperlogWrapper->pszDomain = NULL;
        }
        if (pSuperlogWrapper->pServerContext)
        {
            // TODO VmDirFreeBindingHandle(&g_pServerContext->hBinding);
            VMDIR_SAFE_FREE_MEMORY(pSuperlogWrapper->pServerContext);
            pSuperlogWrapper->pServerContext = NULL;
        }
        VMDIR_SAFE_FREE_MEMORY(pSuperlogWrapper);
        pSuperlogWrapper = NULL;
    }
}

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
__VmAfdSuperLogGetEntries(
        PVMAFD_SUPERLOG_ENTRY_ARRAY *ppEntries
);

static DWORD dwCounter = 0;

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

    dwError = VmAfdAllocateMemory(
            sizeof(SUPERLOG_WRAPPER),
            (PVOID*)&pSuperlogWrapper
            );
    BAIL_ON_VMAFD_ERROR(dwError);

    pSuperlogWrapper->pszNetworkAddress = NULL;
    pSuperlogWrapper->pszDomain = NULL;
    pSuperlogWrapper->pServerContext = NULL;

    // If network address and domain are not provided, leave
    // the server context NULL so that rpc client will connect
    // to the localhost
    dwError = VmAfdAllocateStringA(pszNetworkAddress, &pSuperlogWrapper->pszNetworkAddress);
    BAIL_ON_VMAFD_ERROR(dwError);

/*
    dwError = VmAfdAllocateStringA(pszDomain, &pSuperlogWrapper->pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);
*/
    if(!strcmp(pszNetworkAddress, "localhost"))
    {
        dwError = VmAfdOpenServerA(
            NULL,
            NULL,
            NULL,
            &(pSuperlogWrapper->pServer));
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdOpenServerA(
            pszNetworkAddress,
            pszUserName,
            pszPassword,
            &(pSuperlogWrapper->pServer));
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppSuperlogWrapper = pSuperlogWrapper;

cleanup:
    return dwError;

error:
    FreeSuperlogWrapper(pSuperlogWrapper);
    goto cleanup;
}

DWORD
EnableSuperlog(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        )
{
    DWORD dwError = 0;
    dwError = VmAfdSuperLogEnable(pSuperlogWrapper->pServer);

    return dwError;
}

DWORD
IsSuperlogEnabled(
        PSUPERLOG_WRAPPER pSuperlogWrapper,
        PBOOLEAN pbEnabled
        )
{
    DWORD dwError = 0;
    BOOLEAN bEnabled = FALSE;

    dwError = VmAfdIsSuperLogEnabled(pSuperlogWrapper->pServer, &bEnabled);
    *pbEnabled = bEnabled;

    return dwError;
}

DWORD
DisableSuperlog(
      PSUPERLOG_WRAPPER pSuperlogWrapper
      )
{
    DWORD dwError = 0;
    dwError = VmAfdSuperLogDisable(pSuperlogWrapper->pServer);
    BAIL_ON_VMAFD_ERROR(dwError);

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
    dwError = VmAfdSuperLogSetSize(pSuperlogWrapper->pServer, dwSize);
    BAIL_ON_VMAFD_ERROR(dwError);

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

    printf("\n%s\n", "superlogWrapper::GetSuperlogBufferSize");
    dwError = VmAfdSuperLogGetSize(pSuperlogWrapper->pServer, &dwSize);
    *pdwSize = dwSize;

    return dwError;
}


DWORD
RetrieveSuperlogEntries(
        PSUPERLOG_WRAPPER pSuperlogWrapper,
        PVMAFD_SUPERLOG_ENTRY_ARRAY *ppEntries,
        UINT32 **ppEnumerationCookie,
        DWORD dwCount,
        BOOLEAN clean
        )
{
    DWORD dwError = 0;
    PVMAFD_SUPERLOG_ENTRY_ARRAY pEntries = NULL;

    printf("%s\n", "Retrieve superlog entries.");

    dwError = VmAfdSuperLogGetEntries(
            pSuperlogWrapper->pServer,
            ppEnumerationCookie,
            dwCount,
            &pEntries
            );

    if(dwError == 0)
    {
        printf("%s %d\n", "Success: retrieved log entries: ", pEntries->dwCount);
    }
    else
    {
        printf("%s %d\n", "Error: failed to get log entries: ", dwError);
        dwError = __VmAfdSuperLogGetEntries(&pEntries);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (clean)
    {
        dwError = ClearSuperlogBuffer(pSuperlogWrapper);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppEntries = pEntries;

    printf("Retrieved super logging entries. Number of entries: %d\n", pEntries->dwCount);

cleanup:
    return dwError;

error:
    goto cleanup;
}


DWORD
ClearSuperlogBuffer(
        PSUPERLOG_WRAPPER pSuperlogWrapper
        )
{
    DWORD dwError = 0;

    dwError = VmAfdClearSuperLog(pSuperlogWrapper->pServer);

    return dwError;
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
            VMAFD_SAFE_FREE_MEMORY(pSuperlogWrapper->pszNetworkAddress);
            pSuperlogWrapper->pszNetworkAddress = NULL;
        }

        if (pSuperlogWrapper->pszDomain)
        {
            VMAFD_SAFE_FREE_MEMORY(pSuperlogWrapper->pszDomain);
            pSuperlogWrapper->pszDomain = NULL;
        }

        if (pSuperlogWrapper->pServerContext)
        {
            VMAFD_SAFE_FREE_MEMORY(pSuperlogWrapper->pServerContext);
            pSuperlogWrapper->pServerContext = NULL;
        }

        VMAFD_SAFE_FREE_MEMORY(pSuperlogWrapper);
        pSuperlogWrapper = NULL;
    }
}

static
DWORD
__VmAfdSuperLogGetEntries(
    PVMAFD_SUPERLOG_ENTRY_ARRAY *ppEntries
)
{
    DWORD dwError = 0;
    PVMAFD_SUPERLOG_ENTRY_ARRAY pCollection = NULL;
    DWORD i;


    dwError = VmAfdAllocateMemory(sizeof(VMAFD_SUPERLOG_ENTRY_ARRAY), (PVOID*)&pCollection);
    BAIL_ON_VMAFD_ERROR(dwError);

    pCollection->entries = NULL;
    dwCounter = 12;

    dwError = VmAfdAllocateMemory(sizeof(VMAFD_SUPERLOG_ENTRY) * dwCounter, (PVOID*)&pCollection->entries);
    pCollection->dwCount = dwCounter;
    BAIL_ON_VMAFD_ERROR(dwError);

    for(i = 0; i < dwCounter; i++)
    {
        dwError = VmAfdStringCpyA((PSTR)&pCollection->entries[i].pszDCName, VMAFD_MAX_DN_LEN, "sc-rdops-vm20-dhcp-114-97.eng.vmware.com");
        BAIL_ON_VMAFD_ERROR(dwError);

/*
        dwError = VmAfdStringCpyA((PSTR)&row->pszDomainName, VMAFD_MAX_DN_LEN, );
        BAIL_ON_VMAFD_ERROR(dwError);
*/
        dwError = VmAfdStringCpyA((PSTR)&pCollection->entries[i].pszDCAddress, VMAFD_MAX_DN_LEN, "10.162.114.97");
        BAIL_ON_VMAFD_ERROR(dwError);
/*
        dwError = VmAfdStringCpyA((PSTR)&row->pszSiteName, VMAFD_MAX_DN_LEN, Entry->pszSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);
*/
        pCollection->entries[i].bCDCIsAlive = 1;
        pCollection->entries[i].bHBIsAlive = 1;
        pCollection->entries[i].dwCDCLastPing = 589894859 + 1081;
        pCollection->entries[i].dwCDCPingTime = 589894849 + 1074;
        pCollection->entries[i].dwHBCount = 38 + i;
        pCollection->entries[i].dwState = 5 % (i + 1);
        pCollection->entries[i].dwErrorCode = 0;
        pCollection->entries[i].iStartTime = 589893849 + 1095;
        pCollection->entries[i].iEndTime = 589894849 + 1044;

    }

    *ppEntries = pCollection;

cleanup:
    return dwError;

error:
    //VmAfdFreeSuperLogEntryArray(pCollection);
    goto cleanup;
}

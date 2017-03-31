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

VOID
VdcStringListFree(
    PSTRING_LIST pStringList
    )
{
    DWORD i = 0;

    if (pStringList != NULL)
    {
        for (i = 0; i < pStringList->dwCount; ++i)
        {
            VmDirFreeStringA(pStringList->pStringList[i]);
        }

        pStringList->pStringList = NULL;
        pStringList->dwCount = 0;
    }
}

DWORD
VdcStringListInitialize(
    PSTRING_LIST *ppStringList,
    DWORD dwInitialCount
    )
{
    DWORD dwError = 0;
    PSTRING_LIST pStringList = NULL;
    size_t sAllocationSize = 0;

    dwError = VmDirAllocateMemory(sizeof(*pStringList), (PVOID *)&pStringList);
    BAIL_ON_VMDIR_ERROR(dwError);

    sAllocationSize = dwInitialCount * sizeof(PSTR);
    if (sAllocationSize < dwInitialCount)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(sAllocationSize, (PVOID *)&pStringList->pStringList);
    BAIL_ON_VMDIR_ERROR(dwError);

    pStringList->dwCount = 0;
    pStringList->dwSize = dwInitialCount;

    *ppStringList = pStringList;

cleanup:
    return dwError;

error:
    VdcStringListFree(pStringList);
    goto cleanup;
}

DWORD
VdcStringListAdd(
    PSTRING_LIST pStringList,
    PCSTR pszString
    )
{
    DWORD dwError = 0;

    if (pStringList->dwCount == pStringList->dwSize)
    {
        size_t iOldSize = pStringList->dwCount;
        size_t iNewSize = pStringList->dwSize * 2;

        //
        // Check for overflow.
        //
        if (iNewSize < pStringList->dwSize)
        {
            dwError = VMDIR_ERROR_SIZELIMIT_EXCEEDED;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirReallocateMemoryWithInit(
                    pStringList->pStringList,
                    (PVOID*)&pStringList->pStringList,
                    (iNewSize + 1) * sizeof(PSTR),
                    iOldSize * sizeof(PSTR));
        BAIL_ON_VMDIR_ERROR(dwError);

        pStringList->dwSize = iNewSize;
    }

    pStringList->pStringList[pStringList->dwCount++] = (PSTR)pszString;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VdcStringListRemove(
    PSTRING_LIST pStringList,
    PCSTR pszString
    )
{
    DWORD i = 0;

    for (i = 0; i < pStringList->dwCount; ++i)
    {
        if (strcmp(pStringList->pStringList[i], pszString) == 0)
        {
            memmove(&pStringList->pStringList[i],
                    &pStringList->pStringList[i + 1],
                    pStringList->dwCount - i - 1);
            pStringList->dwCount -= 1;
        }
    }

    //
    // Specified string not found.
    //
    return VMDIR_ERROR_INVALID_PARAMETER;
}

BOOLEAN
VdcStringListContains(
    PSTRING_LIST pStringList,
    PCSTR pszString
    )
{
    DWORD i = 0;

    for (i = 0; i < pStringList->dwCount; ++i)
    {
        if (strcmp(pStringList->pStringList[i], pszString) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

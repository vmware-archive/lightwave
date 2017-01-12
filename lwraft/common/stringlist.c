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
VmDirStringListFreeContent(
    PVMDIR_STRING_LIST pStringList
    )
{
    DWORD i = 0;

    if (pStringList != NULL)
    {
        for (i = 0; i < pStringList->dwCount; ++i)
        {
            VmDirFreeStringA((PSTR)pStringList->pStringList[i]);
        }
        pStringList->dwCount = 0;
    }
}

VOID
VmDirStringListFree(
    PVMDIR_STRING_LIST pStringList
    )
{
    if (pStringList != NULL)
    {
        VmDirStringListFreeContent(pStringList);
        VMDIR_SAFE_FREE_MEMORY(pStringList->pStringList);
        VmDirFreeMemory(pStringList);
    }
}

DWORD
VmDirStringListInitialize(
    PVMDIR_STRING_LIST *ppStringList,
    DWORD dwInitialCount
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pStringList = NULL;
    DWORD dwAllocationSize = 0;

    if (dwInitialCount < 4)
    {
        dwInitialCount = 4; // default to 4
    }

    dwAllocationSize = dwInitialCount * sizeof(PSTR);
    if (dwAllocationSize < dwInitialCount)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(*pStringList), (PVOID *)&pStringList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
                dwAllocationSize,
                (PVOID *)&pStringList->pStringList);
    BAIL_ON_VMDIR_ERROR(dwError);

    pStringList->dwCount = 0;
    pStringList->dwSize = dwInitialCount;

    *ppStringList = pStringList;

cleanup:
    return dwError;

error:
    VmDirStringListFree(pStringList);
    goto cleanup;
}

DWORD
VmDirStringListAdd(
    PVMDIR_STRING_LIST pStringList,
    PCSTR pszString
    )
{
    DWORD dwError = 0;

    // keep string list null-terminated
    if (pStringList->dwCount + 1 == pStringList->dwSize)
    {
        DWORD dwOldSize = pStringList->dwSize;
        DWORD dwNewSize = pStringList->dwSize * 2;

        //
        // Check for overflow.
        //
        if (dwNewSize < pStringList->dwSize)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SIZELIMIT_EXCEEDED);
        }

        dwError = VmDirReallocateMemoryWithInit(
                    pStringList->pStringList,
                    (PVOID*)&pStringList->pStringList,
                    dwNewSize * sizeof(PSTR),
                    dwOldSize * sizeof(PSTR));
        BAIL_ON_VMDIR_ERROR(dwError);

        pStringList->dwSize = dwNewSize;
    }

    pStringList->pStringList[pStringList->dwCount++] = pszString;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirStringListRemove(
    PVMDIR_STRING_LIST pStringList,
    PCSTR pszString
    )
{
    DWORD i = 0;
    DWORD dwError = 0;
    BOOL bFound = FALSE;

    for (i = 0; i < pStringList->dwCount; ++i)
    {
        if (pStringList->pStringList[i] == pszString)
        {
            memmove(&pStringList->pStringList[i],
                    &pStringList->pStringList[i + 1],
                    (pStringList->dwCount - i - 1) * sizeof(PCSTR));
            pStringList->dwCount -= 1;
            bFound = TRUE;
            break;
        }
    }

    if (!bFound)
    {
        //
        // Specified string not found.
        //
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

BOOLEAN
VmDirStringListContains(
    PVMDIR_STRING_LIST pStringList,
    PCSTR pszString
    )
{
    DWORD i = 0;

    for (i = 0; i < pStringList->dwCount; ++i)
    {
        if (pStringList->pStringList[i] == pszString)
        {
            return TRUE;
        }
    }

    return FALSE;
}

DWORD
VmDirStringListAddStrClone(
    PCSTR               pszStr,
    PVMDIR_STRING_LIST  pStrList
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalStr = NULL;

    if (!pszStr || !pStrList)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pszStr, &pszLocalStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringListAdd(pStrList, pszLocalStr);
    BAIL_ON_VMDIR_ERROR(dwError);
    pszLocalStr = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalStr);
    return dwError;

error:
    goto cleanup;
}


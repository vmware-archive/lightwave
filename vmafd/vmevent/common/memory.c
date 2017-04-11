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

#include "includes.h"

DWORD
EventLogAllocateStringA(
    PCSTR pszString,
    PSTR * ppszString
    )
{
    DWORD  dwError = 0;
    PSTR   pszNewString = NULL;
    DWORD dwLen = 0;

    if (!pszString || !ppszString)
    {
        if (ppszString) { *ppszString = NULL; }
        return 0;
    }

    dwLen = EventLogStringLenA(pszString);
    // + 1 for \'0'
    dwError = EventLogAllocateMemory(dwLen + 1, (PVOID*)&pszNewString);
    BAIL_ON_VMEVENT_ERROR(dwError);

#ifndef _WIN32
    memcpy(pszNewString, pszString, dwLen);
#else
    memcpy_s(pszNewString, (dwLen + 1), pszString, dwLen);
#endif
    *ppszString = pszNewString;

cleanup:

    return dwError;

error:

    VMEVENT_SAFE_FREE_MEMORY(pszNewString);

    goto cleanup;
}

DWORD
EventLogAllocateMemory(
    DWORD dwSize,
    PVOID * ppMemory
    )
{
    DWORD dwError = 0;
    void* pMemory = NULL;

    if (!ppMemory || !dwSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

    pMemory = calloc(1, dwSize);
    if (!pMemory)
    {
        dwError = ENOMEM;
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

cleanup:
    *ppMemory = pMemory;

    return dwError;

error:
    VMEVENT_SAFE_FREE_MEMORY(pMemory);
    pMemory = NULL;

    goto cleanup;
}

VOID
EventLogFreeStringA(
    PSTR pszString
    )
{
    VMEVENT_SAFE_FREE_MEMORY(pszString);
    return;
}

VOID
EventLogFreeStringW(
    PWSTR pszString
    )
{
    VMEVENT_SAFE_FREE_MEMORY(pszString);
    return;
}

VOID
EventLogFreeMemory(
    PVOID pMemory
    )
{
    if (pMemory)
    {
        free(pMemory);
    }
}

DWORD
EventLogConvertUnicodetoAnsiString(
    PCWSTR pszUnicodeString,
    PSTR * ppszAnsiString
    )
{
    if (!pszUnicodeString || !ppszAnsiString) {
        if (ppszAnsiString) { *ppszAnsiString = NULL; }
        return 0;
    }
#ifndef _WIN32
    // caller owns ppszAnsiString and should free via  RepoFreeStringA(*ppszAnsiString)
    return LwWc16sToMbs(pszUnicodeString, ppszAnsiString);
#else
    return 0;
#endif
}

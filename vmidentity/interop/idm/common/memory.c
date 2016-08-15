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
 * Module Name:
 *
 *        memory.c
 *
 * Abstract:
 *        Identity Manager - Common Routines
 *
 *        Memory Management
 *
 * Authors: Krishna Ganugapati (krishnag@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *          Adam Bernstein (abernstein@vmware.com)
 *
 */

#include "includes.h"

DWORD
IDMAllocateMemory(
    SIZE_T size,
    PVOID* ppMemory
    )
{
    DWORD dwError = 0;
    PVOID pMemory = NULL;

    if (!ppMemory || (size <= 0))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    pMemory = calloc(size, sizeof(unsigned char));
    if (!pMemory)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_IDM_ERROR(dwError);
    }

    *ppMemory = pMemory;

cleanup:

    return dwError;

error:

    if (pMemory)
    {
        free(pMemory);
        pMemory = NULL;
    }

    goto cleanup;
}


VOID
IDMFreeMemory(
    PVOID pMemory
    )
{
    if (pMemory)
    {
        free(pMemory);
    }

    return;
}

DWORD
IDMAllocateString(
    PWSTR  pszString,
    PWSTR* ppszString
    )
{
    DWORD  dwError = 0;
    PWSTR pszNewString = NULL;

    if (!pszString || !ppszString)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_IDM_ERROR(dwError);
    }
#ifdef _WIN32
{
    SIZE_T len = 0;
    len = wcslen(pszString);

    dwError = IDMAllocateMemory(
                    (len + 1) * sizeof(WCHAR),
                    (PVOID*)&pszNewString);
    BAIL_ON_IDM_ERROR(dwError);

    wcscpy_s(pszNewString, len+1, pszString);
}
#else

    dwError = LwRtlWC16StringDuplicate(
                  &pszNewString,
                  pszString);
    BAIL_ON_IDM_ERROR(dwError);

#endif

    *ppszString = pszNewString;

cleanup:

    return dwError;

error:

    if (ppszString)
    {
        *ppszString = NULL;
    }

    IDM_SAFE_FREE_MEMORY(pszNewString);

    goto cleanup;
}

DWORD
IDMAllocateStringA(
    PSTR  pszString,
    PSTR* ppszString
    )
{
    DWORD dwError = 0;
    PSTR  pszNewString = NULL;

    if (!pszString || !ppszString)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_IDM_ERROR(dwError);
    }
#ifdef _WIN32
{
    SIZE_T len = 0;
    len = strlen(pszString);

    dwError = IDMAllocateMemory(
                    len + 1,
                    (PVOID*)&pszNewString);
    BAIL_ON_IDM_ERROR(dwError);

    strcpy_s(pszNewString, len+1, pszString);
}
#else

    dwError = LwRtlCStringDuplicate(&pszNewString, pszString);
    BAIL_ON_IDM_ERROR(dwError);

#endif

    *ppszString = pszNewString;

cleanup:

    return dwError;

error:

    if (ppszString)
    {
        *ppszString = NULL;
    }

    IDM_SAFE_FREE_MEMORY(pszNewString);

    goto cleanup;
}

VOID
IDMFreeString(
    PWSTR pszString
    )
{
    if (pszString)
    {
        IDMFreeMemory(pszString);
    }
}


#ifdef _WIN32
DWORD
IDMCloneSid(
    PSID pSid,
    PSID *ppNewSid
    )
{
    DWORD dwError = 0;
    DWORD sidLen = 0;
    PSID pNewSid = NULL;

    if (!IsValidSid(pSid))
    {
        dwError = ERROR_INVALID_SID;
        BAIL_ON_IDM_ERROR(dwError);
    }

    sidLen = GetLengthSid(pSid);
    dwError = IDMAllocateMemory(
                    sidLen,
                    (PVOID*) &pNewSid);
    BAIL_ON_IDM_ERROR(dwError);

    if (!CopySid(sidLen, pNewSid, pSid))
    {
        dwError = GetLastError();
        BAIL_ON_IDM_ERROR(dwError);
    }

    *ppNewSid = pNewSid;

error:
    if (dwError)
    {
        IDM_SAFE_FREE_MEMORY(pNewSid);
    }
    return dwError;
}

VOID
IDMFreeSid(
    PSID pSid
    )
{
    if (!pSid || !IsValidSid(pSid))
    {
        return;
    }
    FreeSid(pSid);
    return;
}

DWORD
IDMAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;
    LPWSTR pszUnicodeString = NULL;

    if (!pszSrc || !ppwszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    *ppwszDst = NULL;

    dwSize = MultiByteToWideChar( CP_UTF8, 0, pszSrc, -1, NULL, 0 );
    if (!dwSize)
    {
        dwError = GetLastError();
        BAIL_ON_IDM_ERROR(dwError);
    }

    dwError =
        IDMAllocateMemory(
            dwSize * sizeof(WCHAR), (PVOID *)&pszUnicodeString
        );
    BAIL_ON_IDM_ERROR(dwError);

    dwSize =
        MultiByteToWideChar(
            CP_UTF8, 0, pszSrc, -1, pszUnicodeString, dwSize
        );
    if (!dwSize)
    {
        dwError = GetLastError();
        BAIL_ON_IDM_ERROR(dwError);
    }

    *ppwszDst = pszUnicodeString;
    pszUnicodeString = NULL;

error:

    IDM_SAFE_FREE_MEMORY(pszUnicodeString);
    return dwError;
}

DWORD
IDMAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    )
{
    DWORD dwError = 0;
    LPSTR pszAnsiString = NULL;
    DWORD dwSize = 0;

    if (!pwszSrc || !ppszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_IDM_ERROR(dwError);
    }

    *ppszDst = NULL;

    dwSize =
        WideCharToMultiByte(
            CP_UTF8, 0, pwszSrc, -1, NULL, 0, NULL, NULL
        );
    if(dwSize != 0)
    {
        dwError = IDMAllocateMemory( dwSize+1, (PVOID *)&pszAnsiString );
        BAIL_ON_IDM_ERROR(dwError);

        dwSize =
            WideCharToMultiByte(
                CP_UTF8, 0, pwszSrc, -1, pszAnsiString, dwSize, NULL, NULL
            );
        if (!dwSize)
        {
            dwError = GetLastError();
            BAIL_ON_IDM_ERROR(dwError);
        }

        *ppszDst = pszAnsiString;
        pszAnsiString = NULL;
    }

error:

    IDM_SAFE_FREE_MEMORY(pszAnsiString);
    return dwError;
}
#else

DWORD
IDMCloneSid(
    PSID pSid,
    PSID *ppNewSid
    )
{
    DWORD dwError = 0;
    NTSTATUS status = 0;
    PSID pNewSid = NULL;

    status = RtlDuplicateSid(&pNewSid, pSid);
    dwError = LwNtStatusToWin32Error(status);
    BAIL_ON_IDM_ERROR(dwError);

    *ppNewSid = pNewSid;

error:
    if (dwError)
    {
        IDM_SAFE_FREE_MEMORY(pNewSid);
    }
    return dwError;
}

VOID
IDMFreeSid(
    PSID pSid
    )
{
    if (!pSid)
    {
        return;
    }
    LwFreeMemory(pSid);

    return;
}

DWORD
IDMAllocateStringPrintfV(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    )
{
    DWORD dwError = 0;

    if (!ppszStr || !pszFormat)
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    else
    {
        dwError = LwNtStatusToWin32Error(
                        LwRtlCStringAllocatePrintfV(
                                ppszStr,
                                pszFormat,
                                argList));
    }

    return dwError;
}

DWORD
IDMAllocateStringPrintf(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    )
{
    DWORD dwError = 0;
    va_list args;

    va_start(args, pszFormat);
    dwError = IDMAllocateStringPrintfV(ppszString, pszFormat, args);
    va_end(args);

    return dwError;
}

#endif


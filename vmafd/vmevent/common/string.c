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
EventLogAllocateStringW(
    PCWSTR pszString,
    PWSTR* ppszString
    )
{
    if (!pszString || !ppszString) {
        if (ppszString) { *ppszString = NULL; }
        return 0;
    } 
    return LwAllocateWc16String(ppszString, pszString);
}

DWORD
EventLogStringLenA(
    PCSTR pszStr
)
{
    return ( pszStr != NULL) ? (DWORD)strlen(pszStr) : 0;
}

DWORD
EventLogGetStringLengthW(
    PCWSTR  pwszStr,
    PSIZE_T pLength
    )
{
    ULONG ulError = 0;

    if (!pwszStr || !pLength)
    {
        ulError = ERROR_INVALID_PARAMETER;
    }
    else
    {
        *pLength = LwRtlWC16StringNumChars(pwszStr);
    }

    return ulError;
}

DWORD
EventLogAllocateStringWFromA(
    PCSTR pszAnsiString,
    PWSTR * ppszUnicodeString
    )
{
    if (!pszAnsiString || !ppszUnicodeString) {
        if (ppszUnicodeString) { *ppszUnicodeString = NULL; }
        return 0;
    } 

    // caller owns ppszUnicodeString and should free via
    //EventLogFreeStringW(*ppszUnicodeString)
    return LwMbsToWc16s(pszAnsiString, ppszUnicodeString);
}

DWORD
EventLogAllocateStringAFromW(
    PCWSTR pszUnicodeString,
    PSTR * ppszAnsiString
    )
{
    if (!pszUnicodeString || !ppszAnsiString) {
        if (ppszAnsiString) { *ppszAnsiString = NULL; }
        return 0;
    } 
    // caller owns ppszAnsiString and should free via  RepoFreeStringA(*ppszAnsiString)
    return LwWc16sToMbs(pszUnicodeString, ppszAnsiString);
}

int
EventLogStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    )
{
    return LwRtlCStringCompare(pszStr1, pszStr2, bIsCaseSensitive);
}

DWORD
EventLogAllocateStringPrintfV(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    )
{
    ULONG ulError = 0;

    if (!ppszStr || !pszFormat)
    {
        ulError = ERROR_INVALID_PARAMETER;
    }
    else
    {
        ulError = LwNtStatusToWin32Error(
                        LwRtlCStringAllocatePrintfV(
                                ppszStr,
                                pszFormat,
                                argList));
    }

    return ulError;
}

DWORD
EventLogAllocateStringPrintf(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    )
{
    DWORD dwError = 0;
    va_list args;

    va_start(args, pszFormat);
    dwError = EventLogAllocateStringPrintfV(ppszString, pszFormat, args);
    va_end(args);

    return dwError;
}

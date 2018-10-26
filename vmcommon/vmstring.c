/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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
_VmAllocateStringPrintfV(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    )
{
    DWORD dwError = 0;

    if (!ppszStr || !pszFormat)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
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
VmAllocateStringPrintf(
    PSTR* ppszString,
    PCSTR pszFormat,
    ...
    )
{
    DWORD dwError = 0;
    va_list args;

    va_start(args, pszFormat);
    dwError = _VmAllocateStringPrintfV(ppszString, pszFormat, args);
    va_end(args);

    return dwError;
}

DWORD
VmStringPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    PCSTR pszFormat,
    ...
)
{
    DWORD dwError = 0;
    BOOLEAN bVaStarted = FALSE;
    va_list args;

    va_start(args, pszFormat);
    bVaStarted = TRUE;

    if (!pDestination)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (vsnprintf(pDestination, destinationSize, pszFormat, args ) < 0)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

error:
    if(bVaStarted != FALSE)
    {
        va_end(args);
    }

    return dwError;
}

int
VmStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    )
{
    return LwRtlCStringCompare(pszStr1, pszStr2, bIsCaseSensitive);
}

size_t
VmStringLenA(
    PCSTR pszStr
    )
{
    return strlen(pszStr);
}

VOID
VmStringTrimSpace(
    PSTR pszStr
    )
{
    size_t  len = 0;
    size_t  start = 0;
    size_t  end = 0;
    size_t  i = 0;
    size_t  j = 0;

    if (pszStr)
    {
        len = VmStringLenA(pszStr);
        if (len > 0)
        {
            for (start = 0; start < len && VM_COMMON_ASCII_SPACE(pszStr[start]); start++);
            for (end = len - 1; end > 0 && VM_COMMON_ASCII_SPACE(pszStr[end]); end--);

            for (i = start; i <= end; i++, j++)
            {
                pszStr[j] = pszStr[i];
            }
            pszStr[j] = '\0';
        }
    }

}
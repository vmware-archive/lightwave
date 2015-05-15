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
VmwDeployAllocateStringW(
    PCWSTR pwszSrc,
    PWSTR* ppwszDst
    )
{
    DWORD dwError = 0;

    if (!pwszSrc || !ppwszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    else
    {
        dwError = LwNtStatusToWin32Error(
                        LwRtlWC16StringDuplicate(ppwszDst, pwszSrc));
    }

    return dwError;
}

DWORD
VmwDeployAllocateStringA(
    PCSTR pszSrc,
    PSTR* ppszDst
    )
{
    DWORD dwError = 0;

    if (!pszSrc || !ppszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    else
    {
        dwError = LwNtStatusToWin32Error(
                        LwRtlCStringDuplicate(ppszDst, pszSrc));
    }

    return dwError;
}

DWORD
VmwDeployAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    )
{
    DWORD dwError = 0;

    if (!pszSrc || !ppwszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    else
    {
        dwError = LwNtStatusToWin32Error(
                        LwRtlWC16StringAllocateFromCString(ppwszDst, pszSrc));
    }

    return dwError;
}

DWORD
VmwDeployAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    )
{
    DWORD dwError = 0;

    if (!pwszSrc || !ppszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    else
    {
        dwError = LwNtStatusToWin32Error(
                        LwRtlCStringAllocateFromWC16String(ppszDst, pwszSrc));
    }

    return dwError;
}

DWORD
VmwDeployAllocateStringPrintf(
    PSTR* ppszString,
    PCSTR pszFormat,
    ...
    )
{
    DWORD dwError = 0;
    va_list args;

    va_start(args, pszFormat);
    dwError = VmwDeployAllocateStringPrintfV(ppszString, pszFormat, args);
    va_end(args);

    return dwError;
}

DWORD
VmwDeployAllocateStringPrintfV(
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



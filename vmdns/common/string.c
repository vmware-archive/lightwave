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
 * Module   : string.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Common Utilities (Client & Server)
 *
 *            String Management
 *
 */

#include "includes.h"

#ifndef _WIN32

ULONG
VmDnsAllocateStringW(
    PCWSTR pwszSrc,
    PWSTR* ppwszDst
    )
{
    ULONG ulError = 0;

    if (!pwszSrc || !ppwszDst)
    {
        ulError = ERROR_INVALID_PARAMETER;
    }
    else
    {
        ulError = LwNtStatusToWin32Error(
                        LwRtlWC16StringDuplicate(ppwszDst, pwszSrc));
    }

    return ulError;
}

ULONG
VmDnsAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    )
{
    ULONG ulError = 0;

    if (!pszSrc || !ppwszDst)
    {
        ulError = ERROR_INVALID_PARAMETER;
    }
    else
    {
        ulError = LwNtStatusToWin32Error(
                        LwRtlWC16StringAllocateFromCString(ppwszDst, pszSrc));
    }

    return ulError;
}

ULONG
VmDnsAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    )
{
    ULONG ulError = 0;

    if (!pwszSrc || !ppszDst)
    {
        ulError = ERROR_INVALID_PARAMETER;
    }
    else
    {
        ulError = LwNtStatusToWin32Error(
                        LwRtlCStringAllocateFromWC16String(ppszDst, pwszSrc));
    }

    return ulError;
}

ULONG
VmDnsAllocateStringPrintfVA(
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

ULONG
VmDnsAllocateStringPrintfA(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    )
{
    DWORD dwError = 0;
    va_list args;

    va_start(args, pszFormat);
    dwError = VmDnsAllocateStringPrintfVA(ppszString, pszFormat, args);
    va_end(args);

    return dwError;
}

ULONG
VmDnsGetStringLengthW(
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

ULONG
VmDnsStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    )
{
    if (!pszStr1 && !pszStr2)
    {
        return 0;
    }
    else if (!pszStr1)
    {
        return 1;
    }
    else if (!pszStr2)
    {
        return -1;
    }
    else
    {
        return LwRtlCStringCompare(pszStr1, pszStr2, bIsCaseSensitive);
    }
}

ULONG
VmDnsStringNCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    size_t n,
    BOOLEAN bIsCaseSensitive
    )
{
    if( bIsCaseSensitive != FALSE )
    {
        return strncmp(pszStr1, pszStr2, n) ;
    }
    else
    {
        return strncasecmp(pszStr1, pszStr2, n) ;
    }
}

SIZE_T
VmDnsStringLenA(
    PCSTR pszStr
)
{
    return strlen(pszStr);
}

PSTR
VmDnsStringChrA(
   PCSTR str,
   int c
)
{
    return strchr( str, c );
}

PSTR
VmDnsStringRChrA(
   PCSTR str,
   int c
)
{
    return strrchr(str, c);
}

PSTR
VmDnsStringTokA(
   PSTR strToken,
   PCSTR strDelimit,
   PSTR* context
)
{
    return strtok_r( strToken, strDelimit, context );
}

PSTR
VmDnsStringStrA(
   PCSTR str,
   PCSTR strSearch
)
{
    return strstr( str, strSearch );
}

DWORD
VmDnsStringCpyA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
)
{
    // TODO: add check for destination buffer size ?
    strcpy( strDestination, strSource);
    return 0;
}

DWORD
VmDnsStringNCpyA(
   PSTR strDest,
   size_t numberOfElements,
   PCSTR strSource,
   size_t count
)
{
    // TODO: add check for destination buffer size ?
    strncpy(strDest, strSource, count);
    return 0;
}

DWORD
VmDnsStringCatA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
)
{
    // TODO: add check for destination buffer size ?
    strcat( strDestination, strSource );
    return 0;
}

int64_t
VmDnsStringToLA(
   PCSTR nptr,
   PSTR* endptr,
   int base
)
{
    return strtol( nptr, endptr, base );
}

int VmDnsStringToIA(
   PCSTR pStr
)
{
    return atoi( pStr );
}

DWORD
VmDnsStringErrorA(
   PSTR buffer,
   size_t numberOfElements,
   int errnum
)
{
    return VmDnsStringNCpyA(
        buffer, numberOfElements, strerror(errnum), numberOfElements - 1 );
}

PSTR
VmDnsCaselessStrStrA(
    PCSTR pszStr1,
    PCSTR pszStr2
    )
{
    PSTR pszPosStr1 = NULL;
    size_t sStr2Len = 0;

    if (IsNullOrEmptyString(pszStr1) || IsNullOrEmptyString(pszStr2))
        return NULL;

    pszPosStr1 = (PSTR)pszStr1;
    sStr2Len = strlen(pszStr2);

    do
    {
        if (!strncasecmp(pszPosStr1, pszStr2, sStr2Len))
        {
            return pszPosStr1;
        }
    }while (*(++pszPosStr1));

    return NULL;
}

DWORD
VmDnsStringPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    PCSTR pszFormat,
    ...
)
{
    DWORD dwError = ERROR_SUCCESS;
    BOOLEAN bVaStarted = FALSE;
    va_list args;

    va_start(args, pszFormat);
    bVaStarted = TRUE;

    // TODO: check buffer size

    if ( vsprintf( pDestination, pszFormat, args ) < 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

error:
    if(bVaStarted != FALSE)
    {
        va_end(args);
    }

    return dwError;
}

DWORD
VmDnsStringNPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    size_t maxSize,
    PCSTR pszFormat,
    ...
)
{
    DWORD dwError = ERROR_SUCCESS;
    BOOLEAN bVaStarted = FALSE;
    va_list args;

    va_start(args, pszFormat);
    bVaStarted = TRUE;

    // TODO: check buffer size

    if ( vsnprintf(
             pDestination, maxSize, pszFormat, args ) < 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

error:

    if(bVaStarted != FALSE)
    {
        va_end(args);
    }

    return dwError;
}

#endif //#ifndef _WIN32


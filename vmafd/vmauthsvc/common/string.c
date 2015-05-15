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

#ifndef _WIN32

ULONG
VmAuthsvcAllocateStringW(
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
VmAuthsvcAllocateStringWFromA(
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
VmAuthsvcAllocateStringAFromW(
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
VmAuthsvcAllocateStringPrintfV(
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
VmAuthsvcAllocateStringPrintf(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    )
{
    DWORD dwError = 0;
    va_list args;

    va_start(args, pszFormat);
    dwError = VmAuthsvcAllocateStringPrintfV(ppszString, pszFormat, args);
    va_end(args);

    return dwError;
}

ULONG
VmAuthsvcGetStringLengthW(
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
VmAuthsvcStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    )
{
    return LwRtlCStringCompare(pszStr1, pszStr2, bIsCaseSensitive);
}

ULONG
VmAuthsvcStringNCompareA(
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
VmAuthsvcStringLenA(
    PCSTR pszStr
)
{
    return strlen(pszStr);
}

PSTR
VmAuthsvcStringChrA(
   PCSTR str,
   int c
)
{
    return strchr( str, c );
}

PSTR
VmAuthsvcStringRChrA(
   PCSTR str,
   int c
)
{
    return strrchr(str, c);
}

PSTR
VmAuthsvcStringTokA(
   PSTR strToken,
   PCSTR strDelimit,
   PSTR* context
)
{
    return strtok_r( strToken, strDelimit, context );
}

PSTR
VmAuthsvcStringStrA(
   PCSTR str,
   PCSTR strSearch
)
{
    return strstr( str, strSearch );
}

DWORD
VmAuthsvcStringCpyA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
)
{
    DWORD dwError = 0;
    size_t count = 0;

    if (!strDestination || !strSource)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

    count = strlen(strSource) + 1;
    if (count > numberOfElements)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

    strncpy(strDestination, strSource, count);

error:

    return dwError;
}

DWORD
VmAuthsvcStringNCpyA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource,
   size_t count
)
{
    DWORD dwError = 0;

    if (!strDestination || !strSource)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

    if (count > numberOfElements)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

    strncpy(strDestination, strSource, count);

error:

    return dwError;
}

DWORD
VmAuthsvcStringCatA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
)
{
    DWORD dwError = 0;
    size_t count = 0;
    size_t cchSource = 0;

    if (!strDestination || !strSource)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

    cchSource = strlen(strSource);
    count = strlen(strDestination) + cchSource + 1;
    if (count > numberOfElements)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

    strncat(strDestination, strSource, cchSource + 1);

error:

    return dwError;
}

int64_t
VmAuthsvcStringToLA(
   PCSTR nptr,
   PSTR* endptr,
   int base
)
{
    return strtol( nptr, endptr, base );
}

int VmAuthsvcStringToIA(
   PCSTR pStr
)
{
    return atoi( pStr );
}

DWORD
VmAuthsvcStringErrorA(
   PSTR buffer,
   size_t numberOfElements,
   int errnum
)
{
    return VmAuthsvcStringNCpyA(
        buffer, numberOfElements, strerror(errnum), numberOfElements - 1 );
}

PSTR
VmAuthsvcCaselessStrStrA(
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
VmAuthsvcStringPrintFA(
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

    if (!pDestination)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

    if (vsnprintf(pDestination, destinationSize, pszFormat, args ) < 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

error:
    if(bVaStarted != FALSE)
    {
        va_end(args);
    }

    return dwError;
}

DWORD
VmAuthsvcStringNPrintFA(
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

    if (!pDestination || maxSize > destinationSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

    if ( vsnprintf(
             pDestination, maxSize, pszFormat, args ) < 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAUTHSVC_ERROR(dwError);
    }

error:

    if(bVaStarted != FALSE)
    {
        va_end(args);
    }

    return dwError;
}

#endif //#ifndef _WIN32


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
VmAfdAllocateStringW(
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
VmAfdAllocateStringWFromA(
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
VmAfdAllocateStringAFromW(
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
VmAfdAllocateStringPrintfV(
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
VmAfdAllocateStringPrintf(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    )
{
    DWORD dwError = 0;
    va_list args;

    va_start(args, pszFormat);
    dwError = VmAfdAllocateStringPrintfV(ppszString, pszFormat, args);
    va_end(args);

    return dwError;
}

ULONG
VmAfdGetStringLengthW(
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

int
VmAfdStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    )
{
    return LwRtlCStringCompare(pszStr1, pszStr2, bIsCaseSensitive);
}

int
VmAfdStringNCompareA(
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

int
VmAfdStringIsEqualW(
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    BOOLEAN bIsCaseSensitive
    )
{
    return LwRtlWC16StringIsEqual (
                            pwszStr1,
                            pwszStr2,
                            bIsCaseSensitive
                            );
}

SIZE_T
VmAfdStringLenA(
    PCSTR pszStr
)
{
    return strlen(pszStr);
}

PSTR
VmAfdStringChrA(
   PCSTR str,
   int c
)
{
    return strchr( str, c );
}

PSTR
VmAfdStringRChrA(
   PCSTR str,
   int c
)
{
    return strrchr(str, c);
}

PSTR
VmAfdStringTokA(
   PSTR strToken,
   PCSTR strDelimit,
   PSTR* context
)
{
    return strtok_r( strToken, strDelimit, context );
}

PSTR
VmAfdStringStrA(
   PCSTR str,
   PCSTR strSearch
)
{
    return strstr( str, strSearch );
}

DWORD
VmAfdStringCpyA(
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
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    count = strlen(strSource) + 1;
    if (count > numberOfElements)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    strncpy(strDestination, strSource, count);

error:

    return dwError;
}

DWORD
VmAfdStringNCpyA(
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
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (count > numberOfElements)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    strncpy(strDestination, strSource, count);

error:

    return dwError;
}

DWORD
VmAfdStringCatA(
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
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    cchSource = strlen(strSource);
    count = strlen(strDestination) + cchSource + 1;
    if (count > numberOfElements)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    strncat(strDestination, strSource, cchSource + 1);

error:

    return dwError;
}

int64_t
VmAfdStringToLA(
   PCSTR nptr,
   PSTR* endptr,
   int base
)
{
    return strtol( nptr, endptr, base );
}

DWORD
VmAfdStringToINT64(
    PCSTR     pszString,
    PSTR*     ppEndPtr,
    INT64*    pOutVal
    )
{
    DWORD    dwError = 0;
    INT64    value = 0;

    if (!pszString || !pOutVal)
    {
        BAIL_WITH_VMAFD_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    errno = 0;
    value = strtoll(pszString, ppEndPtr, 10);
    if (errno)
    {
        BAIL_WITH_VMAFD_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    *pOutVal = value;

cleanup:
    return dwError;

error:
    goto cleanup;
}

int
VmAfdStringToIA(
   PCSTR pStr
)
{
    return atoi( pStr );
}

DWORD
VmAfdStringErrorA(
   PSTR buffer,
   size_t numberOfElements,
   int errnum
)
{
    return VmAfdStringNCpyA(
        buffer, numberOfElements, strerror(errnum), numberOfElements - 1 );
}

PSTR
VmAfdCaselessStrStrA(
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
    } while (*(++pszPosStr1));

    return NULL;
}

DWORD
VmAfdStringPrintFA(
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
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (vsnprintf(pDestination, destinationSize, pszFormat, args ) < 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:
    if(bVaStarted != FALSE)
    {
        va_end(args);
    }

    return dwError;
}

DWORD
VmAfdStringNPrintFA(
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
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if ( vsnprintf(
             pDestination, maxSize, pszFormat, args ) < 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    if(bVaStarted != FALSE)
    {
        va_end(args);
    }

    return dwError;
}

DWORD
VmAfdStringNCpyW(
    PWSTR strDestination,
    size_t numberOfElements,
    PCWSTR strSource,
    size_t count
    )
{
    DWORD dwError = 0;
    size_t bytesToCopy = 0;
    if (count > 0){
        if (strDestination == NULL){
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        if (numberOfElements == 0 || numberOfElements < count){
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            BAIL_ON_VMAFD_ERROR (dwError);
        }
        bytesToCopy = count * sizeof(WCHAR);
        memcpy (strDestination,strSource,bytesToCopy);
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmAfdUpperCaseStringW(
    PWSTR pwszString)
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszString, dwError);

    dwError = ERROR_CALL_NOT_IMPLEMENTED;

error:

    return dwError;
}

DWORD
VmAfdLowerCaseStringW(
    PWSTR pwszString)
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszString, dwError);

    dwError = ERROR_CALL_NOT_IMPLEMENTED;

error:

    return dwError;
}

#endif //#ifndef _WIN32

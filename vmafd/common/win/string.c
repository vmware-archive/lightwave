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

#ifdef _WIN32

ULONG
VmAfdAllocateStringW(
    PCWSTR pwszSrc,
    PWSTR* ppwszDst
    )
{
    DWORD dwError = 0;
    LPWSTR pwszNewString = NULL;
    size_t len = 0;

    if (!pwszSrc || !ppwszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    *ppwszDst = NULL;

    len = wcslen(pwszSrc);
    dwError =
        VmAfdAllocateMemory(
            (len + 1)*sizeof(WCHAR), (PVOID *)&pwszNewString
        );
    BAIL_ON_VMAFD_ERROR(dwError);

    wcscpy_s(pwszNewString, (len + 1), pwszSrc);

    *ppwszDst = pwszNewString;
    pwszNewString = NULL;

error:
    VMAFD_SAFE_FREE_MEMORY(pwszNewString);
    return dwError;
}

ULONG
VmAfdAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;
    LPWSTR pszUnicodeString = NULL;

    if( !pszSrc )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!ppwszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    *ppwszDst = NULL;

    dwSize = MultiByteToWideChar( CP_UTF8, 0, pszSrc, -1, NULL, 0 );
    if (!dwSize)
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =
        VmAfdAllocateMemory(
            dwSize * sizeof(WCHAR), (PVOID *)&pszUnicodeString
        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwSize =
        MultiByteToWideChar(
            CP_UTF8, 0, pszSrc, -1, pszUnicodeString, dwSize
        );
    if (!dwSize)
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszDst = pszUnicodeString;
    pszUnicodeString = NULL;

error:

    VMAFD_SAFE_FREE_MEMORY(pszUnicodeString);
    return dwError;
}

ULONG
VmAfdAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    )
{
    DWORD dwError = 0;
    LPSTR pszAnsiString = NULL;
    DWORD dwSize = 0;

    if( !pwszSrc )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!ppszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    *ppszDst = NULL;

    dwSize =
        WideCharToMultiByte(
            CP_UTF8, 0, pwszSrc, -1, NULL, 0, NULL, NULL
        );
    if(dwSize != 0)
    {
        dwError = VmAfdAllocateMemory( dwSize+1, (PVOID *)&pszAnsiString );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwSize =
            WideCharToMultiByte(
                CP_UTF8, 0, pwszSrc, -1, pszAnsiString, dwSize, NULL, NULL
            );
        if (!dwSize)
        {
            dwError = GetLastError();
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        *ppszDst = pszAnsiString;
        pszAnsiString = NULL;
    }

error:

    VMAFD_SAFE_FREE_MEMORY(pszAnsiString);
    return dwError;
}

ULONG
VmAfdAllocateStringPrintfV(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    )
{
    DWORD dwError = 0;
    int iSize = 0;
    LPSTR pszAnsiString = NULL;

    if (!ppszStr || !pszFormat)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    *ppszStr = NULL;

    iSize = _vscprintf(pszFormat, argList);
    if (iSize <= -1)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    iSize += 1;

    dwError =
        VmAfdAllocateMemory( (iSize*sizeof(CHAR)), (PVOID *)&pszAnsiString );
    BAIL_ON_VMAFD_ERROR(dwError);

    // we should never truncate since we measured the buffer ....
    if( vsnprintf_s(
            pszAnsiString, iSize, _TRUNCATE, pszFormat, argList
        ) < 0
      )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszStr = pszAnsiString;
    pszAnsiString = NULL;

error:

    VMAFD_SAFE_FREE_MEMORY(pszAnsiString);

    return dwError;
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
    DWORD dwError = 0;

    if (!pwszStr || !pLength)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *pLength = wcslen(pwszStr);

error:

    return dwError;
}

DWORD
VmAfdStringCpyW(
    PWSTR strDestination,
    size_t numberOfElements,
    PCWSTR strSource
)
{
    DWORD dwError = ERROR_SUCCESS;

    if ( wcscpy_s(strDestination, (numberOfElements + 1), strSource) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
error:
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
    DWORD dwError = ERROR_SUCCESS;

    if ( wcsncpy_s(strDestination, numberOfElements, strSource, count) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
error:
    return dwError;
}

DWORD
VmAfdStringCatW(
    PWSTR strDestination,
    size_t numberOfElements,
    PCWSTR strSource
)
{
    DWORD dwError = ERROR_SUCCESS;

    if ( wcscat_s( strDestination, numberOfElements, strSource) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
error:
    return dwError;
}

PWSTR
VmAfdStringChrW(
    PCWSTR str,
    WCHAR wchr
)
{
    return (PWSTR)wcschr(str, wchr);
}

int
VmAfdStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    )
{
    int result = 0;
    if ( ( pszStr1 == NULL ) && (pszStr2 == NULL) )
    {
        result = 0; // NULL == NULL
    }
    else if ( pszStr1 == NULL )
    {
        result = -1; // NULL < not-NULL
    }
    else if ( pszStr2 == NULL )
    {
        result = 1; // NOT-NULL > NULL
    }
    else
    {
        if( bIsCaseSensitive )
        {
            result = strcmp(pszStr1, pszStr2);
        }
        else
        {
            result = _stricmp(pszStr1, pszStr2);
        }
    }
    return result;
}

int
VmAfdStringNCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    size_t n,
    BOOLEAN bIsCaseSensitive
    )
{
    int result = 0;
    if ( ( pszStr1 == NULL ) && (pszStr2 == NULL) )
    {
        result = 0; // NULL == NULL
    }
    else if ( pszStr1 == NULL )
    {
        result = -1; // NULL < not-NULL
    }
    else if ( pszStr2 == NULL )
    {
        result = 1; // NOT-NULL > NULL
    }
    else
    {
        if( bIsCaseSensitive )
        {
            result = strncmp(pszStr1, pszStr2, n);
        }
        else
        {
            result = _strnicmp(pszStr1, pszStr2, n);
        }
    }
    return result;
}

int
VmAfdStringCompareW(
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    BOOLEAN bIsCaseSensitive
)
{
    int result = 0;
    if ( ( pwszStr1 == NULL ) && (pwszStr2 == NULL) )
    {
        result = 0; // NULL == NULL
    }
    else if ( pwszStr1 == NULL )
    {
        result = -1; // NULL < not-NULL
    }
    else if ( pwszStr2 == NULL )
    {
        result = 1; // NOT-NULL > NULL
    }
    else
    {
        if( bIsCaseSensitive )
        {
            result = wcscmp(pwszStr1, pwszStr2);
        }
        else
        {
            result = _wcsicmp(pwszStr1, pwszStr2);
        }
    }
    return result;
}

int
VmAfdStringIsEqualW (
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    BOOLEAN bIsCaseSensitive
    )
{
    return ! (VmAfdStringCompareW (pwszStr1, pwszStr2, bIsCaseSensitive));
}

int
VmAfdStringNCompareW(
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    size_t n,
    BOOLEAN bIsCaseSensitive
)
{
    int result = 0;
    if ( ( pwszStr1 == NULL ) && (pwszStr2 == NULL) )
    {
        result = 0; // NULL == NULL
    }
    else if ( pwszStr1 == NULL )
    {
        result = -1; // NULL < not-NULL
    }
    else if ( pwszStr2 == NULL )
    {
        result = 1; // NOT-NULL > NULL
    }
    else
    {
        if( bIsCaseSensitive )
        {
            result = wcsncmp(pwszStr1, pwszStr2, n);
        }
        else
        {
            result = _wcsnicmp(pwszStr1, pwszStr2, n);
        }
    }
    return result;
}

SIZE_T
VmAfdStringLenA(
    PCSTR pszStr
)
{
    return ( pszStr != NULL) ? strlen(pszStr) : 0;
}

PSTR
VmAfdStringChrA(
    PCSTR str,
    int c
)
{
    return (PSTR)strchr( str, c );
}

PSTR
VmAfdStringRChrA(
    PCSTR str,
    int c
)
{
    return (PSTR)strrchr(str, c);
}

PSTR
VmAfdStringTokA(
    PSTR strToken,
    PCSTR strDelimit,
    PSTR* context
)
{
    return strtok_s( strToken, strDelimit, context );
}

PSTR
VmAfdStringStrA(
    PCSTR str,
    PCSTR strSearch
)
{
    return (PSTR)strstr( str, strSearch );
}

DWORD
VmAfdStringCpyA(
    PSTR strDestination,
    size_t numberOfElements,
    PCSTR strSource
)
{
    DWORD dwError = ERROR_SUCCESS;
    if ( strcpy_s( strDestination, numberOfElements, strSource) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
error:
    return dwError;
}

DWORD
VmAfdStringNCpyA(
    PSTR strDest,
    size_t numberOfElements,
    PCSTR strSource,
    size_t count
)
{
    DWORD dwError = ERROR_SUCCESS;
    if ( strncpy_s(strDest, numberOfElements, strSource, count) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
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
    DWORD dwError = ERROR_SUCCESS;
    if ( strcat_s( strDestination, numberOfElements, strSource) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
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
    return _strtoi64( nptr, endptr, base );
}

int VmAfdStringToIA(
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
    DWORD dwError = ERROR_SUCCESS;
    if ( strerror_s( buffer, numberOfElements, errnum) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
error:
    return dwError;
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
    {
        return NULL;
    }

    pszPosStr1 = (PSTR)pszStr1;
    sStr2Len = strlen(pszStr2);
    do
    {
        if (!_strnicmp(pszPosStr1, pszStr2, sStr2Len))
        {
            return pszPosStr1;
        }
    }
    while (*(++pszPosStr1));

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

    if ( vsprintf_s( pDestination, destinationSize, pszFormat, args ) < 0 )
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

    if ( vsnprintf_s(
             pDestination, destinationSize, maxSize, pszFormat, args ) < 0 )
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
VmAfdUpperCaseStringW(
    PWSTR pwszString)
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszString, dwError);

    _wcsupr(pwszString);

error:

    return dwError;
}

DWORD
VmAfdLowerCaseStringW(
    PWSTR pwszString)
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszString, dwError);

    _wcslwr(pwszString);

error:

    return dwError;
}

#endif //#ifdef _WIN32

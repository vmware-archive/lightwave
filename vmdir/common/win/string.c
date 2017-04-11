/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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
VmDirAllocateStringW(
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
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    *ppwszDst = NULL;

    len = wcslen(pwszSrc);
    dwError =
        VmDirAllocateMemory(
            (len + 1)*sizeof(WCHAR), (PVOID *)&pwszNewString
        );
    BAIL_ON_VMDIR_ERROR(dwError);

    wcscpy_s(pwszNewString, (len + 1), pwszSrc);

    *ppwszDst = pwszNewString;
    pwszNewString = NULL;

error:
    VMDIR_SAFE_FREE_MEMORY(pwszNewString);
    return dwError;
}

ULONG
VmDirAllocateStringWFromA(
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
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!ppwszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    *ppwszDst = NULL;

    dwSize = MultiByteToWideChar( CP_UTF8, 0, pszSrc, -1, NULL, 0 );
    if (!dwSize)
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError =
        VmDirAllocateMemory(
            dwSize * sizeof(WCHAR), (PVOID *)&pszUnicodeString
        );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwSize =
        MultiByteToWideChar(
            CP_UTF8, 0, pszSrc, -1, pszUnicodeString, dwSize
        );
    if (!dwSize)
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppwszDst = pszUnicodeString;
    pszUnicodeString = NULL;

error:

    VMDIR_SAFE_FREE_MEMORY(pszUnicodeString);
    return dwError;
}

ULONG
VmDirAllocateStringAFromW(
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
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!ppszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    *ppszDst = NULL;

    dwSize =
        WideCharToMultiByte(
            CP_UTF8, 0, pwszSrc, -1, NULL, 0, NULL, NULL
        );
    if(dwSize != 0)
    {
        dwError = VmDirAllocateMemory( dwSize+1, (PVOID *)&pszAnsiString );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwSize =
            WideCharToMultiByte(
                CP_UTF8, 0, pwszSrc, -1, pszAnsiString, dwSize, NULL, NULL
            );
        if (!dwSize)
        {
            dwError = GetLastError();
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        *ppszDst = pszAnsiString;
        pszAnsiString = NULL;
    }

error:

    VMDIR_SAFE_FREE_MEMORY(pszAnsiString);
    return dwError;
}

ULONG
VmDirAllocateStringPrintfV(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    )
{
    DWORD dwError = 0;
    // TODO: pgu
    // change dwSize to iSize
    int dwSize = 0;
    LPSTR pszAnsiString = NULL;

    if (!ppszStr || !pszFormat)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    *ppszStr = NULL;

    /*
    MSDN:
        If execution is allowed to continue, the functions
        return -1 and set errno to EINVAL.
    */
    //TODO: pgu
    // signed/unsigned comparison issue need to be fixed.
    dwSize = _vscprintf(pszFormat, argList);
    if (dwSize <= -1)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /*
    MSDN:
        _vscprintf returns the number of characters that would be generated
        if the string pointed to by the list of arguments was printed or
        sent to a file or buffer using the specified formatting codes.
        The value returned does not include the terminating null character.
    */
    dwSize += 1;

    dwError =
        VmDirAllocateMemory( (dwSize*sizeof(CHAR)), (PVOID *)&pszAnsiString );
    BAIL_ON_VMDIR_ERROR(dwError);

    /*
    MSDN:
        return the number of characters written, not including
        the terminating null, or a negative value if an output error occurs.
    */
    // we should never truncate since we measured the buffer ....
    if( vsnprintf_s(
            pszAnsiString, dwSize, _TRUNCATE, pszFormat, argList
        ) < 0
      )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszStr = pszAnsiString;
    pszAnsiString = NULL;

error:

    VMDIR_SAFE_FREE_MEMORY(pszAnsiString);

    return dwError;
}

ULONG
VmDirAllocateStringPrintf(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    )
{
    DWORD dwError = 0;
    va_list args;

    va_start(args, pszFormat);
    dwError = VmDirAllocateStringPrintfV(ppszString, pszFormat, args);
    va_end(args);

    return dwError;
}

ULONG
VmDirGetStringLengthW(
    PCWSTR  pwszStr,
    PSIZE_T pLength
    )
{
    DWORD dwError = 0;

    if (!pwszStr || !pLength)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pLength = wcslen(pwszStr);

error:

    return dwError;
}

LONG
VmDirStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    )
{
    ULONG result = 0;
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

LONG
VmDirStringNCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    size_t n,
    BOOLEAN bIsCaseSensitive
    )
{
    ULONG result = 0;
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

BOOLEAN
VmDirStringStartsWith(
    PCSTR   pszStr,
    PCSTR   pszPrefix,
    BOOLEAN bIsCaseSensitive
    )
{
    BOOLEAN bStartsWith = FALSE;

    if (IsNullOrEmptyString(pszPrefix))
    {
        bStartsWith = TRUE;
    }
    else if (!IsNullOrEmptyString(pszStr))
    {
        size_t strlen = VmDirStringLenA(pszStr);
        size_t prefixlen = VmDirStringLenA(pszPrefix);

        if (strlen >= prefixlen)
        {
            if (VmDirStringNCompareA(
                    pszStr, pszPrefix, prefixlen, bIsCaseSensitive) == 0)
            {
                bStartsWith = TRUE;
            }
        }
    }

    return bStartsWith;
}

BOOLEAN
VmDirStringEndsWith(
    PCSTR   pszStr,
    PCSTR   pszSuffix,
    BOOLEAN bIsCaseSensitive
    )
{
    BOOLEAN bEndsWith = FALSE;

    if (IsNullOrEmptyString(pszSuffix))
    {
        bEndsWith = TRUE;
    }
    else if (!IsNullOrEmptyString(pszStr))
    {
        size_t strlen = VmDirStringLenA(pszStr);
        size_t suffixlen = VmDirStringLenA(pszSuffix);

        if (strlen >= suffixlen)
        {
            size_t offset = strlen - suffixlen;

            if (VmDirStringCompareA(
                    pszStr + offset, pszSuffix, bIsCaseSensitive) == 0)
            {
                bEndsWith = TRUE;
            }
        }
    }

    return bEndsWith;
}

SIZE_T
VmDirStringLenA(
    PCSTR pszStr
)
{
    return ( pszStr != NULL) ? strlen(pszStr) : 0;
}

PSTR
VmDirStringChrA(
   PCSTR str,
   int c
)
{
    return (PSTR)strchr( str, c );
}

PSTR
VmDirStringRChrA(
   PCSTR str,
   int c
)
{
    return (PSTR)strrchr(str, c);
}

PSTR
VmDirStringTokA(
   PSTR strToken,
   PCSTR strDelimit,
   PSTR* context
)
{
    return strtok_s( strToken, strDelimit, context );
}

PSTR
VmDirStringStrA(
   PCSTR str,
   PCSTR strSearch
)
{
    return (PSTR)strstr( str, strSearch );
}

static
char *strcasestr(const char *haystack, const char *needle)
{
    char *haystack_lc = NULL;
    char *needle_lc = NULL;
    int i = 0;
    char *found = NULL;

    haystack_lc = _strdup(haystack);
    needle_lc = _strdup(needle);
    if (haystack_lc && needle_lc)
    {
        for (i=0; haystack[i]; i++)
        {
            haystack_lc[i] = tolower(haystack[i]);
        }
        for (i=0; needle[i]; i++)
        {
            needle_lc[i] = tolower(needle[i]);
        }
        found = strstr(haystack_lc, needle_lc);
    }
    if (found)
    {
        found = (char *) haystack + (found - haystack_lc);
    }

    if (haystack_lc)
    {
        free(haystack_lc);
    }
    if (needle_lc)
    {
        free(needle_lc);
    }
    return found;
}

PSTR
VmDirStringCaseStrA(
   PCSTR    pszSource,
   PCSTR    pszPattern
)
{
    return strcasestr( pszSource, pszPattern );
}

DWORD
VmDirStringCpyA(
    PSTR strDestination,
    size_t numberOfElements,
    PCSTR strSource
)
{
    DWORD dwError = ERROR_SUCCESS;
    size_t count = 0;

    if (strSource == NULL || strDestination == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    count = strlen(strSource) + 1;
    if (count > numberOfElements)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( strcpy_s( strDestination, numberOfElements, strSource) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
error:
    return dwError;
}

DWORD
VmDirStringNCpyA(
   PSTR strDest,
   size_t numberOfElements,
   PCSTR strSource,
   size_t count
)
{
    DWORD dwError = ERROR_SUCCESS;

    if (strSource == NULL || strDest == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (count + 1 > numberOfElements)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( strncpy_s(strDest, numberOfElements, strSource, count) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
error:
    return dwError;
}

DWORD
VmDirStringCatA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
)
{
    DWORD dwError = ERROR_SUCCESS;
    size_t count = 0;
    size_t cchSource = 0;

    if (strSource == NULL || strDestination == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    cchSource = strlen(strSource);
    count = strlen(strDestination) + cchSource + 1;
    if (count > numberOfElements)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( strcat_s( strDestination, numberOfElements, strSource) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
error:
    return dwError;
}

DWORD
VmDirStringNCatA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource,
   size_t number
)
{
    DWORD dwError = ERROR_SUCCESS;
    size_t count;

    if (strSource == NULL || strDestination == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    count = strlen(strDestination) + strlen(strSource) + 1;
    if (count > numberOfElements )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( strncat_s( strDestination, numberOfElements, strSource, number) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
error:
    return dwError;
}

int64_t
VmDirStringToLA(
   PCSTR nptr,
   PSTR* endptr,
   int base
)
{
    return _strtoi64( nptr, endptr, base );
}

int VmDirStringToIA(
   PCSTR pStr
)
{
    return atoi( pStr );
}

DWORD
VmDirStringErrorA(
   PSTR buffer,
   size_t numberOfElements,
   int errnum
)
{
    DWORD dwError = ERROR_SUCCESS;
    if ( strerror_s( buffer, numberOfElements, errnum) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
error:
    return dwError;
}

PSTR
VmDirCaselessStrStrA(
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
VmDirStringPrintFA(
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
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    if(bVaStarted != FALSE)
    {
        va_end(args);
    }

    return dwError;
}

DWORD
VmDirStringNPrintFA(
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

    if (maxSize > destinationSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    va_start(args, pszFormat);
    bVaStarted = TRUE;

    if ( vsnprintf_s(
             pDestination, destinationSize, maxSize, pszFormat, args ) < 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    if(bVaStarted != FALSE)
    {
        va_end(args);
    }

    return dwError;
}

DWORD
VmDirStringToTokenList(
    PCSTR pszStr,
    PCSTR pszDelimiter,
    PVMDIR_STRING_LIST *ppStrList
    )
{
    DWORD       dwError = 0;
    PSTR        pszToken = NULL;
    PSTR        pszLocal = NULL;
    PSTR        pszSavePtr = NULL;
    PVMDIR_STRING_LIST  pList = NULL;

    if ( IsNullOrEmptyString(pszStr) || IsNullOrEmptyString(pszDelimiter) || ppStrList == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringListInitialize(&pList, 10);
    BAIL_ON_VMDIR_ERROR(dwError);

    // make a local copy
    dwError = VmDirAllocateStringA(
                pszStr,
                &pszLocal);
    BAIL_ON_VMDIR_ERROR(dwError);

    for ( pszToken = VmDirStringTokA(pszLocal, pszDelimiter, &pszSavePtr);
          pszToken;
          pszToken=VmDirStringTokA(NULL, pszDelimiter, &pszSavePtr))
    {
        dwError = VmDirStringListAddStrClone (pszToken, pList);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppStrList = pList;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocal);

    return dwError;

error:
    VmDirStringListFree(pList);
    goto cleanup;
}

#endif //#ifdef _WIN32

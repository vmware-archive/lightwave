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
VmDirAllocateStringW(
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
VmDirAllocateStringWFromA(
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
VmDirAllocateStringAFromW(
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
VmDirAllocateStringPrintfV(
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

LONG
VmDirStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    )
{
    return LwRtlCStringCompare(pszStr1, pszStr2, bIsCaseSensitive);
}

LONG
VmDirStringNCompareA(
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
    return strlen(pszStr);
}

PSTR
VmDirStringChrA(
   PCSTR str,
   int c
)
{
    return strchr( str, c );
}

PSTR
VmDirStringRChrA(
   PCSTR str,
   int c
)
{
    return strrchr(str, c);
}

PSTR
VmDirStringTokA(
   PSTR strToken,
   PCSTR strDelimit,
   PSTR* context
)
{
    return strtok_r( strToken, strDelimit, context );
}

PSTR
VmDirStringStrA(
   PCSTR str,
   PCSTR strSearch
)
{
    return strstr( str, strSearch );
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
    DWORD dwError = 0;
    size_t count = 0;

    if (!strDestination || !strSource)
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

    strncpy(strDestination, strSource, count);

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
    DWORD dwError = 0;
    if (!strDest || !strSource)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (count + 1 > numberOfElements)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    strncpy(strDest, strSource, count);

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
    DWORD dwError = 0;
    size_t count = 0;
    size_t cchSource = 0;

    if (!strDestination || !strSource)
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

    strncat(strDestination, strSource, cchSource + 1);

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
    DWORD dwError = 0;
    size_t count = 0;

    if (!strDestination || !strSource)
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

    strncat(strDestination, strSource, number);

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
    return strtol( nptr, endptr, base );
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
    return VmDirStringNCpyA(
        buffer, numberOfElements, strerror(errnum), numberOfElements - 1 );
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

    if (!pDestination)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (vsnprintf(pDestination, destinationSize, pszFormat, args ) < 0)
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

    va_start(args, pszFormat);
    bVaStarted = TRUE;

    if (!pDestination || maxSize > destinationSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (vsnprintf(
             pDestination, maxSize, pszFormat, args ) < 0)
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

VOID
VmDirStringTrimSpace(
    PSTR    pszStr
    )
{
    size_t  len = 0;
    size_t  start = 0;
    size_t  end = 0;
    size_t  i = 0;
    size_t  j = 0;

    if (pszStr)
    {
        len = VmDirStringLenA(pszStr);
        if (len > 0)
        {
            for (start = 0; start < len && VMDIR_ASCII_SPACE(pszStr[start]); start++);
            for (end = len - 1; end > 0 && VMDIR_ASCII_SPACE(pszStr[end]); end--);

            for (i = start; i <= end; i++, j++)
            {
                pszStr[j] = pszStr[i];
            }
            pszStr[j] = '\0';
        }
    }
}

/*
 * remove heading/trailing spaces
 * compact consecutive spaces into a single one
 */
VOID
VmdDirNormalizeString(
    PSTR    pszStr
    )
{
    size_t  len = 0;
    size_t  i = 0;
    size_t  j = 0;

    if (pszStr)
    {
        len = VmDirStringLenA(pszStr);

        for (i = 0, j = 0; i < len; i++)
        {
            if (VMDIR_ASCII_SPACE(pszStr[i]) &&
                (i == 0 || VMDIR_ASCII_SPACE(pszStr[i-1])))
            {
                continue;
            }

            pszStr[j++] = pszStr[i];
        }
        pszStr[j] = '\0';

        // handle last space if exists
        if (j > 0 && VMDIR_ASCII_SPACE(pszStr[j-1]))
        {
            pszStr[j-1] = '\0';
        }
        else
        {
            pszStr[j] = '\0';
        }
    }
}

/*
 *  does NOT return empty string token.
 *  say pszStr = "(A;;RP;;;MYSID)" and pszDelimiter = ";"
 *  return pList->pStringList[0] = "(A"
 *         pList->pStringList[1] = "RP"
 *         pList->pStringList[2] = "MYSID)"
 */
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

/*
 *  return empty string token.
 *  say pszStr = "(A;;RP;;;MYSID)" and pszDelimiter = ";"
 *  return pList->pStringList[0] = "(A"
 *         pList->pStringList[1] = ""
 *         pList->pStringList[2] = "RP"
 *         pList->pStringList[3] = ""
 *         pList->pStringList[4] = ""
 *         pList->pStringList[5] = "MYSID)"
 */
DWORD
VmDirStringToTokenListExt(
    PCSTR pszStr,
    PCSTR pszDelimiter,
    PVMDIR_STRING_LIST *ppStrList
    )
{
    DWORD       dwError = 0;
    PSTR        pszToken = NULL;
    PSTR        pszLocal = NULL;
    PSTR        pszHead = NULL;
    SIZE_T      dwSize = 0;
    PVMDIR_STRING_LIST  pList = NULL;

    if ( IsNullOrEmptyString(pszStr) || IsNullOrEmptyString(pszDelimiter) || ppStrList == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwSize = VmDirStringLenA(pszDelimiter);

    dwError = VmDirStringListInitialize(&pList, 10);
    BAIL_ON_VMDIR_ERROR(dwError);

    // make a local copy
    dwError = VmDirAllocateStringA(
                pszStr,
                &pszLocal);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszHead = pszLocal;
    while ((pszToken = strstr(pszHead, pszDelimiter)) != NULL)
    {
        *pszToken = '\0';
        dwError = VmDirStringListAddStrClone (pszHead, pList);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszHead = pszToken + dwSize;
    }

    dwError = VmDirStringListAddStrClone (pszHead, pList);

    *ppStrList = pList;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocal);

    return dwError;

error:
    VmDirStringListFree(pList);
    goto cleanup;
}

#endif //#ifndef _WIN32


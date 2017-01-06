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
#define strcasecmp _stricmp
#endif

#define NSECS_PER_MSEC        1000000

int
VMCAStringCompareA(
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
            result = strcasecmp(pszStr1, pszStr2);
        }
    }
    return result;
}

SIZE_T
VMCAStringLenA(
    PCSTR pszStr
    )
{
    return strlen(pszStr);
}

#ifndef _WIN32

ULONG
VMCAAllocateStringWFromA(
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
VMCAAllocateStringAFromW(
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

BOOL
VMCAStringIsEqualW(
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    BOOLEAN bIsCaseSensitive
    )
{
    BOOL result = FALSE;
    if ( ( pwszStr1 == NULL ) && (pwszStr2 == NULL) )
    {
        result = TRUE; // NULL == NULL
    }
    else if ( pwszStr1 == NULL )
    {
        result = FALSE; // NULL < not-NULL
    }
    else if ( pwszStr2 == NULL )
    {
        result = FALSE; // NOT-NULL > NULL
    }
    else
    {
        result = LwRtlWC16StringIsEqual (
                            pwszStr1,
                            pwszStr2,
                            bIsCaseSensitive
                            );
    }

    return result;
}

/* Fixed time string comparision no matter what pszTheirs value is */
BOOLEAN
VMCAIsValidSecret(
    PWSTR pszTheirs,
    PWSTR pszOurs
    )
{
    ULONG ret = 0;
    SIZE_T  len1 = 0;
    SIZE_T  len2 = 0;
    int  i = 0;
    PWSTR p = NULL;

    if (pszOurs == NULL || VMCAGetStringLengthW(pszOurs, &len1) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (pszTheirs == NULL || VMCAGetStringLengthW(pszTheirs, &len2) != ERROR_SUCCESS ||
        len2 != len1)
    {
        ret = 1;
        p = pszOurs;
    } else
    {
        p = pszTheirs;
    }

    for (i = (int)len1 - 1; i >= 0; i--)
       ret |= p[i] ^ pszOurs[i];
    return ret == 0;
}

DWORD
VMCAStringNCpyA(
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
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (count > numberOfElements)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    strncpy(strDestination, strSource, count);

error:

    return dwError;
}

int
VMCAStringNCompareA(
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

PSTR
VMCAStringChrA(
    PCSTR str,
    int c
    )
{
    return strchr( str, c );
}

PSTR
VMCAStringTokA(
   PSTR strToken,
   PCSTR strDelimit,
   PSTR* context
)
{
    return strtok_r( strToken, strDelimit, context );
}

DWORD
VMCAStringCountSubstring(
    PSTR pszHaystack,
    PCSTR pszNeedle,
    int** ppnCount
)
{
    DWORD dwError = 0;
    PSTR tmp = NULL;
    int *pnCount = NULL;

    if (!pszHaystack || !pszNeedle || !ppnCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(sizeof(int), (PVOID*) &pnCount);
    BAIL_ON_VMCA_ERROR(dwError);

    tmp = pszHaystack;
    (*pnCount) = 0;
    while ( (tmp = strstr(tmp, pszNeedle)) )
    {
        (*pnCount)++;
        tmp++;
    }

    *ppnCount = pnCount;
cleanup:
    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY(pnCount);
    goto cleanup;
}

DWORD
VMCAStringCatA(
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
        BAIL_ON_VMCA_ERROR(dwError);
    }

    cchSource = strlen(strSource);
    count = strlen(strDestination) + cchSource + 1;
    if (count > numberOfElements)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    strncat(strDestination, strSource, cchSource + 1);

error:

    return dwError;
}

DWORD
VMCAGetUTCTimeString(PSTR *pszTimeString)
{
    struct      timespec tspec = {0};
    struct      tm mytm = {0};
    PSTR        szDateTime = NULL;
    DWORD       dwError = 0;

    if (!pszTimeString)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR_NO_LOG(dwError);
    }

#ifndef __MACH__
    clock_gettime(CLOCK_REALTIME, &tspec);
#endif
    gmtime_r(&tspec.tv_sec, &mytm);

    dwError = VMCAAllocateStringPrintfA(
                        &szDateTime,
                        "%4d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
                        mytm.tm_year+1900,
                        mytm.tm_mon+1,
                        mytm.tm_mday,
                        mytm.tm_hour,
                        mytm.tm_min,
                        mytm.tm_sec,
                        tspec.tv_nsec/NSECS_PER_MSEC);
    BAIL_ON_VMCA_ERROR_NO_LOG(dwError);

    *pszTimeString = szDateTime;
cleanup:
    return dwError;
error:
    VMCA_SAFE_FREE_STRINGA(szDateTime);
    goto cleanup;
}

#endif

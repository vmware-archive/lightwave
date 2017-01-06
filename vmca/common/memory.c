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
#pragma warning(disable : 4996 4995)
#endif

DWORD
VMCAAllocateMemory(
    DWORD dwSize,
    PVOID * ppMemory
    )
{
    DWORD dwError = 0;
    void* pMemory = NULL;

    if (!ppMemory || !dwSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pMemory = calloc(1, dwSize);
    if (!pMemory)
    {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }

cleanup:
    *ppMemory = pMemory;

    return dwError;

error:
    VMCAFreeMemory(pMemory);
    pMemory = NULL;

    goto cleanup;
}

DWORD
VMCAReallocateMemory(
    PVOID        pMemory,
    PVOID*       ppNewMemory,
    DWORD        dwSize
    )
{
    DWORD       dwError = 0;
    void*       pNewMemory = NULL;

    if (!ppNewMemory)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pMemory)
    {
        pNewMemory = realloc(pMemory, dwSize);
    }
    else
    {
        dwError = VMCAAllocateMemory(dwSize, &pNewMemory);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!pNewMemory)
    {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    *ppNewMemory = pNewMemory;

cleanup:

    return dwError;

error:

    goto cleanup;
}

VOID
VMCAFreeMemory(
    PVOID pMemory
    )
{
    if (pMemory)
    {
        free(pMemory);
    }

    return;
}

DWORD
VMCAAllocateStringWithLengthA(
    RP_PCSTR pszString,
    DWORD dwSize,
    RP_PSTR * ppszString
    )
{
    DWORD dwError = 0;
    PSTR pszNewString = NULL;

    if (!ppszString || (DWORD)(dwSize + 1) == 0) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    pszNewString = malloc(dwSize + 1);
    if (!pszNewString) {
        dwError = VMCA_OUT_MEMORY_ERR;
        BAIL_ON_ERROR(dwError);
    }
    memcpy(pszNewString, pszString, dwSize);
    pszNewString[dwSize] = 0;
    *ppszString = pszNewString;

cleanup:
    return dwError;
error:
    goto cleanup;
}


DWORD
VMCAAllocateStringA(
    RP_PCSTR pszString,
    RP_PSTR * ppszString
    )
{
    size_t dwLen;

    if (!pszString || !ppszString)
    {
        if (ppszString) { *ppszString = NULL; }
        return 0;
    }
    dwLen = strlen(pszString);
    if (dwLen != (DWORD)dwLen) {
        return ERROR_BUFFER_OVERFLOW;
    }
    return VMCAAllocateStringWithLengthA(pszString, (DWORD) strlen(pszString), ppszString);
}


DWORD
VMCAAllocateStringPrintfA(
    RP_PSTR* ppszString,
    RP_PCSTR pszFormat,
    ...
    )
{
    ULONG ulError = 0;

#ifndef _WIN32
    if (!ppszString || !pszFormat)
    {
        ulError = ERROR_INVALID_PARAMETER;
    }
    else
    {
        va_list args;
        va_start(args, pszFormat);


        ulError = LwNtStatusToWin32Error(
                        LwRtlCStringAllocatePrintfV(
                                ppszString,
                                pszFormat,
                                args));
        va_end(args);
    }

	return ulError;

#else
	if (!ppszString || !pszFormat)
	{
		ulError = ERROR_INVALID_PARAMETER;
	}
	else
	{
		int len = 0;
        va_list args;
        PSTR pTempStr = NULL;
        va_start(args, pszFormat);

        len = _vscprintf(pszFormat, args) + 1 ;

        ulError = VMCAAllocateMemory((DWORD)len, (PVOID*)&pTempStr);
        BAIL_ON_ERROR(ulError);

        if (vsprintf( pTempStr, pszFormat, args ) < 0) {
            ulError = ERROR_INVALID_PARAMETER;
            BAIL_ON_ERROR(ulError);
        }

    *ppszString = pTempStr;
    pTempStr = NULL;
    va_end(args);

error :
    if (pTempStr != NULL) {
        VMCAFreeStringA(pTempStr);
    }
  }
#endif
    return ulError;
}

VOID
VMCAFreeStringA(
    RP_PSTR pszString
    )
{
    VMCAFreeMemory(pszString);
}

DWORD
VMCAAllocateStringW(
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
#ifdef _WIN32
        PWSTR pwszNewString = NULL;
        size_t len = wcslen(pwszSrc);
        dwError = VMCAAllocateMemory(
                        (DWORD) (len + 1)*sizeof(WCHAR), 
                        (PVOID *)&pwszNewString
                        );
        BAIL_ON_VMCA_ERROR(dwError);

        wcscpy_s(pwszNewString, (len + 1), pwszSrc);

        *ppwszDst = pwszNewString;
#else
		dwError = LwNtStatusToWin32Error(
						LwRtlWC16StringDuplicate(ppwszDst, pwszSrc));
        BAIL_ON_VMCA_ERROR(dwError);
#endif
    }
error:
    return dwError;
}


VOID
VMCAFreeStringW(
    RP_PWSTR pszString
    )
{
    VMCAFreeMemory(pszString);
}

DWORD
ConvertAnsitoUnicodeString(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    )
{
    DWORD dwError = 0;
#ifdef _WIN32
	PSTR   pszNewString = NULL;
#endif

    if (!pszSrc || !ppwszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    else
    {
#ifdef _WIN32
		// allocate space for new string then pass addr to output
		// double the original PSTR size for WPSTR
		ULONG srcLen =  (ULONG)strlen(pszSrc)+1;
		ULONG destLen = (srcLen+1)*2;
		dwError = VMCAAllocateMemory((DWORD) destLen, (PVOID*)&pszNewString);
		BAIL_ON_ERROR(dwError);

		if (0 == MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, (LPWSTR)pszNewString, destLen))
		{
			dwError = GetLastError();
			BAIL_ON_ERROR(dwError);
		}
		*ppwszDst = (LPWSTR)pszNewString;
cleanup:
	return dwError;

error:
	VMCAFreeMemory(pszNewString);
	goto cleanup;
#else
        dwError = LwNtStatusToWin32Error(
                        LwRtlWC16StringAllocateFromCString(ppwszDst, pszSrc));
#endif
    }
	return dwError;
}

DWORD
ConvertUnicodetoAnsiString(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    )
{
    DWORD dwError = 0;
#ifdef _WIN32
	PSTR   pszNewString = NULL;
#endif

    if (!pwszSrc || !ppszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    else
    {
#ifdef _WIN32
		ULONG srcLen =  (ULONG)wcslen(pwszSrc)+1;
		ULONG destLen = srcLen*2;
		dwError = VMCAAllocateMemory((DWORD) destLen, (PVOID*)&pszNewString);
		BAIL_ON_ERROR(dwError);

		if (0 == WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszNewString, destLen, NULL, NULL ))
		{
			dwError = GetLastError();
			BAIL_ON_ERROR(dwError);
		}
		*ppszDst = pszNewString;
cleanup:
		return dwError;

error:
		VMCAFreeMemory(pszNewString);
		goto cleanup;

#else
		dwError = LwNtStatusToWin32Error(
						LwRtlCStringAllocateFromWC16String(ppszDst, pwszSrc));
#endif
	}
	return dwError;
}

DWORD
VMCAGetStringLengthW(
    PCWSTR  pwszStr,
    PSIZE_T pLength
    )
{
    DWORD dwError = 0;

    if (!pwszStr || !pLength)
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    else
    {
#ifdef _WIN32
		*pLength = wcslen(pwszStr);
#else
		*pLength = LwRtlWC16StringNumChars(pwszStr);
#endif
    }

    return dwError;
}

VOID
VMCAFreeStringArrayA(
    PSTR* ppszStrings,
    DWORD dwCount
    )
{
    if (ppszStrings)
    {
        DWORD dwCnt = 0;

        for (dwCnt = 0; dwCnt < dwCount; dwCnt++)
        {
            if (ppszStrings[dwCnt])
            {
              VMCAFreeStringA(ppszStrings[dwCnt]);
            }
        }

        VMCAFreeMemory(ppszStrings);
    }
}


void
VMCASetBit(unsigned long *flag, int bit)
{
	*flag |= 1<< bit;
}

int
VMCAisBitSet(unsigned long flag, int bit)
{
	return (flag & ( 1 << bit));
}


void
VMCAClearBit(unsigned long flag, int bit)
{
	 flag &= ~(1 << bit);
}

void
VMCAToggleBit(unsigned long flag, int bit)
{
	flag ^= (1 << bit);
}


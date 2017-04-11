/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : string.c
 *
 * Abstract :
 *
 *            VMware Directory Service
 *
 *            Common Utilities (Client & Server)
 *
 *            String Management
 *
 */

#include "includes.h"

#ifdef _WIN32

//ULONG
//VMCAAllocateStringW(
//    PCWSTR pwszSrc,
//    PWSTR* ppwszDst
//    )
//{
//    DWORD dwError = 0;
//    LPWSTR pwszNewString = NULL;
//    size_t len = 0;
//
//    if (!pwszSrc || !ppwszDst)
//    {
//        dwError = ERROR_INVALID_PARAMETER;
//        BAIL_ON_VMCA_ERROR(dwError);
//    }
//    *ppwszDst = NULL;
//
//    len = wcslen(pwszSrc);
//    dwError =
//        VMCAAllocateMemory(
//            (DWORD) (len + 1)*sizeof(WCHAR), (PVOID *)&pwszNewString
//        );
//    BAIL_ON_VMCA_ERROR(dwError);
//
//    wcscpy_s(pwszNewString, (len + 1), pwszSrc);
//
//    *ppwszDst = pwszNewString;
//    pwszNewString = NULL;
//
//error:
//    VMCA_SAFE_FREE_MEMORY(pwszNewString);
//    return dwError;
//}

ULONG
VMCAAllocateStringWFromA(
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
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!ppwszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    *ppwszDst = NULL;

    dwSize = MultiByteToWideChar( CP_UTF8, 0, pszSrc, -1, NULL, 0 );
    if (!dwSize)
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError =
        VMCAAllocateMemory(
            dwSize * sizeof(WCHAR), (PVOID *)&pszUnicodeString
        );
    BAIL_ON_VMCA_ERROR(dwError);

    dwSize =
        MultiByteToWideChar(
            CP_UTF8, 0, pszSrc, -1, pszUnicodeString, dwSize
        );
    if (!dwSize)
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR(dwError);
    }

    *ppwszDst = pszUnicodeString;
    pszUnicodeString = NULL;

error:

    VMCA_SAFE_FREE_MEMORY(pszUnicodeString);
    return dwError;
}

ULONG
VMCAAllocateStringAFromW(
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
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!ppszDst)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    *ppszDst = NULL;

    dwSize =
        WideCharToMultiByte(
            CP_UTF8, 0, pwszSrc, -1, NULL, 0, NULL, NULL
        );
    if(dwSize != 0)
    {
        dwError = VMCAAllocateMemory( dwSize+1, (PVOID *)&pszAnsiString );
        BAIL_ON_VMCA_ERROR(dwError);

        dwSize =
            WideCharToMultiByte(
                CP_UTF8, 0, pwszSrc, -1, pszAnsiString, dwSize, NULL, NULL
            );
        if (!dwSize)
        {
            dwError = GetLastError();
            BAIL_ON_VMCA_ERROR(dwError);
        }

        *ppszDst = pszAnsiString;
        pszAnsiString = NULL;
    }

error:

    VMCA_SAFE_FREE_MEMORY(pszAnsiString);
    return dwError;
}

BOOL
VMCAStringIsEqualW(
    PCWSTR pwszStr1,
    PCWSTR pwszStr2,
    BOOLEAN bIsCaseSensitive
    )
{
    return !VMCAStringCompareW (pwszStr1, pwszStr2, bIsCaseSensitive);
}

int
VMCAStringCompareW(
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


ULONG
VMCAAllocateStringPrintfV(
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
        BAIL_ON_VMCA_ERROR(dwError);
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
        BAIL_ON_VMCA_ERROR(dwError);
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
        VMCAAllocateMemory( (dwSize*sizeof(CHAR)), (PVOID *)&pszAnsiString );
    BAIL_ON_VMCA_ERROR(dwError);

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
        BAIL_ON_VMCA_ERROR(dwError);
    }

    *ppszStr = pszAnsiString;
    pszAnsiString = NULL;

error:

    VMCA_SAFE_FREE_MEMORY(pszAnsiString);

    return dwError;
}

ULONG
VMCAAllocateStringPrintf(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    )
{
    DWORD dwError = 0;
    va_list args;

    va_start(args, pszFormat);
    dwError = VMCAAllocateStringPrintfV(ppszString, pszFormat, args);
    va_end(args);

    return dwError;
}

//ULONG
//VMCAGetStringLengthW(
//    PCWSTR  pwszStr,
//    PSIZE_T pLength
//    )
//{
//    DWORD dwError = 0;
//
//    if (!pwszStr || !pLength)
//    {
//        dwError = ERROR_INVALID_PARAMETER;
//        BAIL_ON_VMCA_ERROR(dwError);
//    }
//
//    *pLength = wcslen(pwszStr);
//
//error:
//
//    return dwError;
//}

#pragma warning (disable: 4142)
ULONG
VMCAStringCompareA(
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
#pragma warning(default:4142)
int
VMCAStringNCompareA(
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

SIZE_T
VMCAStringLenA(
    PCSTR pszStr
)
{
    return ( pszStr != NULL) ? strlen(pszStr) : 0;
}

PSTR
VMCAStringChrA(
   PCSTR str,
   int c
)
{
    return (PSTR)strchr( str, c );
}

PSTR
VMCAStringRChrA(
   PCSTR str,
   int c
)
{
    return (PSTR)strrchr(str, c);
}

PSTR
VMCAStringTokA(
   PSTR strToken,
   PCSTR strDelimit,
   PSTR* context
)
{
    return strtok_s( strToken, strDelimit, context );
}

PSTR
VMCAStringStrA(
   PCSTR str,
   PCSTR strSearch
)
{
    return (PSTR)strstr( str, strSearch );
}

DWORD
VMCAStringCpyA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
)
{
    DWORD dwError = ERROR_SUCCESS;
    if ( strcpy_s( strDestination, numberOfElements, strSource) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }
error:
    return dwError;
}

DWORD
VMCAStringNCpyA(
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
        BAIL_ON_VMCA_ERROR(dwError);
    }
error:
    return dwError;
}

DWORD
VMCAStringCatA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
)
{
    DWORD dwError = ERROR_SUCCESS;
    if ( strcat_s( strDestination, numberOfElements, strSource) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }
error:
    return dwError;
}


DWORD
VMCAStringErrorA(
   PSTR buffer,
   size_t numberOfElements,
   int errnum
)
{
    DWORD dwError = ERROR_SUCCESS;
    if ( strerror_s( buffer, numberOfElements, errnum) != 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }
error:
    return dwError;
}

PSTR
VMCACaselessStrStrA(
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
VMCAStringPrintFA(
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
        BAIL_ON_VMCA_ERROR(dwError);
    }

error:
    if(bVaStarted != FALSE)
    {
        va_end(args);
    }

    return dwError;
}

DWORD
VMCAStringNPrintFA(
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
        BAIL_ON_VMCA_ERROR(dwError);
    }

error:
    if(bVaStarted != FALSE)
    {
        va_end(args);
    }

    return dwError;
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

    if (pszTheirs == NULL || VMCAGetStringLengthW(pszTheirs, &len2) != ERROR_SUCCESS || len2 != len1)
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

// Get Current UTC time in ISO 8601 format
DWORD
VMCAGetUTCTimeString(PSTR *pszTimeString)
{
    SYSTEMTIME st = {0};
    PSTR szDateTime = NULL;
    DWORD dwError = 0;

    if (!pszTimeString)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR_NO_LOG(dwError);
    }

    GetSystemTime(&st);
    dwError = VMCAAllocateStringPrintf(
                        &szDateTime,
                        "%4d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
                        st.wYear,
                        st.wMonth,
                        st.wDay,
                        st.wHour,
                        st.wMinute,
                        st.wSecond,
                        st.wMilliseconds);
    BAIL_ON_VMCA_ERROR_NO_LOG(dwError);

    *pszTimeString = szDateTime;
cleanup:
    return dwError;
error:
    VMCA_SAFE_FREE_STRINGA(szDateTime);
    goto cleanup;
}

#endif //#ifdef _WIN32

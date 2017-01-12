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

static
DWORD
vmkdcVsnprintf(
    PSTR*    ppszOut,
    PCSTR    pszFormat,
    va_list  args
    );

DWORD
VmKdcAllocateMemory(
    size_t   dwSize,
    PVOID*   ppMemory
    )
{
    DWORD dwError = 0;
    void* pMemory = NULL;

    if (!ppMemory || !dwSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    pMemory = calloc(1, dwSize);
    if (!pMemory)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

cleanup:
    *ppMemory = pMemory;

    return dwError;

error:
    VMKDC_SAFE_FREE_MEMORY(pMemory);
    pMemory = NULL;

    goto cleanup;
}

DWORD
VmKdcReallocateMemory(
    PVOID        pMemory,
    PVOID*       ppNewMemory,
    size_t       dwSize
    )
{
    DWORD dwError = 0;
    void*    pNewMemory = NULL;

    if (!ppNewMemory)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (pMemory)
    {
        pNewMemory = realloc(pMemory, dwSize);
    }
    else
    {
        dwError = VmKdcAllocateMemory(dwSize, &pNewMemory);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (!pNewMemory)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    *ppNewMemory = pNewMemory;

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmKdcCopyMemory(
    PVOID   pDestination,
    size_t  destinationSize,
    PCVOID  pSource,
    size_t  maxCount
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if (!pDestination || !pSource || maxCount > destinationSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

#ifndef _WIN32
    memcpy(pDestination, pSource, maxCount);

#else

    if (memcpy_s( pDestination, destinationSize, pSource, maxCount ) != 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

#endif
error:
    return dwError;
}

DWORD
VmKdcReallocateMemoryWithInit(
    PVOID         pMemory,
    PVOID*        ppNewMemory,
    size_t        dwNewSize,
    size_t        dwOldSize
    )
{
    DWORD    dwError = 0;

    if (dwNewSize < dwOldSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcReallocateMemory(pMemory, ppNewMemory, dwNewSize);

    if (dwError == 0)
    {
        memset(((char*)(*ppNewMemory)) + dwOldSize, 0, dwNewSize - dwOldSize);
    }

error:

    return dwError;
}

VOID
VmKdcFreeMemory(
    PVOID   pMemory
    )
{
    if (pMemory)
    {
        free(pMemory);
    }

    return;
}

DWORD
VmKdcAllocateStringAVsnprintf(
    PSTR*   ppszOut,
    PCSTR   pszFormat,
    ...
    )
{
    DWORD   dwError = 0;
    BOOLEAN bVAEnd = FALSE;
    va_list args;
    //PSTR str1, str2;


    if (!ppszOut || !pszFormat)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    va_start(args, pszFormat);
    bVAEnd = TRUE;

    //TODO: Clean this up later
    //str1 = va_arg( args, char*);
    //str2 = va_arg( args, char*);

    dwError = vmkdcVsnprintf(
                ppszOut,
                pszFormat,
                args);
    BAIL_ON_VMKDC_ERROR(dwError);

cleanup:

    if (bVAEnd)
    {
        va_end(args);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmKdcAllocateStringA(
    PCSTR   pszString,
    PSTR*   ppszString
    )
{
    DWORD  dwError = 0;
    PSTR   pszNewString = NULL;
    size_t dwLen = 0;

    if (!pszString || !ppszString)
    {
        if (ppszString) { *ppszString = NULL; }
        return 0;
    }

    dwLen = VmKdcStringLenA(pszString);
    // + 1 for \'0'
    dwError = VmKdcAllocateMemory(dwLen + 1, (PVOID*)&pszNewString);
    BAIL_ON_VMKDC_ERROR(dwError);

#ifndef _WIN32
    memcpy(pszNewString, pszString, dwLen);
#else
    memcpy_s(pszNewString, (dwLen + 1), pszString, dwLen);
#endif
    *ppszString = pszNewString;

cleanup:

    return dwError;

error:

    VMKDC_SAFE_FREE_MEMORY(pszNewString);

    goto cleanup;
}

VOID
VmKdcFreeStringA(
    PSTR pszString
    )
{
    VMKDC_SAFE_FREE_MEMORY(pszString);

    return;
}

/*
 * free array of PSTR
 * assume array stop at PSTR == NULL
 */
VOID
VmKdcFreeStringArrayA(
    PSTR* ppszString
    )
{
    DWORD dwCnt = 0;

    if (!ppszString)
    {
        return;
    }

    for (dwCnt = 0; ppszString[dwCnt]; dwCnt++)
    {
        VmKdcFreeStringA(ppszString[dwCnt]);
    }

    return;
}

static
DWORD
vmkdcVsnprintf(
    PSTR*    ppszOut,
    PCSTR    pszFormat,
    va_list args
    )
{
#ifndef _WIN32
    DWORD      dwError = 0;
    int        iInitSize = 10;
    int        iFinalSize = 1;
    PSTR       pszBuf = NULL;
    va_list    argsNext;

    va_copy(argsNext, args);

    dwError = VmKdcAllocateMemory(
            iInitSize,
            (PVOID*)&pszBuf);
    BAIL_ON_VMKDC_ERROR(dwError);

    iFinalSize = vsnprintf(
            pszBuf,
            iInitSize,
            pszFormat,
            args);

    if (iFinalSize < 0)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (iInitSize - iFinalSize <= 0)
    {
        int iRealSize = 0;

        dwError = VmKdcReallocateMemory(
                pszBuf,
                (PVOID*)&pszBuf,
                iFinalSize + 1);
        BAIL_ON_VMKDC_ERROR(dwError);

        iRealSize = vsnprintf(
                pszBuf,
                iFinalSize + 1,
                pszFormat,
                argsNext);

        if (iRealSize < 0 || iRealSize != iFinalSize)
        {
            dwError = ERROR_NO_MEMORY;
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    *ppszOut = pszBuf;

cleanup:

    va_end(argsNext);

    return dwError;

error:

    VMKDC_SAFE_FREE_MEMORY(pszBuf);

    goto cleanup;

#else // _WIN32
    DWORD dwError = 0;
    int iSize = 0;
    LPSTR pszAnsiString = NULL;

    if (!ppszOut || !pszFormat)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    *ppszOut = NULL;

    /*
    MSDN:
        If execution is allowed to continue, the functions return -1 and
        set errno to EINVAL.
    */
    iSize = _vscprintf(pszFormat, args);
    if ( iSize <= -1)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /*
    MSDN:
        _vscprintf returns the number of characters that would be generated
        if the string pointed to by the list of arguments was printed or
        sent to a file or buffer using the specified formatting codes.
        The value returned does not include the terminating null character.
    */
    iSize += 1;

    dwError =
        VmKdcAllocateMemory( (iSize*sizeof(CHAR)), (PVOID *)&pszAnsiString );
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
    MSDN:
        return the number of characters written, not including
        the terminating null, or a negative value if an output error occurs.
    */
    // we should never truncate since we measured the buffer ....
    if( vsnprintf_s( pszAnsiString, iSize, _TRUNCATE, pszFormat, args) < 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    *ppszOut = pszAnsiString;
    pszAnsiString = NULL;

error:

    VMKDC_SAFE_FREE_MEMORY(pszAnsiString);

    return dwError;

#endif
}

ULONG
VmKdcLengthRequiredSid(
    IN UCHAR SubAuthorityCount
    )
{
#ifndef _WIN32
    return RtlLengthRequiredSid(SubAuthorityCount);
#else
    return GetSidLengthRequired(SubAuthorityCount);
#endif
}

ULONG
VmKdcInitializeSid(
    PSID Sid,
    PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    UCHAR SubAuthorityCount
    )
{
#ifndef _WIN32
    return LwNtStatusToWin32Error(RtlInitializeSid(Sid,
                                                  IdentifierAuthority,
                                                  SubAuthorityCount));
#else
    DWORD dwError = ERROR_SUCCESS;
    if ( InitializeSid(Sid, IdentifierAuthority, SubAuthorityCount ) == 0 )
    {
        dwError = GetLastError();
    }
    return dwError;
#endif
}

ULONG
VmKdcSetSidSubAuthority(
    PSID pSid,
    DWORD nSubAuthority,
    DWORD subAuthorityValue
    )
{
    DWORD dwError = ERROR_SUCCESS;
#ifdef _WIN32
    PDWORD pSubAuthority = NULL;
    PUCHAR pSubAuth = NULL;
#endif

    if(pSid == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

#ifndef _WIN32

    if(pSid->SubAuthorityCount <= nSubAuthority)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    pSid->SubAuthority[nSubAuthority] = subAuthorityValue;

#else

    if( IsValidSid(pSid) == 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    pSubAuth = GetSidSubAuthorityCount( pSid );

    if( *pSubAuth <= nSubAuthority )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    pSubAuthority = GetSidSubAuthority( pSid, nSubAuthority );

    *pSubAuthority = subAuthorityValue;

#endif

error:
    return dwError;
}

ULONG
VmKdcAllocateSidFromCString(
    PCSTR pszSidString,
    PSID* ppSid
    )
{
#ifndef _WIN32
    return LwNtStatusToWin32Error(
                RtlAllocateSidFromCString(ppSid,
                                         (PCSTR)pszSidString));
#else
    DWORD dwError = ERROR_SUCCESS;
    PSID originalSid = NULL;
    PSID cloneSid = NULL;
    DWORD sidLength = 0;

    if (!pszSidString || !ppSid)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    *ppSid = NULL;

    /*
    MSDN:
        Sid [out]
        A pointer to a variable that receives a pointer to
        the converted SID.
        To free the returned buffer, call the LocalFree function.
    */
    if( ConvertStringSidToSidA( pszSidString, &originalSid ) == 0 )
    {
        dwError = GetLastError();
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    // callers of this function are prepared to free the
    // sid, with regular VMKdcFreeMemory, but on windows
    // original Sid needs to be freed with LocalFree...
    // therefore we will clone the Sid being returned ...
    if( IsValidSid(originalSid) == 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    sidLength = GetLengthSid( originalSid);

    dwError = VmKdcAllocateMemory( sidLength, ((PVOID*)(&cloneSid)));
    BAIL_ON_VMKDC_ERROR(dwError);

    if( CopySid( sidLength, cloneSid, originalSid ) == 0 )
    {
        dwError = GetLastError();
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    *ppSid = cloneSid;
    cloneSid = NULL;

error:

    VMKDC_SAFE_FREE_MEMORY(cloneSid);
    if( originalSid != NULL )
    {
        LocalFree(originalSid);
        originalSid = NULL;
    }

    return dwError;

#endif
}

ULONG
VmKdcAllocateCStringFromSid(
    PSTR* ppszStringSid,
    PSID pSid
    )
{
#ifndef _WIN32
    return LwNtStatusToWin32Error(
                RtlAllocateCStringFromSid(ppszStringSid,pSid));
#else
    DWORD dwError = ERROR_SUCCESS;
    PSTR origSid = NULL;
    PSTR cloneSid = NULL;

    if (!ppszStringSid || !pSid)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    *ppszStringSid = NULL;

    /*
    MSDN:
        StringSid [out]
            A pointer to a variable that receives a pointer to a
            null-terminated SID string.
            To free the returned buffer, call the LocalFree function.
    */
    if ( ConvertSidToStringSidA( pSid, &origSid ) == 0 )
    {
        dwError = GetLastError();
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    // callers of this function are prepared to free the
    // string sid, with regular VMKdcFreeString, but on windows
    // original Sid needs to be freed with LocalFree...
    // therefore we will clone the string being returned ...
    dwError = VmKdcAllocateStringA(origSid, &cloneSid);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppszStringSid = cloneSid;
    cloneSid = NULL;

error :

    VMKDC_SAFE_FREE_STRINGA(cloneSid);
    if( origSid != NULL )
    {
        LocalFree(origSid);
        origSid = NULL;
    }

    return dwError;

#endif
}


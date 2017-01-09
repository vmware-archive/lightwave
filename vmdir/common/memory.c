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

DWORD
VmDirAllocateMemory(
    size_t   dwSize,
    PVOID*   ppMemory
    )
{
    DWORD dwError = 0;
    void* pMemory = NULL;

    if (!ppMemory || !dwSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pMemory = calloc(1, dwSize);
    if (!pMemory)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    *ppMemory = pMemory;

    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pMemory);
    pMemory = NULL;

    VmDirLog( LDAP_DEBUG_ANY, "VmDirAllocateMemory failed, (%u) requested.", dwSize );

    goto cleanup;
}

DWORD
VmDirReallocateMemory(
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
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pMemory)
    {
        pNewMemory = realloc(pMemory, dwSize);
    }
    else
    {
        dwError = VmDirAllocateMemory(dwSize, &pNewMemory);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pNewMemory)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppNewMemory = pNewMemory;

cleanup:

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "VmDirReallocateMemory failed, (%u) requested.", dwSize );

    goto cleanup;
}

DWORD
VmDirCopyMemory(
    PVOID   pDestination,
    size_t  destinationSize,
    const void* pSource,
    size_t  maxCount
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if (!pDestination || !pSource || maxCount > destinationSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#ifndef _WIN32
    memcpy(pDestination, pSource, maxCount);
#else
    if (memcpy_s( pDestination, destinationSize, pSource, maxCount ) != 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
#endif

error:
    return dwError;
}

DWORD
VmDirAllocateAndCopyMemory(
    PVOID   pBlob,
    size_t  iBlobSize,
    PVOID*  ppOutBlob
    )
{
    DWORD   dwError = 0;
    PVOID   pMemory = NULL;

    if (!pBlob || !ppOutBlob || iBlobSize < 1)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // add 1 for safety (VMDIR assuem bervalue is NULL terminated for string value)
    dwError = VmDirAllocateMemory(iBlobSize + 1, &pMemory);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory(pMemory, iBlobSize, pBlob, iBlobSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppOutBlob = pMemory;

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pMemory);

    goto cleanup;
}

DWORD
VmDirReallocateMemoryWithInit(
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
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirReallocateMemory(pMemory, ppNewMemory, dwNewSize);

    if (dwError == 0)
    {
        memset(((char*)(*ppNewMemory)) + dwOldSize, 0, dwNewSize - dwOldSize);
    }

error:

    return dwError;
}

VOID
VmDirFreeMemory(
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
VmDirAllocateStringOfLenA(
    PCSTR   pszSource,
    SIZE_T  sLength,
    PSTR*   ppszDestination
    )
{
    DWORD  dwError = 0;
    PSTR   pszNewString = NULL;

    if (!pszSource || !ppszDestination)
    {
        if (ppszDestination) { *ppszDestination = NULL; }
        return 0;
    }

    //
    // Check if the user is trying to copy more than is available.
    //
    if (strlen(pszSource) < sLength)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(sLength + 1, (PVOID*)&pszNewString);
    BAIL_ON_VMDIR_ERROR(dwError);

#ifndef _WIN32
    memcpy(pszNewString, pszSource, sLength);
#else
    memcpy_s(pszNewString, sLength + 1, pszSource, sLength);
#endif
    *ppszDestination = pszNewString;

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pszNewString);

    goto cleanup;
}

DWORD
VmDirAllocateStringA(
    PCSTR   pszString,
    PSTR*   ppszString
    )
{
    if (!pszString || !ppszString)
    {
        if (ppszString) { *ppszString = NULL; }
        return 0;
    }

    return VmDirAllocateStringOfLenA(pszString, VmDirStringLenA(pszString), ppszString);
}

VOID
VmDirFreeStringA(
    PSTR pszString
    )
{
    VMDIR_SAFE_FREE_MEMORY(pszString);
}

/*
 * free array of PSTR
 * assume array stop at PSTR == NULL
 */
VOID
VmDirFreeStringArrayA(
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
        VmDirFreeStringA(ppszString[dwCnt]);
    }

    return;
}

VOID
VmDirFreeStringArrayW(
    PWSTR* ppwszStrings,
    DWORD  dwCount
    )
{
    if (ppwszStrings)
    {
        DWORD idx = 0;

        for (; idx < dwCount; idx++)
        {
            VMDIR_SAFE_FREE_MEMORY(ppwszStrings[idx]);
        }

        VmDirFreeMemory(ppwszStrings);
    }
}

/*
 * pszString is expected to be a multistring.
 * A multistring consists of a list of zero or more non-empty nul-terminated
 * strings in which the list is terminated with nul.
 *
 * Examples:
 *    A list with one string ("abc"):  abc\0\0
 *    A list with two strings ("abc", "d"): abc\0d\0\0
 *    An empty list:  \0
 *    An INVALID multistring (No string may be empty):   abc\0\0\dce\0\0
 *
 * Note that some believe all multistrings are 'double terminated'; the
 * empty list is not.
 *
 */
DWORD
VmDirAllocateMultiStringA(
    PCSTR   pszString,
    PSTR*   ppszString
    )
{
    DWORD  dwError = 0;
    PCSTR  pszIter = NULL;
    PSTR   pszNewString = NULL;
    size_t dwLen = 0;

    if (!pszString || !ppszString)
    {
        if (ppszString) { *ppszString = NULL; }
        return VMDIR_ERROR_INVALID_PARAMETER;
    }

    pszIter = pszString;
    while (*pszIter != '\0')
    {
        pszIter += VmDirStringLenA(pszIter) + 1;
    }
    dwLen = (pszIter - pszString) + 1;

    dwError = VmDirAllocateMemory(dwLen, (PVOID*)&pszNewString);
    BAIL_ON_VMDIR_ERROR(dwError);

#ifndef _WIN32
    memcpy(pszNewString, pszString, dwLen);
#else
    memcpy_s(pszNewString, dwLen, pszString, dwLen);
#endif
    *ppszString = pszNewString;

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pszNewString);

    goto cleanup;
}

DWORD
VmDirVsnprintf(
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

    dwError = VmDirAllocateMemory(
            iInitSize,
            (PVOID*)&pszBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    iFinalSize = vsnprintf(
            pszBuf,
            iInitSize,
            pszFormat,
            args);

    if (iFinalSize < 0)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (iInitSize - iFinalSize <= 0)
    {
        int iRealSize = 0;

        dwError = VmDirReallocateMemory(
                pszBuf,
                (PVOID*)&pszBuf,
                iFinalSize + 1);
        BAIL_ON_VMDIR_ERROR(dwError);

        iRealSize = vsnprintf(
                pszBuf,
                iFinalSize + 1,
                pszFormat,
                argsNext);

        if (iRealSize < 0 || iRealSize != iFinalSize)
        {
            dwError = ERROR_NO_MEMORY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *ppszOut = pszBuf;

cleanup:

    va_end(argsNext);

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pszBuf);

    goto cleanup;

#else // _WIN32
    DWORD dwError = 0;
    int iSize = 0;
    LPSTR pszAnsiString = NULL;

    if (!ppszOut || !pszFormat)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
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
        BAIL_ON_VMDIR_ERROR(dwError);
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
        VmDirAllocateMemory( (iSize*sizeof(CHAR)), (PVOID *)&pszAnsiString );
    BAIL_ON_VMDIR_ERROR(dwError);

    /*
    MSDN:
        return the number of characters written, not including
        the terminating null, or a negative value if an output error occurs.
    */
    // we should never truncate since we measured the buffer ....
    if( vsnprintf_s( pszAnsiString, iSize, _TRUNCATE, pszFormat, args) < 0 )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszOut = pszAnsiString;
    pszAnsiString = NULL;

error:

    VMDIR_SAFE_FREE_MEMORY(pszAnsiString);

    return dwError;

#endif
}

ULONG
VmDirLengthRequiredSid(
    IN UCHAR SubAuthorityCount
    )
{
    return RtlLengthRequiredSid(SubAuthorityCount);
}

ULONG
VmDirInitializeSid(
    PSID Sid,
    PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    UCHAR SubAuthorityCount
    )
{
    return LwNtStatusToWin32Error(RtlInitializeSid(Sid,
                                                  IdentifierAuthority,
                                                  SubAuthorityCount));
}

ULONG
VmDirSetSidSubAuthority(
    PSID pSid,
    DWORD nSubAuthority,
    DWORD subAuthorityValue
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if(pSid == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#ifdef _WIN32
    dwError = RtlSetRidSid(
                  subAuthorityValue,
                  nSubAuthority,
                  pSid);

#else
    /*
     * TBD: This is the implementation of RtlSetRidSid() which needs to be
     * added to likewise-open-6.1/lwbase.
     */
    if(pSid->SubAuthorityCount <= nSubAuthority)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pSid->SubAuthority[nSubAuthority] = subAuthorityValue;
#endif

error:
    return dwError;
}

ULONG
VmDirAllocateSidFromCString(
    PCSTR pszSidString,
    PSID* ppSid
    )
{
    return LwNtStatusToWin32Error(
                RtlAllocateSidFromCString(ppSid,
                                         (PCSTR)pszSidString));
}

ULONG
VmDirAllocateCStringFromSid(
    PSTR* ppszStringSid,
    PSID pSid
    )
{
    return LwNtStatusToWin32Error(
                RtlAllocateCStringFromSid(ppszStringSid,pSid));
}

VOID
VmDirFreeTypeSpecContent(
    PVMW_TYPE_SPEC specInput,
    DWORD sizeOfArray
    )
{
    DWORD dCounter = 0;
    for (dCounter = 0; dCounter < sizeOfArray; dCounter++){
      switch(specInput[dCounter].type){
        case VMW_IPC_TYPE_UINT32:
          VMDIR_SAFE_FREE_MEMORY (specInput[dCounter].data.pUint32);
          break;
        case VMW_IPC_TYPE_STRING:
          VMDIR_SAFE_FREE_MEMORY (specInput[dCounter].data.pString);
          break;
        case VMW_IPC_TYPE_WSTRING:
          VMDIR_SAFE_FREE_MEMORY (specInput[dCounter].data.pWString);
          break;
        case VMW_IPC_TYPE_BLOB:
          VMDIR_SAFE_FREE_MEMORY (specInput[dCounter].data.pByte);
          break;
        case VMW_IPC_TYPE_BLOB_SIZE:
          VMDIR_SAFE_FREE_MEMORY (specInput[dCounter].data.pUint32);
          break;
        default:
          VMDIR_SAFE_FREE_MEMORY (specInput[dCounter].data.pByte);
          break;
      }
    }
}

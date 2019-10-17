/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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

VOID
VmStringListFreeContent(
    PVM_STRING_LIST pStringList
    )
{
    DWORD i = 0;

    if (pStringList != NULL)
    {
        for (i = 0; i < pStringList->dwCount; ++i)
        {
            VmFreeStringA((PSTR)pStringList->pStringList[i]);
        }
        pStringList->dwCount = 0;
    }
}

VOID
VmStringListFree(
    PVM_STRING_LIST pStringList
    )
{
    if (pStringList != NULL)
    {
        VmStringListFreeContent(pStringList);
        VM_COMMON_SAFE_FREE_MEMORY(pStringList->pStringList);
        VmFreeMemory(pStringList);
    }
}

DWORD
VmStringListInitialize(
    PVM_STRING_LIST *ppStringList,
    DWORD dwInitialCount
    )
{
    DWORD dwError = 0;
    PVM_STRING_LIST pStringList = NULL;
    DWORD dwAllocationSize = 0;

    if (dwInitialCount < 4)
    {
        dwInitialCount = 4; // default to 4
    }

    dwAllocationSize = (dwInitialCount + 1) * sizeof(PSTR);
    if (dwAllocationSize < dwInitialCount)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmAllocateMemory(sizeof(*pStringList), (PVOID *)&pStringList);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmAllocateMemory(
                dwAllocationSize,
                (PVOID *)&pStringList->pStringList);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pStringList->dwCount = 0;
    pStringList->dwSize = dwInitialCount;

    *ppStringList = pStringList;

cleanup:
    return dwError;

error:
    VmStringListFree(pStringList);
    goto cleanup;
}

DWORD
VmStringListAdd(
    PVM_STRING_LIST pStringList,
    PCSTR pszString
    )
{
    DWORD dwError = 0;

    // keep string list null-terminated
    if (pStringList->dwCount + 1 == pStringList->dwSize)
    {
        DWORD dwOldSize = pStringList->dwSize;
        DWORD dwNewSize = pStringList->dwSize * 2;

        //
        // Check for overflow.
        //
        if (dwNewSize < pStringList->dwSize)
        {
            BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_SIZELIMIT_EXCEEDED);
        }

        dwError = VmReallocateMemoryWithInit(
                    pStringList->pStringList,
                    dwOldSize * sizeof(PSTR),
                    (PVOID*)&pStringList->pStringList,
                    dwNewSize * sizeof(PSTR));
        BAIL_ON_VM_COMMON_ERROR(dwError);

        pStringList->dwSize = dwNewSize;
    }

    pStringList->pStringList[pStringList->dwCount++] = pszString;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmStringListRemove(
    PVM_STRING_LIST pStringList,
    PCSTR pszString
    )
{
    DWORD i = 0;
    DWORD dwError = 0;
    BOOL bFound = FALSE;

    for (i = 0; i < pStringList->dwCount; ++i)
    {
        if (pStringList->pStringList[i] == pszString)
        {
            memmove(&pStringList->pStringList[i],
                    &pStringList->pStringList[i + 1],
                    (pStringList->dwCount - i - 1) * sizeof(PCSTR));
            pStringList->dwCount -= 1;
            bFound = TRUE;
            break;
        }
    }

    if (!bFound)
    {
        //
        // Specified string not found.
        //
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_NOT_FOUND);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

BOOLEAN
VmStringListContains(
    PVM_STRING_LIST pStringList,
    PCSTR pszString
    )
{
    DWORD i = 0;

    for (i = 0; i < pStringList->dwCount; ++i)
    {
        if (pStringList->pStringList[i] == pszString)
        {
            return TRUE;
        }
    }

    return FALSE;
}

DWORD
VmStringListAddStrClone(
    PCSTR               pszStr,
    PVM_STRING_LIST  pStrList
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalStr = NULL;

    if (!pszStr || !pStrList)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmAllocateStringA(pszStr, &pszLocalStr);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmStringListAdd(pStrList, pszLocalStr);
    BAIL_ON_VM_COMMON_ERROR(dwError);
    pszLocalStr = NULL;

cleanup:
    VM_COMMON_SAFE_FREE_MEMORY(pszLocalStr);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmStringListReverse(
    PVM_STRING_LIST pStrList
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    DWORD j = 0;
    PSTR pszTemp = NULL;

    if (!pStrList)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (pStrList->dwCount > 1)
    {
        for (i = 0, j = pStrList->dwCount-1; i < j; ++i, --j)
        {
            pszTemp = (PSTR)pStrList->pStringList[i];
            pStrList->pStringList[i] = (PSTR)pStrList->pStringList[j];
            pStrList->pStringList[j] = pszTemp;
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmStringListRemoveLast(
    PVM_STRING_LIST pStrList
    )
{
    DWORD dwError = 0;

    if (!pStrList || pStrList->dwCount == 0)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    pStrList->dwCount--;

    if (pStrList->pStringList[pStrList->dwCount])
    {
        VmFreeStringA((PSTR)pStrList->pStringList[pStrList->dwCount]);
        pStrList->pStringList[pStrList->dwCount] = NULL;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmStringListFromMultiString(
    PCSTR pszMultiString,
    DWORD dwCountHint, // 0 if caller doesn't know
    PVM_STRING_LIST *ppStrList
    )
{
    PVM_STRING_LIST pStringList = NULL;
    DWORD dwError = 0;

    dwError = VmStringListInitialize(&pStringList, dwCountHint);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    do
    {
        dwError = VmStringListAddStrClone(pszMultiString, pStringList);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        pszMultiString += strlen(pszMultiString) + 1;
    } while (*pszMultiString != '\0');

    *ppStrList = pStringList;

cleanup:
    return dwError;
error:
    VmStringListFree(pStringList);
    goto cleanup;
}

DWORD
VmMultiStringFromStringList(
    PVM_STRING_LIST pStrList,
    PSTR *ppszString,
    PDWORD pdwByteCount // includes all nulls, including final double
    )
{
    PSTR pszMultiString = NULL;
    PSTR pszMultiStringStart = NULL;
    DWORD dwSize = 1; // 1 for terminating null
    DWORD i = 0;
    DWORD dwError = 0;

    for (i = 0; i < pStrList->dwCount; ++i)
    {
        dwSize += strlen(pStrList->pStringList[i]) + 1;
    }

    dwError = VmAllocateMemory(dwSize, (PVOID*)&pszMultiStringStart);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pszMultiString = pszMultiStringStart;
    for (i = 0; i < pStrList->dwCount; ++i)
    {
        strcpy(pszMultiString, pStrList->pStringList[i]);
        pszMultiString += strlen(pStrList->pStringList[i]) + 1;
    }
    *pszMultiString = '\0';

    *ppszString = pszMultiStringStart;
    *pdwByteCount = dwSize;

cleanup:
    return dwError;
error:
    VM_COMMON_SAFE_FREE_MEMORY(pszMultiStringStart);
    goto cleanup;
}

/*
 *  does NOT return empty string token.
 *  say pszStr = "(A;;RP;;;MYSID)" and pszDelimiter = ";"
 *  return pList->pStringList[0] = "(A"
 *         pList->pStringList[1] = "RP"
 *         pList->pStringList[2] = "MYSID)"
 */
DWORD
VmStringToTokenList(
    PCSTR pszStr,
    PCSTR pszDelimiter,
    PVM_STRING_LIST *ppStrList
    )
{
    DWORD       dwError = 0;
    PSTR        pszToken = NULL;
    PSTR        pszLocal = NULL;
    PSTR        pszSavePtr = NULL;
    PVM_STRING_LIST  pList = NULL;

    if ( IsNullOrEmptyString(pszStr) || IsNullOrEmptyString(pszDelimiter) || ppStrList == NULL )
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmStringListInitialize(&pList, 10);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    // make a local copy
    dwError = VmAllocateStringA(
                pszStr,
                &pszLocal);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for ( pszToken = VmStringTokA(pszLocal, pszDelimiter, &pszSavePtr);
          pszToken;
          pszToken=VmStringTokA(NULL, pszDelimiter, &pszSavePtr))
    {
        dwError = VmStringListAddStrClone (pszToken, pList);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    *ppStrList = pList;

cleanup:
    VM_COMMON_SAFE_FREE_MEMORY(pszLocal);

    return dwError;

error:
    VmStringListFree(pList);
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
VmStringToTokenListExt(
    PCSTR pszStr,
    PCSTR pszDelimiter,
    PVM_STRING_LIST *ppStrList
    )
{
    DWORD       dwError = 0;
    PSTR        pszToken = NULL;
    PSTR        pszLocal = NULL;
    PSTR        pszHead = NULL;
    SIZE_T      dwSize = 0;
    PVM_STRING_LIST  pList = NULL;

    if ( IsNullOrEmptyString(pszStr) || IsNullOrEmptyString(pszDelimiter) || ppStrList == NULL )
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwSize = VmStringLenA(pszDelimiter);

    dwError = VmStringListInitialize(&pList, 10);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    // make a local copy
    dwError = VmAllocateStringA(
                pszStr,
                &pszLocal);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pszHead = pszLocal;
    while ((pszToken = strstr(pszHead, pszDelimiter)) != NULL)
    {
        *pszToken = '\0';
        dwError = VmStringListAddStrClone (pszHead, pList);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        pszHead = pszToken + dwSize;
    }

    dwError = VmStringListAddStrClone (pszHead, pList);

    *ppStrList = pList;

cleanup:
    VM_COMMON_SAFE_FREE_MEMORY(pszLocal);

    return dwError;

error:
    VmStringListFree(pList);
    goto cleanup;
}


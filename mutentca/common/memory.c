/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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
LwCAAllocateMemory(
    DWORD dwSize,
    PVOID * ppMemory
    )
{
    DWORD dwError = 0;
    void* pMemory = NULL;

    if (!ppMemory || !dwSize)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pMemory = calloc(1, dwSize);
    if (!pMemory)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

cleanup:
    *ppMemory = pMemory;

    return dwError;

error:
    LwCAFreeMemory(pMemory);
    pMemory = NULL;

    goto cleanup;
}

DWORD
LwCAReallocateMemory(
    PVOID        pMemory,
    PVOID*       ppNewMemory,
    DWORD        dwSize
    )
{
    DWORD       dwError = 0;
    void*       pNewMemory = NULL;

    if (!ppNewMemory)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pMemory)
    {
        pNewMemory = realloc(pMemory, dwSize);
    }
    else
    {
        dwError = LwCAAllocateMemory(dwSize, &pNewMemory);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!pNewMemory)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppNewMemory = pNewMemory;

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
LwCAReallocateMemoryWithInit(
    PVOID         pMemory,
    PVOID*        ppNewMemory,
    size_t        dwNewSize,
    size_t        dwOldSize
    )
{
    DWORD   dwError = 0;
    PVOID   pNewMemory = NULL;

    if (dwNewSize <= dwOldSize || !ppNewMemory)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAReallocateMemory(pMemory, &pNewMemory, dwNewSize);
    BAIL_ON_LWCA_ERROR(dwError);

    memset(((char*)(pNewMemory)) + dwOldSize, 0, dwNewSize - dwOldSize);

    *ppNewMemory = pNewMemory;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_MEMORY(pNewMemory);

    goto cleanup;
}

DWORD
LwCACopyMemory(
    PVOID       pDst,
    size_t      dstSize,
    const void* pSrc,
    size_t      cpySize
    )
{
    DWORD   dwError = 0;

    if (!pDst || !pSrc || cpySize > dstSize)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    memcpy(pDst, pSrc, cpySize);

error:
    return dwError;
}

DWORD
LwCAAllocateAndCopyMemory(
    PVOID   pBlob,
    size_t  iBlobSize,
    PVOID*  ppOutBlob
    )
{
    DWORD   dwError = 0;
    PVOID   pMemory = NULL;

    if (!pBlob || !ppOutBlob || iBlobSize < 1)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(iBlobSize, &pMemory);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACopyMemory(pMemory, iBlobSize, pBlob, iBlobSize);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppOutBlob = pMemory;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_MEMORY(pMemory);

    goto cleanup;
}

VOID
LwCAFreeMemory(
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
LwCAAllocateStringWithLengthA(
    PCSTR pszString,
    DWORD dwSize,
    PSTR * ppszString
    )
{
    DWORD dwError = 0;
    PSTR pszNewString = NULL;

    if (!ppszString || (DWORD)(dwSize + 1) == 0) {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }
    pszNewString = malloc(dwSize + 1);
    if (!pszNewString) {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
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
LwCAAllocateStringA(
    PCSTR pszString,
    PSTR * ppszString
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
        return LWCA_ERROR_BUFFER_OVERFLOW;
    }
    return LwCAAllocateStringWithLengthA(pszString, (DWORD) strlen(pszString), ppszString);
}


DWORD
LwCAAllocateStringPrintfA(
    PSTR* ppszString,
    PCSTR pszFormat,
    ...
    )
{
    ULONG ulError = 0;

    if (!ppszString || !pszFormat)
    {
        ulError = LWCA_ERROR_INVALID_PARAMETER;
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
}

VOID
LwCAFreeStringA(
    PSTR pszString
    )
{
    LwCAFreeMemory(pszString);
}

DWORD
LwCAAllocateStringW(
    PCWSTR pwszSrc,
    PWSTR* ppwszDst
    )
{
    DWORD dwError = 0;

    if (!pwszSrc || !ppwszDst)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
    }
    else
    {
        dwError = LwNtStatusToWin32Error(
                        LwRtlWC16StringDuplicate(ppwszDst, pwszSrc));
        BAIL_ON_LWCA_ERROR(dwError);
    }
error:
    return dwError;
}


VOID
LwCAFreeStringW(
    PWSTR pszString
    )
{
    LwCAFreeMemory(pszString);
}

DWORD
ConvertAnsitoUnicodeString(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    )
{
    DWORD dwError = 0;

    if (!pszSrc || !ppwszDst)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
    }
    else
    {
        dwError = LwNtStatusToWin32Error(
                        LwRtlWC16StringAllocateFromCString(ppwszDst, pszSrc));
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

    if (!pwszSrc || !ppszDst)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
    }
    else
    {
        dwError = LwNtStatusToWin32Error(
                        LwRtlCStringAllocateFromWC16String(ppszDst, pwszSrc));
    }
    return dwError;
}

DWORD
LwCAGetStringLengthW(
    PCWSTR  pwszStr,
    PSIZE_T pLength
    )
{
    DWORD dwError = 0;

    if (!pwszStr || !pLength)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
    }
    else
    {
        *pLength = LwRtlWC16StringNumChars(pwszStr);
    }

    return dwError;
}

DWORD
LwCACopyStringArrayA(
    PSTR            **pppszDst,
    DWORD           dwDstLen,
    PSTR            *ppszSrc,
    DWORD           dwSrcLen
    )
{
    DWORD           dwError = 0;
    DWORD           dwIdx = 0;
    PSTR            *ppszDst = NULL;

    if (!pppszDst ||
        dwDstLen < 1 ||
        !ppszSrc ||
        dwSrcLen < 1 ||
        dwDstLen > dwSrcLen)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(
                    sizeof(PSTR) * dwDstLen,
                    (PVOID *)&ppszDst);
    BAIL_ON_LWCA_ERROR(dwError);

    for (; dwIdx < dwDstLen && dwIdx < dwSrcLen; ++dwIdx)
    {
        dwError = LwCAAllocateStringA(
                            ppszSrc[dwIdx],
                            &ppszDst[dwIdx]);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *pppszDst = ppszDst;


cleanup:

    return dwError;

error:

    LwCAFreeStringArrayA(ppszDst, dwDstLen);
    if (pppszDst)
    {
        *pppszDst = NULL;
    }

    goto cleanup;
}

VOID
LwCAFreeStringArrayA(
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
              LwCAFreeStringA(ppszStrings[dwCnt]);
            }
        }

        LwCAFreeMemory(ppszStrings);
    }
}

DWORD
LwCACreateStringArray(
    PSTR                *ppszSrc,
    DWORD               dwSrcLen,
    PLWCA_STRING_ARRAY* ppStrOutputArray
    )
{
    DWORD dwError = 0;
    DWORD dwIdx = 0;
    PLWCA_STRING_ARRAY pStrTempArray = NULL;

    if (!ppszSrc || dwSrcLen == 0 || !ppStrOutputArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_STRING_ARRAY), (PVOID*)&pStrTempArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(PSTR) * dwSrcLen, (PVOID*)&pStrTempArray->ppData);
    BAIL_ON_LWCA_ERROR(dwError);

    for(; dwIdx < dwSrcLen; ++dwIdx)
    {
        dwError = LwCAAllocateStringA(ppszSrc[dwIdx], &pStrTempArray->ppData[dwIdx]);
        BAIL_ON_LWCA_ERROR(dwError);

        pStrTempArray->dwCount++;
    }

    *ppStrOutputArray = pStrTempArray;

cleanup:
    return dwError;

error:
    LwCAFreeStringArray(pStrTempArray);
    if (ppStrOutputArray)
    {
        *ppStrOutputArray = NULL;
    }

    goto cleanup;
}

DWORD
LwCACopyStringArray(
    PLWCA_STRING_ARRAY  pStrInputArray,
    PLWCA_STRING_ARRAY* ppStrOutputArray
    )
{
    DWORD dwError = 0;
    DWORD dwIdx = 0;
    PLWCA_STRING_ARRAY pStrTempArray = NULL;

    if (!pStrInputArray || !pStrInputArray->ppData || pStrInputArray->dwCount == 0 || !ppStrOutputArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_STRING_ARRAY), (PVOID*)&pStrTempArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(PSTR) * pStrInputArray->dwCount, (PVOID*)&pStrTempArray->ppData);
    BAIL_ON_LWCA_ERROR(dwError);

    for(; dwIdx < pStrInputArray->dwCount; ++dwIdx)
    {
        dwError = LwCAAllocateStringA(pStrInputArray->ppData[dwIdx], &pStrTempArray->ppData[dwIdx]);
        BAIL_ON_LWCA_ERROR(dwError);

        pStrTempArray->dwCount++;
    }

    *ppStrOutputArray = pStrTempArray;

cleanup:
    return dwError;

error:
    LwCAFreeStringArray(pStrTempArray);
    if (ppStrOutputArray)
    {
        *ppStrOutputArray = NULL;
    }

    goto cleanup;
}

VOID
LwCAFreeStringArray(
    PLWCA_STRING_ARRAY pStrArray
    )
{
    DWORD dwIdx = 0;

    if (pStrArray)
    {
        if (pStrArray->ppData)
        {
            for (dwIdx = 0; dwIdx < pStrArray->dwCount; ++dwIdx)
            {
                LWCA_SAFE_FREE_STRINGA(pStrArray->ppData[dwIdx]);
            }
            LWCA_SAFE_FREE_MEMORY(pStrArray->ppData);
        }
        LWCA_SAFE_FREE_MEMORY(pStrArray);
    }
}

DWORD
LwCARequestContextCreate(
    PLWCA_REQ_CONTEXT       *ppReqCtx               // OUT
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;

    if (!ppReqCtx)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_REQ_CONTEXT), (PVOID *)&pReqCtx);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppReqCtx = pReqCtx;


cleanup:

    return dwError;

error:

    LwCARequestContextFree(pReqCtx);
    if (ppReqCtx)
    {
        *ppReqCtx = NULL;
    }

    goto cleanup;
}

VOID
LwCARequestContextFree(
    PLWCA_REQ_CONTEXT       pReqCtx
    )
{
    if (pReqCtx)
    {
        LWCA_SAFE_FREE_STRINGA(pReqCtx->pszBindUPN);
        LWCA_SAFE_FREE_STRINGA(pReqCtx->pszBindUPNDN);
        LWCA_SAFE_FREE_STRINGA(pReqCtx->pszBindUPNTenant);
        LwCAFreeStringArray(pReqCtx->pBindUPNGroups);
        LWCA_SAFE_FREE_STRINGA(pReqCtx->pszCAId);
        LWCA_SAFE_FREE_STRINGA(pReqCtx->pszRequestId);
        LWCA_SAFE_FREE_MEMORY(pReqCtx);
    }
}

void
LwCASetBit(unsigned long *flag, int bit)
{
    *flag |= 1<< bit;
}

int
LwCAisBitSet(unsigned long flag, int bit)
{
    return (flag & ( 1 << bit));
}


void
LwCAClearBit(unsigned long flag, int bit)
{
     flag &= ~(1 << bit);
}

void
LwCAToggleBit(unsigned long flag, int bit)
{
    flag ^= (1 << bit);
}

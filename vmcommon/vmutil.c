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

/*
 * when hash map does not own key and value pair.
 */
VOID
VmNoopHashMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    LW_PVOID            pUnused
    )
{
    return;
}

/*
 * when hash map can use simple free function for both key and value.
 */
VOID
VmSimpleHashMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VM_COMMON_SAFE_FREE_MEMORY(pPair->pKey);
    VM_COMMON_SAFE_FREE_MEMORY(pPair->pValue);
}

/*
 * when hash map can use simple free function for key only.
 */
VOID
VmSimpleHashMapPairFreeKeyOnly(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VM_COMMON_SAFE_FREE_MEMORY(pPair->pKey);
}

/*
 * when hash map can use simple free function for value only.
 */
VOID
VmSimpleHashMapPairFreeValOnly(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VM_COMMON_SAFE_FREE_MEMORY(pPair->pValue);
}

DWORD
VmSignatureEncodeHex(
    const unsigned char     data[],
    const size_t            length,
    PSTR                    *ppHex
    )
{
    DWORD   dwError = 0;
    PSTR    pHex = NULL;
    int     i = 0;

    dwError = VmAllocateMemory(length * 2 + 1, (void**) &pHex);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (i = 0; i < length; ++i)
    {
        sprintf(&pHex[i * 2], "%02x", data[i]);
    }

    *ppHex = pHex;

cleanup:
    return dwError;

error:
    VM_COMMON_SAFE_FREE_STRINGA(pHex);
    if (ppHex)
    {
        *ppHex = NULL;
    }

    goto cleanup;
}

DWORD
VmSignatureDecodeHex(
    PCSTR               pcszHexStr,
    unsigned char       **ppData,
    size_t              *pLength
    )
{
    DWORD               dwError = 0;
    DWORD               dwIdx = 0;
    size_t              hexlen = 0;
    size_t              datalen = 0;
    unsigned char       *pData = NULL;
    char                twoHexChars[3] = {'\0'};

    BAIL_ON_VM_COMMON_INVALID_STR_PARAMETER(pcszHexStr, dwError);
    BAIL_ON_VM_COMMON_INVALID_PARAMETER(ppData, dwError)
    BAIL_ON_VM_COMMON_INVALID_PARAMETER(pLength, dwError)

    hexlen = VmStringLenA(pcszHexStr);
    if (hexlen % 2)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    datalen = hexlen / 2;
    dwError = VmAllocateMemory(datalen, (PVOID*)&pData);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (dwIdx = 0; dwIdx < datalen; ++dwIdx)
    {
        strncpy(twoHexChars, &pcszHexStr[dwIdx * 2], 2);
        twoHexChars[2] = '\0';
        pData[dwIdx] = (unsigned char)strtoul(twoHexChars, NULL, 16);
    }

    *ppData = pData;
    *pLength = datalen;


cleanup:
    return dwError;

error:
    VM_COMMON_SAFE_FREE_MEMORY(pData);
    if (ppData)
    {
        *ppData = NULL;
    }
    if (pLength)
    {
        *pLength = 0;
    }

    goto cleanup;
}

/*
 * encode a string into Base 64
 */
DWORD
VmEncodeToBase64(
    PBYTE       pInput,
    DWORD       dwInputLen,
    PBYTE       *ppBase64Encoded,
    DWORD       *pDwEncodedLen
    )
{
    DWORD   dwError = 0;
    DWORD   dwBase64EncodedBufferLen = 0;
    DWORD   dwBase64EncodedByteLen = 0;
    PBYTE   pBase64Encoded = NULL;

    if (!pInput || !ppBase64Encoded)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwBase64EncodedBufferLen = VM_COMMON_GET_BASE64_ENCODE_LEN(dwInputLen);

    dwError = VmAllocateMemory(dwBase64EncodedBufferLen,
                               (PVOID *)&pBase64Encoded
                               );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = sasl_encode64((PCSTR)pInput,
                            dwInputLen,
                            (PSTR)pBase64Encoded,
                            dwBase64EncodedBufferLen,
                            &dwBase64EncodedByteLen
                            );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    *ppBase64Encoded = pBase64Encoded;
    *pDwEncodedLen = dwBase64EncodedByteLen;

cleanup:
    return dwError;

error:
    VM_COMMON_SAFE_FREE_MEMORY(pBase64Encoded);
    if (ppBase64Encoded)
    {
        *ppBase64Encoded = NULL;
    }
    if (pDwEncodedLen)
    {
        *pDwEncodedLen = 0;
    }
    goto cleanup;
}

/*
 * decode a Base 64 to string
 */
DWORD
VmDecodeToBase64(
    PBYTE       pEncodedInput,
    DWORD       dwEncodedLen,
    PBYTE       *ppBase64Decoded,
    DWORD       *pDecodedLen
    )
{
    DWORD       dwError = 0;
    DWORD       dwBase64DecodedByteLen = 0;
    PBYTE       pBase64Decoded = NULL;

    if (!pEncodedInput || !ppBase64Decoded)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmAllocateMemory(dwEncodedLen, (PVOID *)&pBase64Decoded);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = sasl_decode64((PCSTR)pEncodedInput,
                            dwEncodedLen,
                            (PSTR)pBase64Decoded,
                            dwEncodedLen,
                            &dwBase64DecodedByteLen
                            );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    *ppBase64Decoded = pBase64Decoded;
    *pDecodedLen = dwBase64DecodedByteLen;

cleanup:
    return dwError;

error:
    VM_COMMON_SAFE_FREE_MEMORY(pBase64Decoded);
    if (ppBase64Decoded)
    {
        *ppBase64Decoded = NULL;
    }
    if (pDecodedLen)
    {
        *pDecodedLen = 0;
    }

    goto cleanup;
}

uint64_t
VmGetTimeInMilliSec(
    VOID
    )
{
    uint64_t            iTimeInMSec = 0;

#if !defined(__APPLE__)

    struct timespec     timeValue = {0};

    if (clock_gettime(CLOCK_REALTIME, &timeValue) == 0)
    {
        iTimeInMSec = timeValue.tv_sec * MSECS_PER_SEC + timeValue.tv_nsec / NSECS_PER_MSEC;
    }

#endif

    return  iTimeInMSec;
}

VOID
VmSleep(
    DWORD      dwMilliseconds
    )
{
    struct timespec req={0};
    DWORD   dwSec = dwMilliseconds/1000;
    DWORD   dwMS  = dwMilliseconds%1000;

    req.tv_sec  = dwSec;
    req.tv_nsec = dwMS*1000000;

    nanosleep( &req, NULL ); // ignore error
}


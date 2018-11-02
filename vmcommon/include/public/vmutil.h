/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#define VM_BYTE_REVERSE_BITS(in, out) \
    (out) = (in);                                       \
    (out) = ((out) & 0xF0) >> 4 | ((out) & 0x0F) << 4;  \
    (out) = ((out) & 0xCC) >> 2 | ((out) & 0x33) << 2;  \
    (out) = ((out) & 0xAA) >> 1 | ((out) & 0x55) << 1;

/*
 * when hash map does not own key and value pair.
 */
VOID
VmNoopHashMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    LW_PVOID            pUnused
    );

/*
 * when hash map can use simple free function for both key and value.
 */
VOID
VmSimpleHashMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    );

/*
 * when hash map can use simple free function for key only.
 */
VOID
VmSimpleHashMapPairFreeKeyOnly(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    );

/*
 * when hash map can use simple free function for value only.
 */
VOID
VmSimpleHashMapPairFreeValOnly(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    );

DWORD
VmSignatureEncodeHex(
    const unsigned char     data[],
    const size_t            length,
    PSTR                    *ppHex
    );

DWORD
VmSignatureDecodeHex(
    PCSTR               pcszHexStr,
    unsigned char       **ppData,
    size_t              *pLength
    );

/*
 * encode a string into Base 64
 */
DWORD
VmEncodeToBase64(
    PBYTE       pInput,
    DWORD       inputLen,
    PBYTE       *ppBase64Encoded,
    DWORD       *pEncodedLen
    );

/*
 * decode a Base 64 to string
 */
DWORD
VmDecodeToBase64(
    PBYTE       pEncodedInput,
    DWORD       encodedLen,
    PBYTE       *ppBase64Decoded,
    DWORD       *pDecodedLen
    );
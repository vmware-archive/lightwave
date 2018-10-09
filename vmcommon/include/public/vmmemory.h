/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

#ifndef _VM_COMMON_MEMORY_H__
#define _VM_COMMON_MEMORY_H__

DWORD
VmAllocateMemory(
    size_t  dwSize,
    PVOID*  ppMemory
    );

DWORD
VmReallocateMemory(
    PVOID   ppMemory,
    PVOID*  ppNewMemory,
    size_t  dwSize
    );

DWORD
VmReallocateMemoryWithInit(
    PVOID        pMemory,
    size_t       dwCurrentSize,
    PVOID*       ppNewMemory,
    size_t       dwNewSize
    );

DWORD
VmReallocateString(
    PSTR    pString,
    size_t  dwCurrentSize,
    PSTR*   ppszNewString,
    size_t  dwNewSize
    );

DWORD
VmCopyMemory(
    PVOID   pDestination,
    size_t  destinationSize,
    const void* pSource,
    size_t  maxCount
    );

DWORD
VmAllocateStringOfLenA(
    PCSTR   pszSource,
    size_t  sLength,
    PSTR*   ppszDestination
    );

DWORD
VmAllocateStringA(
    PCSTR   pszString,
    PSTR*   ppszString
    );

VOID
VmFreeMemory(
    PVOID   pMemory
    );

#endif /* __VM_COMMON_MEMORY_H__ */

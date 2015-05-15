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



#ifndef _VMDEPLOY_COMMON_H__
#define _VMDEPLOY_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

// memory.c

DWORD
VmwDeployAllocateMemory(
    size_t size,
    PVOID* ppMemory
    );

VOID
VmwDeployFreeMemory(
    PVOID pMemory
    );

//strings.c
DWORD
VmwDeployStringCatA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
);

DWORD
VmwDeployAllocateStringA(
    PCSTR   pszString,
    PSTR*   ppszString
    );

ULONG
VmwDeployAllocateStringPrintf(
    PSTR* ppszString,
    PCSTR pszFormat,
    ...
    );

DWORD
VmwDeployAllocateStringPrintfV(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    );

DWORD
VmwDeployAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    );

DWORD
VmwDeployAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    );

DWORD
VmDeployAllocateStringPrintf(
    PWSTR* ppszString,
    PCWSTR pszFormat,
    ...
    );

DWORD
VmDeployAllocateStringPrintfV(
    PWSTR*   ppszStr,
    PCWSTR   pszFormat,
    va_list argList
    );

#ifdef __cplusplus
}
#endif

#endif /* _VMDEPLOY_COMMON_H__ */

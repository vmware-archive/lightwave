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

/*
 * Module Name:
 *
 *        idmcommon.h
 *
 * Abstract:
 *
 *        Identity Manager - Common utilities
 *
 *        Public header for common utilities
 *
 * Authors: Sriram Nambakam (snambakam@vmware.com)
 *
 */

DWORD
IDMAllocateMemory(
    SIZE_T size,
    PVOID* ppMemory
    );

VOID
IDMFreeMemory(
    PVOID pMemory
    );

DWORD
IDMAllocateString(
    PWSTR  pszString,
    PWSTR* ppszString
    );

DWORD
IDMAllocateStringA(
    PSTR  pszString,
    PSTR* ppszString
    );

VOID
IDMFreeString(
    PWSTR pszString
    );

DWORD
IDMCloneSid(
    PSID pSid,
    PSID *ppNewSid
    );

VOID
IDMFreeSid(
    PSID pSid
    );

#ifdef _WIN32

DWORD
IDMAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    );

DWORD
IDMAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    );

#else

DWORD
IDMAllocateStringPrintfV(
    PSTR* ppszStr,
    PCSTR pszFormat,
    va_list argList
    );

DWORD
IDMAllocateStringPrintf(
    PSTR* ppszString,
    PCSTR pszFormat,
    ...
    );

#endif /* _WIN32 */


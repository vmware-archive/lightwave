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



#ifndef _VMAUTHSVC_COMMON_H__
#define _VMAUTHSVC_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <dce/uuid.h>
#include <dce/dcethread.h>
#if defined(_WIN32)
typedef unsigned char uuid_t[16];  // typedef dce_uuid_t uuid_t;
#endif

// Logging
extern int  vmauthsvc_syslog;
extern int  vmauthsvc_debug;

DWORD
VmAuthsvcAllocateMemory(
    size_t  dwSize,
    PVOID*  ppMemory
    );

DWORD
VmAuthsvcReallocateMemory(
    PVOID   pMemory,
    PVOID*  ppNewMemory,
    size_t  dwSize
    );

DWORD
VmAuthsvcCopyMemory(
    PVOID   pDestination,
    size_t  destinationSize,
    PCVOID  pSource,
    size_t  maxCount
    );

DWORD
VmAuthsvcReallocateMemoryWithInit(
    PVOID  pMemory,
    PVOID* ppNewMemory,
    size_t dwNewSize,
    size_t dwOldSize
    );

VOID
VmAuthsvcFreeMemory(
    PVOID   pMemory
    );

VOID
VmAuthsvcFreeStringA(
    PSTR    pszString
    );

VOID
VmAuthsvcFreeStringArrayA(
    PSTR*   ppszString
    );

DWORD
VmAuthsvcAllocateStringAVsnprintf(
    PSTR*    ppszOut,
    PCSTR    pszFormat,
    ...
    );

ULONG
VmAuthsvcAllocateStringW(
    PCWSTR pwszSrc,
    PWSTR* ppwszDst
    );

ULONG
VmAuthsvcAllocateStringA(
    PCSTR pszSrc,
    PSTR* ppszDst
    );

ULONG
VmAuthsvcAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    );

ULONG
VmAuthsvcAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    );

ULONG
VmAuthsvcAllocateStringPrintfV(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    );

ULONG
VmAuthsvcAllocateStringPrintf(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    );

ULONG
VmAuthsvcGetStringLengthW(
    PCWSTR  pwszStr,
    PSIZE_T pLength
    );

ULONG
VmAuthsvcStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    );

ULONG
VmAuthsvcStringNCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    size_t n,
    BOOLEAN bIsCaseSensitive
    );

SIZE_T
VmAuthsvcStringLenA(
    PCSTR pszStr
);

PSTR
VmAuthsvcStringChrA(
   PCSTR str,
   int c
);

PSTR
VmAuthsvcStringRChrA(
   PCSTR str,
   int c
);

PSTR
VmAuthsvcStringTokA(
   PSTR strToken,
   PCSTR strDelimit,
   PSTR* context
);

PSTR
VmAuthsvcStringStrA(
   PCSTR str,
   PCSTR strSearch
);

DWORD
VmAuthsvcStringCpyA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
);

DWORD
VmAuthsvcStringNCpyA(
   PSTR strDest,
   size_t numberOfElements,
   PCSTR strSource,
   size_t count
);

DWORD
VmAuthsvcStringCatA(
   PSTR strDestination,
   size_t numberOfElements,
   PCSTR strSource
);

int64_t
VmAuthsvcStringToLA(
   PCSTR nptr,
   PSTR* endptr,
   int base
);

int VmAuthsvcStringToIA(
   PCSTR pStr
);

DWORD
VmAuthsvcStringErrorA(
   PSTR buffer,
   size_t numberOfElements,
   int errnum
);

PSTR
VmAuthsvcCaselessStrStrA(
    PCSTR pszStr1,
    PCSTR pszStr2
    );

DWORD
VmAuthsvcStringPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    PCSTR pszFormat,
    ...
);

DWORD
VmAuthsvcStringNPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    size_t maxSize,
    PCSTR pszFormat,
    ...
);

VOID
VmAuthsvcLog(
   int level,
   const char*      fmt,
   ...);

DWORD
VmAuthsvcLogInitialize(
   const char * logFileName);

VOID
VmAuthsvcLogTerminate();

DWORD
VmAuthsvcFileExists(
    PCSTR pszFileName,
    PBOOLEAN pbFound
    );

DWORD
VmAuthsvcGetCanonicalHostName(
    PCSTR pszHostname,
    PSTR* ppszCanonicalHostName
    );

DWORD
VmAuthsvcGetHostName(
    PSTR* ppszHostName
    );

VOID
VmAuthsvcSleep(
    DWORD dwMilliseconds
    );

#ifdef _WIN32

//cmd line args parsing helpers
BOOLEAN
VmAuthsvcIsCmdLineOption(
    PSTR pArg
);

VOID
VmAuthsvcGetCmdLineOption(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    PCSTR* ppszOptionValue
);

DWORD
VmAuthsvcGetCmdLineIntOption(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    int* pValue
);

DWORD
VmAuthsvcAllocateArgsAFromArgsW(
    int argc,
    WCHAR* argv[],
    PSTR** argvA
);

VOID
VmAuthsvcDeallocateArgsA(
    int argc,
    PSTR argv[]
);

#endif

#ifdef __cplusplus
}
#endif

#endif /* _VMAUTHSVC_COMMON_H__ */

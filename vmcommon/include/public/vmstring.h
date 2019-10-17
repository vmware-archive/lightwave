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

#ifndef _VM_COMMON_STRING_H__
#define _VM_COMMON_STRING_H__

DWORD
VmAllocateStringPrintf(
    PSTR* ppszString,
    PCSTR pszFormat,
    ...
    );

DWORD
VmStringPrintFA(
    PSTR pDestination,
    size_t destinationSize,
    PCSTR pszFormat,
    ...
);

DWORD
VmStringNCatA(
   PSTR     strDestination,
   size_t   numberOfElements,
   PCSTR    strSource,
   size_t   number
   );

int
VmStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    );

VOID
VmStringTrimSpace(
    PSTR pszStr
    );

PSTR
VmStringTokA(
    PSTR    strToken,
    PCSTR   strDelimit,
    PSTR*   context
    );

PSTR
VmStringStrA(
    PCSTR   str,
    PCSTR   strSearch
    );

PSTR
VmStringCaseStrA(
    PCSTR    pszSource,
    PCSTR    pszPattern
    );

size_t
VmStringLenA(
    PCSTR   pszStr
    );

PSTR
VmStringChrA(
    PCSTR   str,
    int     c
    );

PSTR
VmStringRChrA(
    PCSTR   str,
    int     c
    );

int
VmStringNCompareA(
    PCSTR       pszStr1,
    PCSTR       pszStr2,
    size_t      n,
    BOOLEAN     bIsCaseSensitive
    );

BOOLEAN
VmStringStartsWithA(
    PCSTR       pszStr,
    PCSTR       pszPrefix,
    BOOLEAN     bIsCaseSensitive
    );

#endif /* __VM_COMMON_STRING_H__ */

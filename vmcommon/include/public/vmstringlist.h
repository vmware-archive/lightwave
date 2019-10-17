/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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

#ifndef _VM_COMMON_STRING_LIST_H__
#define _VM_COMMON_STRING_LIST_H__

typedef struct _VM_STRING_LIST
{
    PCSTR *pStringList;
    DWORD dwCount; // Current count.
    DWORD dwSize; // Max number of strings we can store currently.
} VM_STRING_LIST, *PVM_STRING_LIST;

/* vmstringlist.c */
VOID
VmStringListFreeContent(
    PVM_STRING_LIST pStringList
    );

VOID
VmStringListFree(
    PVM_STRING_LIST pStringList
    );

DWORD
VmStringListInitialize(
    PVM_STRING_LIST *ppStringList,
    DWORD dwInitialCount
    );

DWORD
VmStringListAdd(
    PVM_STRING_LIST pStringList,
    PCSTR pszString
    );

DWORD
VmStringListRemove(
    PVM_STRING_LIST pStringList,
    PCSTR pszString
    );

BOOLEAN
VmStringListContains(
    PVM_STRING_LIST pStringList,
    PCSTR pszString
    );

DWORD
VmStringListAddStrClone(
    PCSTR               pszStr,
    PVM_STRING_LIST  pStrList
    );

DWORD
VmStringListReverse(
    PVM_STRING_LIST pStrList
    );

DWORD
VmStringListRemoveLast(
    PVM_STRING_LIST pStrList
    );

DWORD
VmStringListFromMultiString(
    PCSTR pszMultiString,
    DWORD dwCountHint, // 0 if caller doesn't know
    PVM_STRING_LIST *ppStrList
    );

DWORD
VmMultiStringFromStringList(
    PVM_STRING_LIST pStrList,
    PSTR *ppszString,
    PDWORD pdwByteCount // includes all nulls, including final double
    );

DWORD
VmStringToTokenList(
    PCSTR pszStr,
    PCSTR pszDelimiter,
    PVM_STRING_LIST *ppStrList
    );

DWORD
VmStringToTokenListExt(
    PCSTR pszStr,
    PCSTR pszDelimiter,
    PVM_STRING_LIST *ppStrList
    );

#endif /* __VM_COMMON_STRING_LIST_H__ */

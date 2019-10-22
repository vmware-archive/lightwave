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



/* internal.c */
DWORD
VmRegConfigGetKeyInternal(
    PVM_REGCONFIG_LIST_ENTRY    pEntry,
    PCSTR                       pszKeyName,
    PSTR                        pszValue,
    size_t*                     piValueSize
    );

DWORD
VmRegConfigReadFileInternal(
    PVM_REGCONFIG_LIST_ENTRY    pEntry
    );

DWORD
VmRegConfigForceReadInternal(
    PVM_REGCONFIG_LIST_ENTRY    pEntry
    );

DWORD
VmRegConfigSetKeyInternal(
    PVM_REGCONFIG_LIST_ENTRY    pEntry,
    PCSTR                       pszKeyName,
    PCSTR                       pszValue,
    size_t                      iValueSize
    );

DWORD
VmRegConfigDeleteKeyInternal(
    PVM_REGCONFIG_LIST_ENTRY    pEntry,
    PCSTR                       pszKeyName
    );

DWORD
VmRegConfigGetLWUserGroupId(
    VOID
    );

DWORD
VmRegConfigWriteFileInternal(
    PVM_REGCONFIG_LIST_ENTRY   pEntry
    );

DWORD
VmRegConfigLockFile(
    PCSTR   pszLockFileName,
    int*    pfd
    );

/* util.c */
VOID
VmRegConfigKVFree(
    PVM_REGCONFIG_LIST_KV   pKV
    );

VOID
VmRegConfigListKVFree(
    PVM_REGCONFIG_LIST_KV   pListKV
    );

VOID
VmRegConfigListEntryFree(
    PVM_REGCONFIG_LIST_ENTRY    pListEntry
    );

VOID
VmRegConfigConstructorKVFree(
    PVM_CONSTRUCT_KV    pConstructor
    );

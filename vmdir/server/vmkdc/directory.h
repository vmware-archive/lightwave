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



#ifndef _VMKDC_DIRECTORY_H
#define _VMKDC_DIRECTORY_H

typedef struct _VMKDC_DIRECTORY {
    PSTR fileName;
} VMKDC_DIRECTORY, *PVMKDC_DIRECTORY;

typedef struct _VMKDC_DIRECTORY_ENTRY {
    char *princName;
    PVMKDC_KEYSET keyset;
} VMKDC_DIRECTORY_ENTRY, *PVMKDC_DIRECTORY_ENTRY;

VOID
VmKdcFreeDirectoryEntry(
    PVMKDC_DIRECTORY_ENTRY pDirectoryEntry);

DWORD
VmKdcInitializeDirectory(
    PVMKDC_GLOBALS pGlobals);

VOID
VmKdcTerminateDirectory(
    PVMKDC_GLOBALS pGlobals);

DWORD
VmKdcOpenDirectory(
    PSTR pszServerHost,
    int pszServerPort,
    PSTR pszUserDn,
    PSTR pszPassword,
    PVMKDC_DIRECTORY *ppRetDirectory);

VOID
VmKdcCloseDirectory(
    PVMKDC_DIRECTORY pDirectory);

DWORD
VmKdcSearchDirectory(
    PVMKDC_CONTEXT pContext,
    PVMKDC_PRINCIPAL pPrincipal,
    PVMKDC_DIRECTORY_ENTRY *pRetDirectoryEntry);

DWORD
VmKdcFindKeyByEType(
    PVMKDC_DIRECTORY_ENTRY pDirectoryEntry,
    VMKDC_ENCTYPE etype,
    PVMKDC_KEY *ppRetKey);

#define VMKDC_SAFE_FREE_DIRECTORY_ENTRY(x) \
do { \
    if (x) \
    { \
        VmKdcFreeDirectoryEntry(x); \
        x = NULL; \
    } \
} while (0)

#endif /* _VMKDC_DIRECTORY_H */

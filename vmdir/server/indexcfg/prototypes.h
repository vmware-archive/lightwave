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
 * Module Name: Directory indexer
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Function prototypes
 *
 */

#ifndef _CFG_PROTOTYPES_H_
#define _CFG_PROTOTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

// init.c
int
VdirAttrIndexNameCmp(
    const void *p1,
    const void *p2
    );

DWORD
VdirAttrIndexInitViaEntry(
    PVDIR_ENTRY  pEntry);

DWORD
VdirAttrIndexInitViaCacheAndDescs(
    PVDIR_ATTR_INDEX_INSTANCE   pFromCache,
    PVDIR_CFG_ATTR_INDEX_DESC   pIndexDesc,
    USHORT                      dwDescSize);

DWORD
VdirAttrIndexCacheAllocate(
    PVDIR_ATTR_INDEX_INSTANCE* ppAttrIdxCache,
    USHORT  usIdxSize
    );

VOID
VdirAttrIdxCacheFree(
    PVDIR_ATTR_INDEX_INSTANCE   pAttrIdxCache
    );

DWORD
VdirstrToAttrIndexDesc(
    PSTR    pszStr,
    PVDIR_CFG_ATTR_INDEX_DESC   pDesc
    );

// indexingthr.c

DWORD
VdirIndexingPreCheck(
    PVDIR_ATTR_INDEX_INSTANCE* ppCache
    );

DWORD
VdirIndexingEntryAppendFlag(
    VDIR_MODIFICATION*   pMods,
    PVDIR_ENTRY          pEntry);

DWORD
InitializeIndexingThread(
    void);

#ifdef __cplusplus
}
#endif

#endif // _CFG_PROTOTYPES_H_


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
 * Filename: interface.h
 *
 * Abstract:
 *
 * indexer api
 *
 */

#ifndef __VIDRINDEXER_H__
#define __VIDRINDEXER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    VDIR_INDEXING_SCHEDULED,
    VDIR_INDEXING_IN_PROGRESS,
    VDIR_INDEXING_VALIDATING_SCOPES,
    VDIR_INDEXING_COMPLETE,
    VDIR_INDEXING_DISABLED,
    VDIR_INDEXING_DELETED

} VDIR_INDEXING_STATUS;

typedef enum
{
    VDIR_INDEX_READ,
    VDIR_INDEX_WRITE

} VDIR_INDEX_USAGE;

typedef struct _VDIR_INDEX_CFG
{
    PSTR                    pszAttrName;
    BOOLEAN                 bDefaultIndex;
    BOOLEAN                 bScopeEditable;
    BOOLEAN                 bGlobalUniq; // Note: this is default index only property
    BOOLEAN                 bIsNumeric;
    int                     iTypes;
    PLW_HASHMAP             pUniqScopes;

    // fields for indexing progress tracking
    PVDIR_LINKED_LIST       pNewUniqScopes;
    PVDIR_LINKED_LIST       pDelUniqScopes;
    PVDIR_LINKED_LIST       pBadUniqScopes;
    VDIR_INDEXING_STATUS    status;
    ENTRYID                 initOffset;

    PVMDIR_MUTEX            mutex;
    USHORT                  usRefCnt;

} VDIR_INDEX_CFG;

typedef struct _VDIR_INDEX_UPD
{
    PLW_HASHMAP         pUpdIndexCfgMap;
    PVDIR_BACKEND_CTX   pBECtx;

    BOOLEAN             bOwnBECtx;
    BOOLEAN             bHasBETxn;

} VDIR_INDEX_UPD;

///////////////////////////////////////////////////////////////////////////////
// indexer library initialize / shutdown
// indexer cache instantiation
///////////////////////////////////////////////////////////////////////////////

/*
 * Initialize indexer library
 */
DWORD
VmDirIndexLibInit(
    PVMDIR_MUTEX    pModMutex
    );

/*
 * Shutdown indexer library
 */
VOID
VmDirIndexLibShutdown(
    VOID
    );

///////////////////////////////////////////////////////////////////////////////
// Set of functions used to open custom indices during bootstrap
///////////////////////////////////////////////////////////////////////////////

BOOLEAN
VmDirIndexIsDefault(
    PCSTR   pszAttrName
    );

DWORD
VmDirCustomIndexCfgInit(
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    PVDIR_INDEX_CFG*        ppIndexCfg
    );

DWORD
VmDirIndexOpen(
    PVDIR_INDEX_CFG pIndexCfg
    );

VOID
VmDirFreeIndexCfg(
    PVDIR_INDEX_CFG pIndexCfg
    );

///////////////////////////////////////////////////////////////////////////////
// Attribute Index lookup
///////////////////////////////////////////////////////////////////////////////

DWORD
VmDirIndexCfgAcquire(
    PCSTR               pszAttrName,
    VDIR_INDEX_USAGE    usage,
    PVDIR_INDEX_CFG*    ppIndexCfg
    );

VOID
VmDirIndexCfgRelease(
    PVDIR_INDEX_CFG pIndexCfg
    );

BOOLEAN
VmDirIndexExist(
    PCSTR   pszAttrName
    );

DWORD
VmDirIndexCfgMap(
    PLW_HASHMAP*    ppIndexCfgMap
    );

DWORD
VmDirIndexCfgGetAllScopesInStrArray(
    PVDIR_INDEX_CFG pIndexCfg,
    PSTR**          pppszScopes
    );

///////////////////////////////////////////////////////////////////////////////
// Attribute Index update functions
///////////////////////////////////////////////////////////////////////////////
/*
 * In order to use these APIs, you must call them in the
 * right sequence:
 *
 * 1. VmDirIndexUpdateBegin
 * 2. Any combination of:
 *    - VmDirIndexSchedule
 *    - VmDirIndexDelete
 *    - VmDirIndexAddUniquenessScope
 *    - VmDirIndexDeleteUniquenessScope
 * 3. VmDirIndexUpdateCommit (if step 2 succeeded)
 * 4. VmDirIndexUpdateAbort (if step 2 or 3 failed)
 */
DWORD
VmDirIndexUpdateBegin(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_INDEX_UPD*    ppIndexUpd
    );

DWORD
VmDirIndexUpdateCommit(
    PVDIR_INDEX_UPD     pIndexUpd
    );

DWORD
VmDirIndexUpdateAbort(
    PVDIR_INDEX_UPD     pIndexUpd
    );

DWORD
VmDirIndexSchedule(
    PVDIR_INDEX_UPD     pIndexUpd,
    PCSTR               pszAttrName,
    PCSTR               pszAttrSyntaxOid
    );

DWORD
VmDirIndexDelete(
    PVDIR_INDEX_UPD     pIndexUpd,
    PCSTR               pszAttrName
    );

DWORD
VmDirIndexAddUniquenessScope(
    PVDIR_INDEX_UPD     pIndexUpd,
    PCSTR               pszAttrName,
    PCSTR*              ppszUniqScopes
    );

DWORD
VmDirIndexDeleteUniquenessScope(
    PVDIR_INDEX_UPD     pIndexUpd,
    PCSTR               pszAttrName,
    PCSTR*              ppszUniqScopes
    );

#ifdef __cplusplus
}
#endif

#endif /* __VIDRINDEXER_H__ */



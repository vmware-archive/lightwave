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

extern PVDIR_CFG_ATTR_INDEX_DESC    pBootStrapIdxAttrDesc;

typedef enum
{
    VDIR_CFG_ATTR_INDEX_ENABLED = 0,
    VDIR_CFG_ATTR_INDEX_DISABLED,
    VDIR_CFG_ATTR_INDEX_BUILDING,
    VDIR_CFG_ATTR_INDEX_ABORTED

} VDIR_CFG_ATTR_INDEX_STATUS;

typedef enum
{
    VDIR_CFG_ATTR_INDEX_READ = 0,  // Index ready for read/search ops.
    VDIR_CFG_ATTR_INDEX_WRITE,     // Index require update/create for write ops.
    VDIR_CFG_ATTR_INDEX_ALL

} VDIR_CFG_ATTR_INDEX_USAGE_CTX;

typedef struct _VDIR_CFG_ATTR_INDEX_DESC
{
    VDIR_CFG_ATTR_INDEX_STATUS  status;
    PSTR        pszAttrName;
    int         iTypes;
    BOOLEAN     bIsUnique;
    BOOLEAN     bIsNumeric;

    int         iId; // index into array of PVDIR_BDB_INDEX_DATABASE

} VDIR_CFG_ATTR_INDEX_DESC, *PVDIR_CFG_ATTR_DESC;

///////////////////////////////////////////////////////////////////////////////
// indexer library initialize / shutdown
// indexer cache instantiation
///////////////////////////////////////////////////////////////////////////////
/*
 * Initialize indexer library
 */
DWORD
VmDirAttrIndexLibInit(
    VOID
    );

/*
 * Shutdown indexer library
 */
void
VmDirAttrIndexLibShutdown(
    void
    );

/*
 * Bootstrap Attribute Index cache to startup BDB with minimum set of db files.
 */
DWORD
VmDirAttrIndexBootStrap(
    VOID
    );

///////////////////////////////////////////////////////////////////////////////
// Attribute index cache modification
//
// NOTE, cache modify check and commit calls are serialized at the entry modify level.
///////////////////////////////////////////////////////////////////////////////
/*
 * 1. Verify modify contents
 * 2. Create new cache version including new attribute with building flag
 *    (but not yet make this version of cache live)
 */
DWORD
VmDirCFGAttrIndexModifyPrepare(
    VDIR_MODIFICATION*   pMods,
    PVDIR_ENTRY          pEntry
    );
/*
 * Commit attribute index modification into cache. (i.e. make it live version)
 */
VOID
VmDirCFGAttrIndexModifyCommit(
    VOID
    );

///////////////////////////////////////////////////////////////////////////////
// Attribute Index lookup
///////////////////////////////////////////////////////////////////////////////

PVDIR_CFG_ATTR_INDEX_DESC
VmDirAttrNameToReadIndexDesc(
    PCSTR       pszName,        // name of the attribute
    USHORT      usVersion,      // version of index cache to use
    USHORT*     pusVersion      // version of index cache used in his call
    );

PVDIR_CFG_ATTR_INDEX_DESC
VmDirAttrNameToWriteIndexDesc(
    PCSTR       pszName,        // name of the attribute
    USHORT      usVersion,      // version of index cache to use
    USHORT*     pusVersion      // version of index cache used in his call
    );

/*
 * Should only call this during server startup/shutdown to help open/close db files.
 */
DWORD
VmDirAttrIndexDescList(
    USHORT*     pusSize,       // size of pAttrIdxDesc
    PVDIR_CFG_ATTR_INDEX_DESC* ppAttrIdxDesc
    );


#ifdef __cplusplus
}
#endif

#endif /* __VIDRINDEXER_H__ */



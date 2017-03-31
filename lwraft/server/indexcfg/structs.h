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
 * Subset of VDIR_INDEX_CFG to define default indices in defines.h
 */
typedef struct _VDIR_DEFAULT_INDEX_CFG
{
    PSTR                    pszAttrName;
    int                     iTypes;
    BOOLEAN                 bScopeEditable;
    BOOLEAN                 bGlobalUniq;
    BOOLEAN                 bIsNumeric;

} VDIR_DEFAULT_INDEX_CFG, *PVDIR_DEFAULT_INDEX_CFG;

typedef struct _VDIR_INDEXING_TASK
{
    PVDIR_LINKED_LIST   pIndicesToPopulate;
    PVDIR_LINKED_LIST   pIndicesToValidate;
    PVDIR_LINKED_LIST   pIndicesToDelete;
    PVDIR_LINKED_LIST   pIndicesCompleted;

} VDIR_INDEXING_TASK, *PVDIR_INDEXING_TASK;

typedef struct _VDIR_INDEX_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PVMDIR_MUTEX        mutex;
    PVMDIR_COND         cond;
    PLW_HASHMAP         pIndexCfgMap;
    PVDIR_INDEX_UPD     pIndexUpd;

    // fields used to determine index status during bootstrap
    BOOLEAN             bFirstboot;
    BOOLEAN             bLegacyDB;

    // current indexing offset
    ENTRYID             offset;

    // indexing thread info
    PVDIR_THREAD_INFO   pThrInfo;

} VDIR_INDEX_GLOBALS, *PVDIR_INDEX_GLOBALS;

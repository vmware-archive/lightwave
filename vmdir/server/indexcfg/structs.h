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
 * Filename: structs.h
 *
 * Abstract:
 *
 * Directory indexer module
 *
 * Private Structures
 *
 */

typedef struct _VDIR_ATTR_INDEX_INSTANCE
{
    USHORT                      usNumIndex;
    PVDIR_CFG_ATTR_INDEX_DESC   pSortName;

} VDIR_ATTR_INDEX_INSTANCE, *PVDIR_ATTR_INDEX_INSTANCE;


typedef struct _VDIR_ATTR_INDEX_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PVMDIR_MUTEX    mutex;
    PVMDIR_COND     condition;
    USHORT          usLive;

    // TRUE between indice entry modify commit to the end of indexing job
    BOOLEAN         bIndexInProgress;

    // Never delete or change the content of pCaches after an instance is added.
    // If we have no more space to add new instance, reject operation.
    PVDIR_ATTR_INDEX_INSTANCE pCaches[MAX_ATTR_INDEX_CACHE_INSTANCE];

    // Temporary holder for newly create pNewCache.  Will add it into pCaches
    // when it goes live.
    PVDIR_ATTR_INDEX_INSTANCE pNewCache;

} VDIR_ATTR_INDEX_GLOBALS, *PVDIR_ATTR_INDEX_GLOBALS;


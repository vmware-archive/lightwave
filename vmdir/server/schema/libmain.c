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
 * Module Name: Directory Schema
 *
 * Filename: libmain.c
 *
 * Abstract:
 *
 * Library Entry points
 *
 */

#include "includes.h"

/*
 *    Bootstrap initial schema from default data.
 *    Not a full schema as ones that initialize from file or entry,
 *    but with limited information that we can bootstrap schema entry from
 *    data store or file.
 *    1.  pSchema->ats.pSortName (in sort order)
 *    1.1 pSchema->ats.pSortName(pszName, usIdMap)
 *    2.  pSchema->ats.dwNumATs
 *    3.  pSchema->ats.ppSortIdMap (in sort order)
 *    4.  pSchema-> bIsBootStrapSchema = TRUE
 *    5.  pSchema->usNextId = 100
 *
 *    gVdirSchemaGlobals.pInstances[0] = bootstrap schema
 *
 */
DWORD
VmDirSchemaLibInit(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVDIR_SCHEMA_INSTANCE pSchema = NULL;

    PSTR OCTable[] = VDIR_SCHEMA_BOOTSTRP_OC_INITIALIZER;
    VDIR_SCHEMA_BOOTSTRAP_TABLE ATTable[] = VDIR_SCHEMA_BOOTSTRP_ATTR_INITIALIZER;

    // initialize gVdirSchemaGlobals
    dwError = VmDirAllocateMutex(&gVdirSchemaGlobals.mutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdirSchemaInstanceAllocate(   &pSchema,
                                            sizeof(ATTable)/sizeof(ATTable[0]),
                                            sizeof(OCTable)/sizeof(OCTable[0]),
                                            0,  // no content rule in bootstrap
                                            0,  // no structure rule in bootstrap
                                            0); // no fnameforma in bootstrap
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0 ; dwCnt < sizeof(ATTable)/sizeof(ATTable[0]); dwCnt++)
    {
        dwError = VmDirSchemaParseStrToATDesc(
                ATTable[dwCnt].pszDesc,
                pSchema->ats.pATSortName+dwCnt);
        BAIL_ON_VMDIR_ERROR(dwError);

        pSchema->ats.pATSortName[dwCnt].usAttrID = ATTable[dwCnt].usAttrID;
    }

    qsort(pSchema->ats.pATSortName,
          pSchema->ats.usNumATs,
          sizeof(VDIR_SCHEMA_AT_DESC),
          VdirSchemaPATNameCmp);

    dwError = VmDirAllocateMemory(
            sizeof(PVDIR_SCHEMA_AT_DESC) * pSchema->ats.usNumATs,
            (PVOID*)&pSchema->ats.ppATSortIdMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0 ; dwCnt < sizeof(ATTable)/sizeof(ATTable[0]); dwCnt++)
    {
        pSchema->ats.ppATSortIdMap[pSchema->ats.pATSortName[dwCnt].usAttrID -1] =
                &pSchema->ats.pATSortName[dwCnt];
    }

    pSchema-> bIsBootStrapSchema = TRUE;
    pSchema->ats.usNextId = MAX_RESERVED_ATTR_ID_MAP + 1;

    dwError = VdirSyntaxLoad();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdirMatchingRuleLoad();
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    gVdirSchemaGlobals.pSchema = pSchema;
    dwError = VdirSchemaCtxAcquireInLock(TRUE, &gVdirSchemaGlobals.pCtx); // add self reference
    BAIL_ON_VMDIR_ERROR(dwError);

    assert(gVdirSchemaGlobals.pCtx);

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Startup bootstrap schema instance (%p)", gVdirSchemaGlobals.pSchema);

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSchemaLibInit failed (%d)", dwError);

    gVdirSchemaGlobals.pSchema = NULL;

    if (pSchema)
    {
        VdirSchemaInstanceFree(pSchema);
    }

    goto cleanup;
}

/*
 * This always call in single thread mode during startup or from tools.
 */
DWORD
VmDirSchemaInitializeViaFile(
    PCSTR pszSchemaFilePath
    )
{
    DWORD     dwError = 0;
    PVDIR_ENTRY    pEntry = NULL;
    BOOLEAN   bInLock = FALSE;

    if (IsNullOrEmptyString(pszSchemaFilePath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaInitalizeFileToEntry(
            pszSchemaFilePath,
            &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaInitializeViaEntry(pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);
    // globals takes over pEntry, use to write to db later
    gVdirSchemaGlobals.pLoadFromFileEntry = pEntry;

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    return dwError;

error:

    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }

    goto cleanup;
}

/*
 * Create a new schema cache via pEntry, then active this cache.
 */
DWORD
VmDirSchemaInitializeViaEntry(
    PVDIR_ENTRY    pEntry
    )
{
    BOOLEAN  bLoadOk = TRUE;
    BOOLEAN  bInLock = FALSE;
    DWORD    dwError = 0;

    PVDIR_SCHEMA_CTX pLiveCtx = NULL;
    PVDIR_SCHEMA_INSTANCE pInstance = NULL;
    PVDIR_SCHEMA_INSTANCE pLiveInstance = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    // get reference to current schema, to clean up later
    pLiveInstance = gVdirSchemaGlobals.pSchema;
    pLiveCtx      = gVdirSchemaGlobals.pCtx;

    // instantiate a schema cache - pInstance
    dwError = VdirSchemaInstanceInitViaEntry(
            pEntry,
            &pInstance);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirSchemaGlobals.pSchema = pInstance;

    dwError = VdirSchemaCtxAcquireInLock(TRUE, &gVdirSchemaGlobals.pCtx); // add self reference
    BAIL_ON_VMDIR_ERROR(dwError);

    assert(gVdirSchemaGlobals.pCtx);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Startup schema instance (%p)", gVdirSchemaGlobals.pSchema);

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

cleanup:

    if (bLoadOk)
    {
        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL,
            "Schema - AttributeTypes:(size=%d, nextid=%d) Objectclasses:(size=%d)",
            pEntry->pSchemaCtx->pSchema->ats.usNumATs,
            pEntry->pSchemaCtx->pSchema->ats.usNextId,
            pEntry->pSchemaCtx->pSchema->ocs.usNumOCs);
    }

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    if (pLiveCtx)
    {
        VmDirSchemaCtxRelease(pLiveCtx);
    }

    return dwError;

error:

    if (pInstance)
    {
        VdirSchemaInstanceFree(pInstance);
    }

    gVdirSchemaGlobals.pSchema = pLiveInstance;
    gVdirSchemaGlobals.pCtx = pLiveCtx;
    pLiveCtx = NULL;

    bLoadOk = FALSE;

    goto cleanup;
}

VOID
VmDirSchemaLibShutdown(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    // release live context and live schema
    VmDirSchemaCtxRelease(gVdirSchemaGlobals.pCtx);
    VMDIR_SAFE_FREE_MEMORY(gVdirSchemaGlobals.pszDN);

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirSchemaGlobals.mutex);

    VMDIR_SAFE_FREE_MUTEX( gVdirSchemaGlobals.mutex );
}


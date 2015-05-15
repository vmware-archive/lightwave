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
 * Filename: libmain.c
 *
 * Abstract:
 *
 * Library Entry points
 *
 */

#include "includes.h"

/*
 *
 */
DWORD
VmDirAttrIndexLibInit(
    VOID
    )
{
    DWORD       dwError = 0;
    VDIR_ENTRY       indiceEntry = {0};
    PVDIR_BACKEND_INTERFACE pBE = NULL;

    VmDirLog( LDAP_DEBUG_TRACE, "InitializeAttrIndex: Begin" );

    // Initialize gVdirAttrIndexGlobals
    dwError = VmDirAllocateMutex( &gVdirAttrIndexGlobals.mutex );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gVdirAttrIndexGlobals.condition);
    BAIL_ON_VMDIR_ERROR(dwError);

    pBE = VmDirBackendSelect(CFG_INDEX_ENTRY_DN);
    assert(pBE);

    dwError = pBE->pfnBESimpleIdToEntry(
            CFG_INDEX_ENTRY_ID,
            &indiceEntry);
    if (dwError == 0)
    {
        dwError = VdirAttrIndexInitViaEntry(&indiceEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (dwError == ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        dwError = 0;  // no indices entry yet, lets continue.
    }
    else
    {
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#if 0
    // fire up indexing thread
    dwError = InitializeIndexingThread();
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

cleanup:

    VmDirFreeEntryContent(&indiceEntry);

    VmDirLog( LDAP_DEBUG_TRACE, "InitializeAttrIndex: End" );

    return dwError;

error:

    goto cleanup;
}


VOID
VmDirAttrIndexLibShutdown(
    VOID
    )
{
    int     iCnt = 0;
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);
    for (iCnt = 0; iCnt <= gVdirAttrIndexGlobals.usLive; iCnt++)
    {
        VdirAttrIdxCacheFree(gVdirAttrIndexGlobals.pCaches[iCnt]);
    }
    if (gVdirAttrIndexGlobals.pNewCache)
    {
        VdirAttrIdxCacheFree(gVdirAttrIndexGlobals.pNewCache);
        gVdirAttrIndexGlobals.pNewCache = NULL;
    }
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    // Un-Initialize gVdirAttrIndexGlobals
    VMDIR_SAFE_FREE_CONDITION( gVdirAttrIndexGlobals.condition );
    VMDIR_SAFE_FREE_MUTEX( gVdirAttrIndexGlobals.mutex );

    return;
}


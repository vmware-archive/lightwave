/*
 * Copyright ©2017 VMware, Inc.  All Rights Reserved.
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
 * Module Name: clusterstate
 *
 * Filename: libmain.c
 *
 * Abstract:
 *
 * Library Entry points
 *
 */

#include "includes.h"

////////////////////////////////////////////////////////////////////////////////////////
// custerstate global variable - managed by clusterthr daemon thread.
////////////////////////////////////////////////////////////////////////////////////////
static
VMDIR_CLUSTER_STATE gClusterState =
{
    VMDIR_SF_INIT(.pRWLock, NULL),
    VMDIR_SF_INIT(.bReload, FALSE),
    VMDIR_SF_INIT(.bEnabled, FALSE),
    VMDIR_SF_INIT(.pNodeSelf, NULL),
    VMDIR_SF_INIT(.phmNodes, NULL),
    VMDIR_SF_INIT(.pSiteList, NULL),
};

PVMDIR_CLUSTER_STATE gpClusterState = &gClusterState;

static
DWORD
_VmDirAllocClusterState(
    VOID
    );

DWORD
VmDirClusterLibInit(
    VOID
    )
{
    DWORD       dwError = 0;

    if (!VmDirIsRegionalMasterEnabled() || !VmDirSchemaSupportSingleMaster())
    {
        goto cleanup;
    }

    dwError = _VmDirAllocClusterState();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirClusterLoadCache();
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirInitClusterStateThread();

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * Before calling, need to make sure
 *  1. LDAP port is closed
 *  2. REST port is closed
 */
VOID
VmDirClusterLibShutdown(
    VOID
    )
{
    VmDirClusterFreeCache();
}

static
DWORD
_VmDirAllocClusterState(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmDirAllocateRWLock(&gpClusterState->pRWLock);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
                &gpClusterState->phmNodes,
                LwRtlHashDigestPstrCaseless,
                LwRtlHashEqualPstrCaseless,
                NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_CLUSTER_SITE_LIST), (PVOID)&gpClusterState->pSiteList);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

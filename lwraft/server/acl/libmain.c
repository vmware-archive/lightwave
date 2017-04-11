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
 * Module Name: Directory ACL
 *
 * Filename: libmain.c
 *
 * Abstract:
 *
 * Library Entry points
 *
 */

#include "includes.h"

static
DWORD
VmDirInitSidGenState(
    PVDIR_SID_GEN_STATE pGsidGenState
    );

static
VOID
VmDirFreeSidGenState(
    PVDIR_SID_GEN_STATE pGsidGenState
    );

DWORD
VmDirVmAclInit(
    VOID
    )
{
    DWORD dwError = 0;

    if (gSidGenState.pHashtable)
    {
        VmDirFreeSidGenState(&gSidGenState);
    }

    dwError = VmDirInitSidGenState(&gSidGenState);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirVmAclShutdown(
    VOID
    )
{
    if ( gSidGenState.pStack )
    {
        VmDirFreeTSStack( gSidGenState.pStack );
        gSidGenState.pStack = NULL;
    }

    // gSidGenState.pRIDSynchThr will be signal and joined by main thr
    VmDirFreeSidGenState(&gSidGenState);

    return;
}

static
LW_PCVOID
_DomainSIDGenStateGetKey(
    PLW_HASHTABLE_NODE      pNode,
    PVOID                   pUnused
    )
{
    PVDIR_DOMAIN_SID_GEN_STATE pDomainSIDGenRec = LW_STRUCT_FROM_FIELD(pNode, VDIR_DOMAIN_SID_GEN_STATE, Node);

    return pDomainSIDGenRec->pszDomainDn;
}

static
ULONG
_LwRtlHashDigestPstr(
    LW_PCVOID pKey,
    PVOID pUnused
    )
{
    ULONG           ulDigest = 0;
    VDIR_BERVALUE   bvDn = VDIR_BERVALUE_INIT;
    PCSTR           pszStr = NULL;

    bvDn.lberbv.bv_val = (PSTR)pKey;
    bvDn.lberbv.bv_len = VmDirStringLenA(bvDn.lberbv.bv_val);

    //ignore failures here
    VmDirNormalizeDNWrapper(&bvDn);
    pszStr = (PCSTR)BERVAL_NORM_VAL(bvDn);

    if (pszStr)
    {
        while (*pszStr)
        {
            ulDigest = ulDigest * 31 + *(pszStr++);
        }
    }

    VmDirFreeBervalContent(&bvDn);
    return ulDigest;
}

static
BOOLEAN
_LwRtlHashEqualPstr(
    LW_PCVOID pKey1,
    LW_PCVOID pKey2,
    PVOID pUnused
    )
{
    BOOLEAN bRet = FALSE;
    VDIR_BERVALUE bvDn1 = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE bvDn2 = VDIR_BERVALUE_INIT;
    PCSTR pszStr1 = NULL;
    PCSTR pszStr2 = NULL;

    bvDn1.lberbv.bv_val = (PSTR)pKey1;
    bvDn1.lberbv.bv_len = VmDirStringLenA(bvDn1.lberbv.bv_val);

    bvDn2.lberbv.bv_val = (PSTR)pKey2;
    bvDn2.lberbv.bv_len = VmDirStringLenA(bvDn2.lberbv.bv_val);

    VmDirNormalizeDNWrapper(&bvDn1);
    VmDirNormalizeDNWrapper(&bvDn2);

    pszStr1 = (PCSTR)BERVAL_NORM_VAL(bvDn1);
    pszStr2 = (PCSTR)BERVAL_NORM_VAL(bvDn2);

    bRet = ((pszStr1 == NULL && pszStr2 == NULL) ||
            (pszStr1 != NULL && pszStr2 != NULL &&
            (VmDirStringCompareA(pszStr1, pszStr2, TRUE) == 0 )));

    VmDirFreeBervalContent(&bvDn1);
    VmDirFreeBervalContent(&bvDn2);
    return bRet;
}

static
DWORD
VmDirInitSidGenState(
    PVDIR_SID_GEN_STATE pGsidGenState
    )
{
    DWORD dwError = 0;

    dwError = LwRtlCreateHashTable(
                    &pGsidGenState->pHashtable,
                    _DomainSIDGenStateGetKey,
                    _LwRtlHashDigestPstr,
                    _LwRtlHashEqualPstr,
                    NULL,
                    VMDIR_DOMAIN_SID_GEN_HASH_TABLE_SIZE);
    dwError = LwNtStatusToWin32Error(dwError);
    BAIL_ON_VMDIR_ERROR(dwError);

	VmDirAllocateMutex( &pGsidGenState->mutex );

error:
	return dwError;
}

static
VOID
VmDirFreeSidGenState(
    PVDIR_SID_GEN_STATE pGsidGenState
    )
{
    BOOLEAN bInLock = FALSE;
    PLW_HASHTABLE_NODE pNode = NULL;
    LW_HASHTABLE_ITER iter = LW_HASHTABLE_ITER_INIT;

    if (!pGsidGenState)
    {
        goto cleanup;
    }

    if( VmDirIsMutexInitialized(pGsidGenState->mutex) != FALSE )
    {
        VMDIR_LOCK_MUTEX(bInLock, pGsidGenState->mutex);
    }

    while (gSidGenState.pHashtable && (pNode = LwRtlHashTableIterate(gSidGenState.pHashtable, &iter)))
    {
        PVDIR_DOMAIN_SID_GEN_STATE pState = (PVDIR_DOMAIN_SID_GEN_STATE)LW_STRUCT_FROM_FIELD(pNode, VDIR_DOMAIN_SID_GEN_STATE, Node);
        LwRtlHashTableRemove(pGsidGenState->pHashtable, &pState->Node);
        VmDirFreeOrgState(pState);
    }

    VMDIR_UNLOCK_MUTEX(bInLock, pGsidGenState->mutex);

    VMDIR_SAFE_FREE_MUTEX( pGsidGenState->mutex );
    LwRtlFreeHashTable(&pGsidGenState->pHashtable);

cleanup:
    return;
}

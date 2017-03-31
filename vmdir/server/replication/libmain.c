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
 * Module Name: Replication
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
int
LoadReplicationAgreements();

static
int
ProcessReplicationAgreementEntry(
    VDIR_ENTRY *         pReplEntry
    );

DWORD
VmDirReplAgrEntryToInMemory(
    PVDIR_ENTRY                     pEntry,
    PVMDIR_REPLICATION_AGREEMENT *  ppReplAgr)
{
    DWORD                           dwError = 0;
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr;
    PVDIR_ATTRIBUTE                 pAttr = NULL;
    VDIR_BERVALUE                   bv = VDIR_BERVALUE_INIT;

    if (!ppReplAgr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    dwError = VmDirAllocateMemory( sizeof( VMDIR_REPLICATION_AGREEMENT ), (PVOID *)&pReplAgr );
    BAIL_ON_VMDIR_ERROR( dwError );

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        if (VmDirStringCompareA(pAttr->pATDesc->pszName, ATTR_LABELED_URI, FALSE) == 0)
        {
            VmDirStringCpyA( pReplAgr->ldapURI, VMDIR_MAX_LDAP_URI_LEN, pAttr->vals[0].lberbv.bv_val );
            continue;
        }
        if (VmDirStringCompareA(pAttr->pATDesc->pszName, ATTR_LAST_LOCAL_USN_PROCESSED, FALSE) == 0)
        {
            dwError = VmDirBervalContentDup( &pAttr->vals[0], &pReplAgr->lastLocalUsnProcessed );
            BAIL_ON_VMDIR_ERROR( dwError );
            continue;
        }
        if (VmDirStringCompareA(pAttr->pATDesc->pszName, ATTR_DN, FALSE) == 0)
        {
            dwError = VmDirBervalContentDup( &pAttr->vals[0], &pReplAgr->dn );
            BAIL_ON_VMDIR_ERROR( dwError );

            if (pReplAgr->dn.bvnorm_val == NULL)
            {
                assert( pEntry->pSchemaCtx );
                dwError = VmDirNormalizeDN( &(pReplAgr->dn), pEntry->pSchemaCtx);
                BAIL_ON_VMDIR_ERROR( dwError );
            }

            continue;
        }
    }

    if (pReplAgr->lastLocalUsnProcessed.lberbv.bv_val == NULL) // Since ATTR_LAST_LOCAL_USN_PROCESSED is an optional attribute.
    {
        bv.lberbv.bv_val = VMDIR_DEFAULT_REPL_LAST_USN_PROCESSED;
        bv.lberbv.bv_len = VMDIR_DEFAULT_REPL_LAST_USN_PROCESSED_LEN;

        dwError = VmDirBervalContentDup( &bv, &pReplAgr->lastLocalUsnProcessed );
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,"Replication partner: (%s) lastLocalUsnProcessed: (%s)",
                   pReplAgr->ldapURI, pReplAgr->lastLocalUsnProcessed.lberbv_val);

    *ppReplAgr = pReplAgr;

cleanup:
    return dwError;

error:
    *ppReplAgr = NULL;

    VmDirFreeReplicationAgreement(pReplAgr);

    goto cleanup;
}

DWORD
VmDirConstructReplAgr(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    PCSTR                           pszReplURI,
    PCSTR                           pszLastLocalUsnProcessed,
    PCSTR                           pszReplAgrDN,
    PVMDIR_REPLICATION_AGREEMENT *  ppReplAgr)
{
    DWORD                           dwError = 0;
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr = NULL;
    VDIR_BERVALUE                   bv = VDIR_BERVALUE_INIT;

    if (!pSchemaCtx || IsNullOrEmptyString(pszReplURI) ||
        IsNullOrEmptyString(pszLastLocalUsnProcessed) || IsNullOrEmptyString(pszReplAgrDN) || !ppReplAgr )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    dwError = VmDirAllocateMemory( sizeof( VMDIR_REPLICATION_AGREEMENT ), (PVOID *)&pReplAgr );
    BAIL_ON_VMDIR_ERROR( dwError );

    VmDirStringCpyA( pReplAgr->ldapURI, VMDIR_MAX_LDAP_URI_LEN, pszReplURI );

    bv.lberbv.bv_val = (PSTR) pszLastLocalUsnProcessed;
    bv.lberbv.bv_len = VmDirStringLenA(bv.lberbv.bv_val);

    dwError = VmDirBervalContentDup( &bv, &pReplAgr->lastLocalUsnProcessed );
    BAIL_ON_VMDIR_ERROR( dwError );

    bv.lberbv.bv_val = (PSTR) pszReplAgrDN;
    bv.lberbv.bv_len = VmDirStringLenA(bv.lberbv.bv_val);

    dwError = VmDirBervalContentDup( &bv, &pReplAgr->dn );
    BAIL_ON_VMDIR_ERROR( dwError );

    if (pReplAgr->dn.bvnorm_val == NULL)
    {
        dwError = VmDirNormalizeDN( &(pReplAgr->dn), pSchemaCtx);
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    *ppReplAgr = pReplAgr;

cleanup:
    return dwError;

error:
    *ppReplAgr = NULL;
    if (pReplAgr)
    {
        VmDirFreeBervalContent( &pReplAgr->lastLocalUsnProcessed );
        VmDirFreeBervalContent( &pReplAgr->dn );
        VMDIR_SAFE_FREE_MEMORY( pReplAgr );
    }
    goto cleanup;
}

/*
 *
 */
DWORD
VmDirReplicationLibInit(
    VOID
    )
{
    DWORD       dwError = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "VmDirReplicationLibInit: Begin" );

    dwError = LoadReplicationAgreements( );
    BAIL_ON_VMDIR_ERROR(dwError);

    // fire up replication thread
    dwError = InitializeReplicationThread();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "VmDirReplicationLibInit: End" );
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirFreeReplicationAgreement(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr
    )
{
    if (pReplAgr)
    {
        VmDirFreeBervalContent( &(pReplAgr->lastLocalUsnProcessed) );
        VmDirFreeBervalContent( &(pReplAgr->dn) );
        VMDIR_SAFE_FREE_MEMORY( pReplAgr );
    }

    return;
}

// VmDirRemoveDeletedRAsFromCache() Remove RAs from gVmdirReplAgrs that are marked as isDeleted = TRUE
VOID
VmDirRemoveDeletedRAsFromCache()
{
    PVMDIR_REPLICATION_AGREEMENT    pPrevReplAgr = NULL; // or pointing to a valid (non-deleted) RA
    PVMDIR_REPLICATION_AGREEMENT    pCurrReplAgr = NULL;
    PVMDIR_REPLICATION_AGREEMENT    pNextReplAgr = NULL;
    PVMDIR_REPLICATION_AGREEMENT    pNewStartReplAgr = NULL;
    BOOLEAN                         bInReplAgrsLock = FALSE;

    VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);

    for (pCurrReplAgr = gVmdirReplAgrs; pCurrReplAgr; pCurrReplAgr = pNextReplAgr)
    {
        pNextReplAgr = pCurrReplAgr->next;

        if (pCurrReplAgr->isDeleted)
        {
            VmDirFreeReplicationAgreement( pCurrReplAgr );
            if (pPrevReplAgr != NULL)
            {
                pPrevReplAgr->next = pNextReplAgr;
            }
        }
        else // valid (non-deleted) RA
        {
            // Set New Start RA pointer, if not already set.
            if (pNewStartReplAgr == NULL)
            {
                pNewStartReplAgr = pCurrReplAgr;
            }
            pPrevReplAgr = pCurrReplAgr;
        }
    }

    gVmdirReplAgrs = pNewStartReplAgr;

    VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
}

static
int
LoadReplicationAgreements()
{
    // Load my Replication Agreements
    VDIR_OPERATION  op = {0};
    PVDIR_FILTER    replAgrFilter = NULL;
    DWORD           dwError = 0;
    int             iCnt = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "LoadReplicationAgreements: Begin" );

    if ( gVmdirServerGlobals.serverObjDN.lberbv.bv_val != NULL )
    {

        dwError = VmDirInitStackOperation( &op,
                                           VDIR_OPERATION_TYPE_INTERNAL,
                                           LDAP_REQ_SEARCH,
                                           NULL );
        BAIL_ON_VMDIR_ERROR(dwError);

        op.pBEIF = VmDirBackendSelect( gVmdirServerGlobals.serverObjDN.lberbv.bv_val );
        assert(op.pBEIF);

        if (VmDirBervalContentDup( &gVmdirServerGlobals.serverObjDN, &op.reqDn ) != 0)
        {
            VmDirLog( LDAP_DEBUG_ANY, "LoadReplicationAgreements: BervalContentDup failed." );
            dwError = -1;
            BAIL_ON_VMDIR_ERROR( dwError );
        }
        op.request.searchReq.scope = LDAP_SCOPE_SUBTREE;
        if (VmDirAllocateMemory( sizeof( VDIR_FILTER ), (PVOID *)&replAgrFilter ) != 0)
        {
            VmDirLog( LDAP_DEBUG_ANY, "LoadReplicationAgreements: VmDirAllocateMemory failed. " );
            dwError = -1;
            BAIL_ON_VMDIR_ERROR( dwError );
        }

        op.request.searchReq.filter = replAgrFilter;

        replAgrFilter->choice = LDAP_FILTER_EQUALITY;
        replAgrFilter->filtComp.ava.type.lberbv.bv_val = ATTR_OBJECT_CLASS;
        replAgrFilter->filtComp.ava.type.lberbv.bv_len = ATTR_OBJECT_CLASS_LEN;
        if ((replAgrFilter->filtComp.ava.pATDesc = VmDirSchemaAttrNameToDesc(
                                                        op.pSchemaCtx,
                                                        replAgrFilter->filtComp.ava.type.lberbv.bv_val)) == NULL)
        {
            dwError = -1;
            VmDirLog( LDAP_DEBUG_ANY, "LoadReplicationAgreements: Getting pATDesc for ATTR_OBJECT_CLASS failed "
                      "(hmm... STRANGE). " );
            BAIL_ON_VMDIR_ERROR( dwError );
        }
        replAgrFilter->filtComp.ava.value.lberbv.bv_val = OC_REPLICATION_AGREEMENT;
        replAgrFilter->filtComp.ava.value.lberbv.bv_len = OC_REPLICATION_AGREEMENT_LEN;
        if (VmDirSchemaBervalNormalize( op.pSchemaCtx, replAgrFilter->filtComp.ava.pATDesc,
                                        &(replAgrFilter->filtComp.ava.value) ) != LDAP_SUCCESS)
        {
            dwError = -1;
            VmDirLog( LDAP_DEBUG_ANY, "LoadReplicationAgreements: Attribute value normalization failed for "
                      "filter type = %s", replAgrFilter->filtComp.ava.type.lberbv.bv_val  );
            BAIL_ON_VMDIR_ERROR( dwError );
        }
        replAgrFilter->next = NULL;

        if ((dwError = VmDirInternalSearch( &op )) != 0)
        {
            VmDirLog( LDAP_DEBUG_ANY, "LoadReplicationAgreements: InternalSearch for Replication Agreements failed. "
                      "Error code: %d, Error string: %s", dwError, VDIR_SAFE_STRING(op.ldapResult.pszErrMsg));
            dwError = -1;
            BAIL_ON_VMDIR_ERROR( dwError );
        }

        // load all replication agreements
        for (iCnt=0; iCnt < op.internalSearchEntryArray.iSize; iCnt++)
        {
            dwError = ProcessReplicationAgreementEntry( op.internalSearchEntryArray.pEntry + iCnt );
            BAIL_ON_VMDIR_ERROR( dwError );
        }
    }

cleanup:
    VmDirFreeOperationContent(&op);
    VmDirLog( LDAP_DEBUG_TRACE, "LoadReplicationAgreements: End" );

    return dwError;

error:
    goto cleanup;
}

static
int
ProcessReplicationAgreementEntry(
    VDIR_ENTRY *         pReplEntry
    )
{
    PVMDIR_REPLICATION_AGREEMENT    replAgr = NULL;
    int                             retVal = LDAP_SUCCESS;

    if (VmDirReplAgrEntryToInMemory( pReplEntry, &replAgr ) != 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    replAgr->next = gVmdirReplAgrs;
    gVmdirReplAgrs = replAgr;

cleanup:
    return retVal;

error:
    goto cleanup;
}


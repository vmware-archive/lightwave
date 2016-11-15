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



#include "includes.h"

DWORD
VmDirMDBSimpleEIdToEntry(
    ENTRYID         eId,
    PVDIR_ENTRY     pEntry)
{
    DWORD               dwError = 0;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    VDIR_BACKEND_CTX    mdbBECtx = {0};
    BOOLEAN             bHasTxn = FALSE;

    assert(pEntry);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBTxnBegin(&mdbBECtx, VDIR_BACKEND_TXN_READ);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    dwError = VmDirMDBEIdToEntry(   &mdbBECtx,
                                    pSchemaCtx,
                                    eId,
                                    pEntry,
                                    VDIR_BACKEND_ENTRY_LOCK_READ);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBTxnCommit(&mdbBECtx);
    bHasTxn = FALSE;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    mdbBECtx.pBEPrivate = NULL;
    VmDirBackendCtxContentFree(&mdbBECtx);

    return dwError;

error:

    if (bHasTxn)
    {
        VmDirMDBTxnAbort(&mdbBECtx);
    }

    goto cleanup;
}

DWORD
VmDirMDBSimpleDnToEntry(
    PSTR        pszEntryDN,
    PVDIR_ENTRY pEntry
    )
{
    DWORD               dwError = 0;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    VDIR_BERVALUE       entryDn = VDIR_BERVALUE_INIT;
    VDIR_BACKEND_CTX    mdbBECtx = {0};
    BOOLEAN             bHasTxn = FALSE;

    assert(pEntry);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    entryDn.lberbv.bv_val = pszEntryDN;
    entryDn.lberbv.bv_len = VmDirStringLenA(entryDn.lberbv.bv_val);

    dwError = VmDirMDBTxnBegin(&mdbBECtx, VDIR_BACKEND_TXN_READ);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    dwError = VmDirMDBDNToEntry(    &mdbBECtx,
                                    pSchemaCtx,
                                    &entryDn,
                                    pEntry,
                                    VDIR_BACKEND_ENTRY_LOCK_READ);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBTxnCommit(&mdbBECtx);
    bHasTxn = FALSE;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    mdbBECtx.pBEPrivate = NULL;
    VmDirBackendCtxContentFree(&mdbBECtx);
    VmDirFreeBervalContent(&entryDn);

    return dwError;

error:

    if (bHasTxn)
    {
        VmDirMDBTxnAbort(&mdbBECtx);
    }

    goto cleanup;
}

// To get the current max ENTRYID
DWORD
VmDirMDBMaxEntryId(
    ENTRYID*            pEId)
{
    DWORD           dwError = 0;
    PVDIR_DB_TXN    pTxn = NULL;
    MDB_val         key = {0};
    MDB_val         value  = {0};
    unsigned char   EIDBytes[sizeof( ENTRYID )] = {0};

    assert(pEId);

    dwError = mdb_txn_begin( gVdirMdbGlobals.mdbEnv, NULL, MDB_RDONLY, &pTxn );
    BAIL_ON_VMDIR_ERROR(dwError);

    key.mv_data = &EIDBytes[0];
    MDBEntryIdToDBT(BE_MDB_ENTRYID_SEQ_KEY, &key);

    dwError =  mdb_get(pTxn, gVdirMdbGlobals.mdbSeqDBi, &key, &value);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_txn_commit(pTxn);
    pTxn = NULL;
    BAIL_ON_VMDIR_ERROR(dwError);

    assert(value.mv_size == sizeof(ENTRYID));
    *pEId = *((ENTRYID*)value.mv_data);

cleanup:

    return dwError;

error:

    if (pTxn)
    {
        mdb_txn_abort(pTxn);
    }

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "VmDirMDBMaxEntryId: failed with error (%d),(%s)",
             dwError, mdb_strerror(dwError) );

    VMDIR_SET_BACKEND_ERROR(dwError);
    goto cleanup;
}

/*
 * Get the next available USN number.
 * Note that USN fetched from the backend database is the next USN to be consumed.
 */
DWORD
VmDirMDBGetNextUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    USN *               pUsn)
{
    DWORD           dwError = 0;
    PVDIR_DB_TXN    pTxn = NULL;
    PVDIR_DB_TXN    pLocalTxn = NULL;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value  = {0};
    unsigned char   USNKeyBytes[sizeof( USN )] = {0};
    unsigned char   USNValueBytes[sizeof( USN )] = {0};
    USN             localUSN = 0;
    BOOLEAN         bRevertUSN = FALSE;
    BOOLEAN         bUnsetMaxUSN = TRUE;

    assert( pBECtx && pUsn );

    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;
    if (pTxn)
    {
        pLocalTxn = pTxn;
    }
    else
    {
        dwError = mdb_txn_begin( gVdirMdbGlobals.mdbEnv, BE_DB_PARENT_TXN_NULL, BE_DB_FLAGS_ZERO, &pLocalTxn );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    key.mv_data = &USNKeyBytes[0];
    MDBEntryIdToDBT(BE_MDB_USN_SEQ_KEY, &key);

    dwError =  mdb_get(pLocalTxn, gVdirMdbGlobals.mdbSeqDBi, &key, &value);
    BAIL_ON_VMDIR_ERROR(dwError);

    assert( value.mv_size == sizeof(USN) );
    localUSN = *((USN*)value.mv_data);

    if (gVmdirServerGlobals.initialNextUSN == (USN)0)
    {
        gVmdirServerGlobals.initialNextUSN = localUSN;
    }
    *((USN*)&USNValueBytes[0]) = localUSN + 1;
    value.mv_size = sizeof(USN);
    value.mv_data = &USNValueBytes[0];

    dwError =  mdb_put(pLocalTxn, gVdirMdbGlobals.mdbSeqDBi, &key, &value, BE_DB_FLAGS_ZERO);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pBECtx->wTxnUSN == 0)
    {   // capture and set outstanding USN within txn scope
        pBECtx->wTxnUSN = localUSN;

        dwError = VmDirBackendAddOutstandingUSN( pBECtx );
        BAIL_ON_VMDIR_ERROR(dwError);
        bRevertUSN = TRUE;
    }
    else
    {   // if BECtx has wTxnUSN already (nested txn), it should be smaller than new USN here.
        if (pBECtx->wTxnUSN >= localUSN)
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "nested OP/TXN USN ordering logic error: (%ld)(%ld)",
                      pBECtx->wTxnUSN, localUSN );
            dwError = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        VmDirBackendSetMaxOutstandingUSN(pBECtx, localUSN);
        bUnsetMaxUSN = TRUE;
    }

    if (pLocalTxn != pTxn)
    {
        dwError = mdb_txn_commit(pLocalTxn);
        pLocalTxn = NULL;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pUsn = localUSN;

cleanup:

    return dwError;

error:

    if (bUnsetMaxUSN)
    {
        VmDirBackendSetMaxOutstandingUSN(pBECtx, localUSN - 1);
    }

    if (bRevertUSN)
    {
        VmDirBackendRemoveOutstandingUSN( pBECtx );
        pBECtx->wTxnUSN = 0;
    }

    if (pLocalTxn && pLocalTxn != pTxn)
    {
        mdb_txn_abort(pLocalTxn);
    }

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "VmDirMDBGetNextUSN: failed with error (%d),(%s)",
             dwError, mdb_strerror(dwError) );

    dwError = MDBToBackendError(dwError, EACCES, VMDIR_ERROR_UNWILLING_TO_PERFORM, pBECtx, "GetNextUSN");

    goto cleanup;
}

/* MdbAddEntry: Creates an entry in the MDB DBs.
 *
 * Returns: BE error codes.
 *
 */
DWORD
VmDirMDBAddEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ENTRY         pEntry)
{
    DWORD             dwError = 0;
    ENTRYID           entryId = 0;
    VDIR_DB_TXN*      pTxn = NULL;
    VDIR_BERVALUE     encodedEntry = VDIR_BERVALUE_INIT;
    VDIR_ATTRIBUTE *  nextAttr = NULL;

    assert( pEntry && pBECtx && pBECtx->pBEPrivate );

    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;

    dwError = VmDirEncodeEntry( pEntry, &encodedEntry );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pEntry->eId != 0)    // Reserved entries have eId already
    {
        entryId = pEntry->eId;
    }
    else
    {
        VmDirRaftNextNewEntryId(&entryId);
        pEntry->eId = entryId;
    }

    assert( entryId > 0 );

    if ((dwError = MDBCreateParentIdIndex(pBECtx, &(pEntry->pdn), entryId)) != 0)
    {
        dwError = MDBToBackendError(dwError, ERROR_BACKEND_ENTRY_NOTFOUND,
                                    ERROR_BACKEND_PARENT_NOTFOUND, pBECtx, "CreateParentIdIndex");
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    // Update DN index first. this make sure we always return ERROR_BACKEND_ENTRY_EXISTS in such case.
    for (nextAttr = pEntry->attrs; nextAttr != NULL; nextAttr = nextAttr->next)
    {
        if (VmDirStringCompareA(nextAttr->type.lberbv.bv_val, ATTR_DN, FALSE) == 0)
        {
            // make sure we store normalized DN value.
            dwError = VmDirNormalizeDN( &(nextAttr->vals[0]), pEntry->pSchemaCtx );
            BAIL_ON_VMDIR_ERROR(dwError);

            if ((dwError = MdbUpdateIndicesForAttr( pTxn, &(pEntry->dn), &(nextAttr->type), nextAttr->vals, nextAttr->numVals,
                                                    entryId, BE_INDEX_OP_TYPE_CREATE)) != 0)
            {
                dwError = MDBToBackendError( dwError,
                                             MDB_KEYEXIST,
                                             ERROR_BACKEND_ENTRY_EXISTS,
                                             pBECtx,
                                             VDIR_SAFE_STRING(nextAttr->vals[0].bvnorm_val));
                BAIL_ON_VMDIR_ERROR( dwError );
            }
            if ((dwError = MdbUpdateAttrMetaData( pTxn, nextAttr, entryId, BE_INDEX_OP_TYPE_CREATE )) != 0)
            {
                dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx, "UpdateDNAttrMetaData");
                BAIL_ON_VMDIR_ERROR( dwError );
            }
        }
    }

    // Update remaining indices
    for (nextAttr = pEntry->attrs; nextAttr != NULL; nextAttr = nextAttr->next)
    {
        if (VmDirStringCompareA(nextAttr->type.lberbv.bv_val, ATTR_DN, FALSE) != 0)
        {
            if ((dwError = MdbUpdateIndicesForAttr( pTxn, &(pEntry->dn), &(nextAttr->type), nextAttr->vals, nextAttr->numVals,
                                                    entryId, BE_INDEX_OP_TYPE_CREATE)) != 0)
            {
                dwError = MDBToBackendError( dwError,
                                             MDB_KEYEXIST,
                                             ERROR_BACKEND_CONSTRAINT,
                                             pBECtx,
                                             VDIR_SAFE_STRING(nextAttr->type.lberbv.bv_val));
                BAIL_ON_VMDIR_ERROR( dwError );
            }
            if ((dwError = MdbUpdateAttrMetaData( pTxn, nextAttr, entryId, BE_INDEX_OP_TYPE_CREATE )) != 0)
            {
                dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx,
                                            VDIR_SAFE_STRING(nextAttr->type.lberbv.bv_val));
                BAIL_ON_VMDIR_ERROR( dwError );
            }
        }
    }

    // Update entry/blob database
    if ((dwError = MdbCreateEIDIndex(pTxn, entryId, &encodedEntry, TRUE /* 1st time new entry creation */)) != 0)
    {
        dwError = MDBToBackendError(dwError, MDB_KEYEXIST, ERROR_BACKEND_CONSTRAINT, pBECtx, "CreateEIDIndex");
        BAIL_ON_VMDIR_ERROR( dwError );
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY( encodedEntry.lberbv.bv_val );
    return dwError;

error:
    // TODO set pBECtx->pszBEErrorMsg?
    VMDIR_LOG_ERROR( LDAP_DEBUG_BACKEND, "BEAddEntry DN (%s),  (%u)(%s)", VDIR_SAFE_STRING(pEntry->dn.lberbv.bv_val),
                              dwError, VDIR_SAFE_STRING(pBECtx->pszBEErrorMsg));

    VMDIR_SET_BACKEND_ERROR(dwError);   // if dwError no in BE space, set to ERROR_BACKEND_ERROR

    goto cleanup;
}

/* MdbCheckRefIntegrity: Checks for the attributes that have referential integrity constraint set, that the DN attribute
 *                    values refer to existing objects.
 *
 * Returns: BE error codes.
 *
 */
DWORD
VmDirMDBCheckRefIntegrity(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ENTRY         pEntry)
{
    DWORD               dwError = 0;
    VDIR_ATTRIBUTE *    attr = NULL;

    assert( pBECtx && pBECtx->pBEPrivate  && pEntry );

    for (attr = pEntry->attrs; attr; attr = attr->next)
    {
        // SJ-TBD: Instead of checking referential integrity for hard coded attributes, we should have a
        // proprietary flag e.g. X-constraint in the attribute schema definition
        if (VmDirStringCompareA(attr->type.lberbv.bv_val, ATTR_MEMBER, FALSE) == 0)
        {
            unsigned int i = 0;
            ENTRYID   eId = 0;
            for (; i < attr->numVals; i++)
            {
                // Lookup in the DN index.
                if ((dwError = VmDirNormalizeDN( &(attr->vals[i]), pEntry->pSchemaCtx)) != 0)
                {
                    dwError = ERROR_BACKEND_OPERATIONS;
                    BAIL_ON_VMDIR_ERROR( dwError );
                }

                if ((dwError = VmDirMDBDNToEntryId( pBECtx, &(attr->vals[i]), &eId )) != 0)
                {
                    dwError = MDBToBackendError(dwError, ERROR_BACKEND_ENTRY_NOTFOUND,
                                                ERROR_BACKEND_CONSTRAINT, pBECtx,
                                                VDIR_SAFE_STRING(attr->vals[i].lberbv.bv_val));
                    BAIL_ON_VMDIR_ERROR( dwError );
                }
            }
        }
    }

cleanup:

    return dwError;

error:
    // TODO set pBECtx->pszBEErrorMsg
    VMDIR_LOG_ERROR( LDAP_DEBUG_BACKEND, "BE DN (%s) reference check, error (%u)(%s)",
                     pEntry->dn.lberbv_val, dwError, VDIR_SAFE_STRING(pBECtx->pszBEErrorMsg) );

    VMDIR_SET_BACKEND_ERROR(dwError);   // if dwError no in BE space, set to ERROR_BACKEND_ERROR

    goto cleanup;
}

/* MdbDeleteEntry: Deletes an entry in the MDB DBs.
 *
 * Returns: BE error codes.
 *
 */
DWORD
VmDirMDBDeleteEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_MODIFICATION  pMods,
    PVDIR_ENTRY         pEntry
    )
{
    DWORD                   dwError = 0;
    VDIR_MODIFICATION *     mod = NULL;
    PVDIR_DB_TXN            pTxn = NULL;

    assert( pBECtx && pBECtx->pBEPrivate && pMods && pEntry );

    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;

    // Delete child from the parentId index
    dwError = MDBDeleteParentIdIndex( pBECtx, &(pEntry->pdn), pEntry->eId );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (mod = pMods; mod != NULL; mod = mod->next)
    {
        switch (mod->operation)
        {
            case MOD_OP_DELETE:
                if ((dwError = MdbUpdateIndicesForAttr( pTxn, &(pEntry->dn), &(mod->attr.type), mod->attr.vals, mod->attr.numVals,
                                                       pEntry->eId, BE_INDEX_OP_TYPE_DELETE )) != 0)
                {
                    dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx,
                                                VDIR_SAFE_STRING(mod->attr.type.lberbv.bv_val));
                    BAIL_ON_VMDIR_ERROR( dwError );
                }
                break;
            default:
                assert( FALSE );
        }
    }

    // Delete Entry Blob
    if ((dwError = MDBDeleteEntryBlob(pBECtx, pEntry->eId)) != 0)
    {
        dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx, "MDBDeleteEntryBlob");
        BAIL_ON_VMDIR_ERROR( dwError );
    }

cleanup:
     return dwError;

error:
    VMDIR_SET_BACKEND_ERROR(dwError);   // if dwError no in BE space, set to ERROR_BACKEND_ERROR

    VMDIR_LOG_ERROR(LDAP_DEBUG_BACKEND, "VmDirMDBDeleteEntry: failed: error=%d,DN=%s",
                    dwError, VDIR_SAFE_STRING(pEntry->dn.lberbv.bv_val));
     goto cleanup;
}

/* VmDirMDBDNToEntry: For a given entry DN, reads an entry from the entry DB.
 *
 * Returns: BE error - BACKEND_ERROR, BACKEND OPERATIONS, BACKEND_ENTRY_NOTFOUND
 *
 */
DWORD
VmDirMDBDNToEntry(
    PVDIR_BACKEND_CTX           pBECtx,
    PVDIR_SCHEMA_CTX            pSchemaCtx,
    VDIR_BERVALUE*              pDn,
    PVDIR_ENTRY                 pEntry,
    VDIR_BACKEND_ENTRY_LOCKTYPE entryLockType)
{
    DWORD       dwError = LDAP_SUCCESS;
    ENTRYID     eId = {0};

    // make sure we look up normalized dn value
    dwError = VmDirNormalizeDN( pDn, pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBDNToEntryId( pBECtx, pDn, &eId );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = VmDirMDBEIdToEntry( pBECtx, pSchemaCtx, eId, pEntry, entryLockType );
    BAIL_ON_VMDIR_ERROR( dwError );

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( LDAP_DEBUG_BACKEND, "BEDNToEntry DN (%s) failed, (%u)(%s)",
                     VDIR_SAFE_STRING( pDn->bvnorm_val), dwError, VDIR_SAFE_STRING(pBECtx->pszBEErrorMsg) );

    VMDIR_SET_BACKEND_ERROR(dwError);   // if dwError no in BE space, set to ERROR_BACKEND_ERROR

    goto cleanup;
}

/* MdbEIdToEntry: For a given entry ID, reads an entry from the entry DB.
 *
 * Returns: BE error - BACKEND_ERROR, BACKEND OPERATIONS, BACKEND_ENTRY_NOTFOUND
 *
 */
DWORD
VmDirMDBEIdToEntry(
    PVDIR_BACKEND_CTX           pBECtx,
    PVDIR_SCHEMA_CTX            pSchemaCtx,
    ENTRYID                     eId,
    PVDIR_ENTRY                 pEntry,
    VDIR_BACKEND_ENTRY_LOCKTYPE entryLockType)
{
    DWORD           dwError = 0;
    VDIR_DB         mdbDBi = 0;
    PVDIR_DB_TXN    pTxn = NULL;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};
    unsigned char   eIdBytes[sizeof( ENTRYID )] = {0};
    unsigned char*  pszBlob = NULL;

    assert(pBECtx && pBECtx->pBEPrivate && pSchemaCtx && pEntry);

    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;

    mdbDBi = gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].mdbDBi;

    // Set key
    key.mv_data = &eIdBytes[0];
    MDBEntryIdToDBT(eId, &key);

    if ((dwError = mdb_get(pTxn, mdbDBi, &key, &value) ) != 0)
    {
        dwError = MDBToBackendError(dwError, MDB_NOTFOUND, ERROR_BACKEND_ENTRY_NOTFOUND, pBECtx, "EIDToEntry");
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    if ((dwError = VmDirAllocateMemory( value.mv_size, (PVOID *)&pszBlob)) != 0)
    {
        dwError = ERROR_BACKEND_OPERATIONS;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    if ((dwError = VmDirCopyMemory(pszBlob, value.mv_size, value.mv_data, value.mv_size)) != 0)
    {
        dwError = ERROR_BACKEND_OPERATIONS;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    // encodedEntry takes over pszBlob
    pEntry->encodedEntry = pszBlob;
    pszBlob = NULL;

    pEntry->eId = eId;
    dwError = VmDirDecodeEntry(pSchemaCtx, pEntry );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( LDAP_DEBUG_BACKEND, "VmDirMDBEIdToEntry, eid(%u) failed (%u)", eId, dwError);

    VMDIR_SAFE_FREE_MEMORY(pszBlob);
    VmDirFreeEntryContent( pEntry );

    VMDIR_SET_BACKEND_ERROR(dwError);   // if dwError no in BE space, set to ERROR_BACKEND_ERROR

    goto cleanup;
}

/* MdbModifyEntry: Updates an entry in the MDB DBs.
 *
 * Returns: BE error codes.
 *
 */
DWORD
VmDirMDBModifyEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_MODIFICATION*  pMods,
    PVDIR_ENTRY         pEntry
    )
{
    DWORD                   dwError = 0;
    VDIR_BERVALUE           newEncodedEntry = VDIR_BERVALUE_INIT;
    VDIR_MODIFICATION *     mod = NULL;
    PVDIR_DB_TXN            pTxn = NULL;

    assert( pBECtx && pBECtx->pBEPrivate && pMods && pEntry );

    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;

    dwError = VmDirEncodeEntry( pEntry, &newEncodedEntry );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_SAFE_FREE_MEMORY( pEntry->encodedEntry );
    // entry takes over the responsibility to free newEncodedEntry.lberbv.bv_val
    pEntry->encodedEntry = (unsigned char *)newEncodedEntry.lberbv.bv_val;

    // Delete child from the parentId index
    if (pEntry->newpdn.lberbv.bv_len)
    {
        dwError = MDBDeleteParentIdIndex( pBECtx, &(pEntry->pdn), pEntry->eId );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Create/Delete appropriate indices for DN first in case DN is already in use
    for (mod = pMods; mod != NULL; mod = mod->next)
    {
        if (mod->ignore)
        {
            continue;
        }

        if (VmDirStringCompareA(mod->attr.type.lberbv.bv_val, ATTR_DN, FALSE) == 0)
        {
            switch (mod->operation)
            {
                case MOD_OP_ADD:
                    if ((dwError = MdbUpdateIndicesForAttr( pTxn, &(pEntry->dn), &(mod->attr.type), mod->attr.vals, mod->attr.numVals,
                                                           pEntry->eId, BE_INDEX_OP_TYPE_CREATE)) != 0)
                    {
                        dwError = MDBToBackendError( dwError, MDB_KEYEXIST, ERROR_BACKEND_CONSTRAINT, pBECtx,
                                                     VDIR_SAFE_STRING(mod->attr.type.lberbv.bv_val));
                        BAIL_ON_VMDIR_ERROR( dwError );
                    }
                    break;

                case MOD_OP_DELETE:
                    if ((dwError = MdbUpdateIndicesForAttr( pTxn, &(pEntry->dn), &(mod->attr.type), mod->attr.vals, mod->attr.numVals,
                                                           pEntry->eId, BE_INDEX_OP_TYPE_DELETE )) != 0)
                    {
                        dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx,
                                                    VDIR_SAFE_STRING(mod->attr.type.lberbv.bv_val));
                        BAIL_ON_VMDIR_ERROR( dwError );
                    }
                    break;

                case MOD_OP_REPLACE:
                default:
                    assert( FALSE );
            }

            if ((dwError = MdbUpdateAttrMetaData( pTxn, &(mod->attr), pEntry->eId, BE_INDEX_OP_TYPE_UPDATE )) != 0)
            {
                dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx,
                                            VDIR_SAFE_STRING(mod->attr.type.lberbv.bv_val));
                BAIL_ON_VMDIR_ERROR( dwError );
            }
        }
    }

    // Create/Delete appropriate indices for DN
    for (mod = pMods; mod != NULL; mod = mod->next)
    {
        if (mod->ignore)
        {
            continue;
        }
        if (VmDirStringCompareA(mod->attr.type.lberbv.bv_val, ATTR_DN, FALSE) != 0)
        {
            switch (mod->operation)
            {
                case MOD_OP_ADD:
                    if ((dwError = MdbUpdateIndicesForAttr( pTxn, &(pEntry->dn), &(mod->attr.type), mod->attr.vals, mod->attr.numVals,
                                                           pEntry->eId, BE_INDEX_OP_TYPE_CREATE)) != 0)
                    {
                        dwError = MDBToBackendError( dwError, MDB_KEYEXIST, ERROR_BACKEND_CONSTRAINT, pBECtx,
                                                     VDIR_SAFE_STRING(mod->attr.type.lberbv.bv_val));
                        BAIL_ON_VMDIR_ERROR( dwError );
                    }
                    break;

                case MOD_OP_DELETE:
                    if ((dwError = MdbUpdateIndicesForAttr( pTxn, &(pEntry->dn), &(mod->attr.type), mod->attr.vals, mod->attr.numVals,
                                                           pEntry->eId, BE_INDEX_OP_TYPE_DELETE )) != 0)
                    {
                        dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx,
                                                    VDIR_SAFE_STRING(mod->attr.type.lberbv.bv_val));
                        BAIL_ON_VMDIR_ERROR( dwError );
                    }
                    break;

                case MOD_OP_REPLACE:
                default:
                    assert( FALSE );
            }

            if ((dwError = MdbUpdateAttrMetaData( pTxn, &(mod->attr), pEntry->eId, BE_INDEX_OP_TYPE_UPDATE )) != 0)
            {
                dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx,
                                            VDIR_SAFE_STRING(mod->attr.type.lberbv.bv_val));
                BAIL_ON_VMDIR_ERROR( dwError );
            }
        }
    }

    // Update Entry DB.
    if ((dwError = MdbCreateEIDIndex(pTxn, pEntry->eId, &newEncodedEntry, FALSE /* update current eId key */)) != 0)
    {
        dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx, "CreateEIDIndex");
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    if (pEntry->newpdn.lberbv.bv_len)
    {
        if ((dwError = MDBCreateParentIdIndex(pBECtx, &(pEntry->newpdn), pEntry->eId)) != 0)
        {
            dwError = MDBToBackendError(dwError, ERROR_BACKEND_ENTRY_NOTFOUND,
                                        ERROR_BACKEND_PARENT_NOTFOUND, pBECtx, "CreateParentIdIndex");
            BAIL_ON_VMDIR_ERROR( dwError );
        }
    }

cleanup:

     return dwError;

error:

    VMDIR_SET_BACKEND_ERROR(dwError);   // if dwError no in BE space, set to ERROR_BACKEND_ERROR

    VMDIR_LOG_ERROR(LDAP_DEBUG_BACKEND,
             "ModifyEntry failed: error=%d,DN=%s", dwError, VDIR_SAFE_STRING(pEntry->dn.lberbv.bv_val));

     goto cleanup;
}

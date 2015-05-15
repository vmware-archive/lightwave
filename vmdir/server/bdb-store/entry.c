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
BDBSimpleEIdToEntry(
    ENTRYID     eId,
    PVDIR_ENTRY      pEntry)
{
    DWORD   dwError = 0;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    VDIR_BACKEND_CTX    bdbBECtx = {0};

    assert(pEntry);

	dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
	BAIL_ON_VMDIR_ERROR(dwError);

    dwError = BdbEIdToEntry(
            &bdbBECtx,       // implicit txn
            pSchemaCtx,
            eId,
            pEntry,
            VDIR_BACKEND_ENTRY_LOCK_READ);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    bdbBECtx.pBEPrivate = NULL;
    VmDirBackendCtxContentFree(&bdbBECtx);

    return dwError;

error:

    goto cleanup;
}

DWORD
BDBSimpleDnToEntry(
    PSTR pszObjectDn,
    PVDIR_ENTRY pEntry
    )
{
    DWORD   dwError = 0;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    VDIR_BACKEND_CTX    bdbBECtx = {0};
    VDIR_BERVALUE entryDn = VDIR_BERVALUE_INIT;


    assert(pEntry);

	dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
	BAIL_ON_VMDIR_ERROR(dwError);

    // no need to free entryDn.lberbv.lberbv.bv_val
    entryDn.lberbv.bv_val = pszObjectDn;
    entryDn.lberbv.bv_len = VmDirStringLenA(entryDn.lberbv.bv_val);

    dwError = BdbDNToEntry(
            &bdbBECtx,       // implicit txn
            pSchemaCtx,
            &entryDn,
            pEntry,
            VDIR_BACKEND_ENTRY_LOCK_READ);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    bdbBECtx.pBEPrivate = NULL;
    VmDirBackendCtxContentFree(&bdbBECtx);

    return dwError;

error:

    goto cleanup;
}

// To get the maximum generated entryid
DWORD
BdbMaxEntryId(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID*            pEId)
{
    DWORD       dwError = 0;
    ENTRYID     llEntryId = 0;

    assert(pEId);

    dwError = gVdirBdbGlobals.bdbEidSeq->get (
            gVdirBdbGlobals.bdbEidSeq,
            BDB_TXN_NULL,   //TODO, concurrent issue?
            1,              //TODO, 0 is not allowed?
            &llEntryId,
            BDB_FLAGS_ZERO);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pEId = llEntryId;

cleanup:

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY,
            "BdbMaxEntryId: g_eidSeq->get() failed with error code: %d, error string: %s",
             dwError,
             db_strerror(dwError) );

    dwError = ERROR_BACKEND_ERROR;

    goto cleanup;
}

DWORD
BdbGetNextUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    USN *               usn)
{
    DWORD       dwError = 0;

    assert(usn);

    dwError = gVdirBdbGlobals.bdbUsnSeq->get (
                gVdirBdbGlobals.bdbUsnSeq,
                BDB_TXN_NULL,   //TODO, concurrent issue?
                1,              //TODO, 0 is not allowed?
                usn,
                BDB_FLAGS_ZERO);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

/* BdbAddEntry: Creates an entry in the BDB DBs.
 *
 * Returns: BDB error codes.
 *
 */
DWORD
BdbAddEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ENTRY         pEntry)
{
    int               retVal = 0;
    ENTRYID           entryId = 0;
    DB_TXN*           pTxn = NULL;
    VDIR_BERVALUE          encodedEntry = VDIR_BERVALUE_INIT;
    VDIR_ATTRIBUTE *       nextAttr = NULL;


    assert( pEntry != NULL && pBECtx && pBECtx->pBEPrivate );

    VmDirLog( LDAP_DEBUG_TRACE, "BdbAddEntry: Begin, entry DN = %s", pEntry->dn.lberbv.bv_val );

    pTxn = (DB_TXN*)pBECtx->pBEPrivate;

    VmDirEncodeEntry( pEntry, &encodedEntry );

    if (pEntry->eId != 0)    // Reserved entries have eId already
    {
        entryId = pEntry->eId;
    }
    else
    {
        if ((retVal = gVdirBdbGlobals.bdbEidSeq->get (
                                gVdirBdbGlobals.bdbEidSeq,
                                BDB_TXN_NULL,
                                1,          // Change the sequence value by this delta.
                                &entryId,
                                BDB_FLAGS_ZERO
                                )) != 0)
        {
            VmDirLog( LDAP_DEBUG_ANY, "BdbAddEntry: g_eidSeq->get() failed with error code: %d, error string: %s",
                      retVal, db_strerror(retVal) );
            retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
            BAIL_ON_VMDIR_ERROR( retVal );
        }
    }
    assert(entryId>0);

    if ((retVal = CreateParentIdIndex(pBECtx, &(pEntry->pdn), entryId)) != 0)
    {
        retVal = BdbToBackendError(retVal, ERROR_BACKEND_ENTRY_NOTFOUND, ERROR_BACKEND_PARENT_NOTFOUND, pBECtx);
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    // Update the DN Index first to detect if entry/DN already exists
    for (nextAttr = pEntry->attrs; nextAttr != NULL; nextAttr = nextAttr->next)
    {
        if (VmDirStringCompareA(nextAttr->type.lberbv.bv_val, ATTR_DN, FALSE) == 0)
        {
            if ((retVal = UpdateIndicesForAttribute( pTxn, &(nextAttr->type), nextAttr->vals, nextAttr->numVals,
                                                     entryId, BDB_INDEX_OP_TYPE_CREATE)) != 0)
            {
                retVal = BdbToBackendError( retVal, DB_KEYEXIST, ERROR_BACKEND_ENTRY_EXISTS, pBECtx);
                BAIL_ON_VMDIR_ERROR( retVal );
            }
            if ((retVal = UpdateAttributeMetaData( pTxn, nextAttr, entryId, BDB_INDEX_OP_TYPE_CREATE )) != 0)
            {
                retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
                BAIL_ON_VMDIR_ERROR( retVal );
            }
        }
    }

    // Update remaining Indices
    for (nextAttr = pEntry->attrs; nextAttr != NULL; nextAttr = nextAttr->next)
    {
        if (VmDirStringCompareA(nextAttr->type.lberbv.bv_val, ATTR_DN, FALSE) != 0)
        {
            if ((retVal = UpdateIndicesForAttribute( pTxn, &(nextAttr->type), nextAttr->vals, nextAttr->numVals,
                                                     entryId, BDB_INDEX_OP_TYPE_CREATE)) != 0)
            {
                retVal = BdbToBackendError( retVal, DB_KEYEXIST, ERROR_BACKEND_CONSTRAINT, pBECtx);
                BAIL_ON_VMDIR_ERROR( retVal );
            }
            if ((retVal = UpdateAttributeMetaData( pTxn, nextAttr, entryId, BDB_INDEX_OP_TYPE_CREATE )) != 0)
            {
                retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
                BAIL_ON_VMDIR_ERROR( retVal );
            }
        }
    }

    // Update primary index (EntryId)
    if ((retVal = CreateEntryIdIndex(pTxn, entryId, &encodedEntry, TRUE /* 1st time new entry creation */)) != 0)
    {
        retVal = BdbToBackendError(retVal, DB_KEYEXIST, ERROR_BACKEND_CONSTRAINT, pBECtx);
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY( encodedEntry.lberbv.bv_val );
    VmDirLog( LDAP_DEBUG_TRACE, "BdbAddEntry: End" );

    return retVal;

error:
    goto cleanup;
}

/* BdbCheckRefIntegrity: Checks for the attributes that have referential integrity constraint set, that the DN attribute
 *                    values refer to existing objects.
 *
 * Returns: BDB error codes.
 *
 */
DWORD
BdbCheckRefIntegrity(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ENTRY              pEntry)
{
    int             retVal = 0;
    VDIR_ATTRIBUTE *     attr = NULL;

    assert( pBECtx != NULL && pBECtx->pBEPrivate != NULL && pEntry != NULL );

    VmDirLog( LDAP_DEBUG_TRACE, "BdbCheckRefIntegrity: Begin, entry DN = %s", pEntry->dn.lberbv.bv_val );

    for (attr = pEntry->attrs; attr; attr = attr->next)
    {
        // SJ-TBD: Instead of checking referential integrity for hard coded attributes, we should have a
        // proprietary flag e.g. X-constraint in the attribute schema definition
        if (VmDirStringCompareA(attr->type.lberbv.bv_val, ATTR_MEMBER, FALSE) == 0)
        {
            int i = 0;
            ENTRYID   eId = 0;
            for (; i < attr->numVals; i++)
            {
                // Lookup in the DN index.
                if ((retVal = VmDirNormalizeDN( &(attr->vals[i]), pEntry->pSchemaCtx)) != 0)
                {
                    retVal = ERROR_BACKEND_OPERATIONS;
                    BAIL_ON_VMDIR_ERROR( retVal );
                }

                if ((retVal = BdbDNToEntryId( pBECtx, &(attr->vals[i]), &eId )) != 0)
                {
                    retVal = BdbToBackendError(retVal, DB_NOTFOUND, ERROR_BACKEND_CONSTRAINT, pBECtx);
                    BAIL_ON_VMDIR_ERROR( retVal );
                }
            }
        }
    }

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "BdbCheckRefIntegrity: End" );
    return retVal;

error:
    goto cleanup;
}

/* BdbDeleteEntry: Deletes an entry in the BDB DBs.
 *
 * Returns: BDB error codes.
 *
 */
DWORD
BdbDeleteEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_MODIFICATION  pMods,
    PVDIR_ENTRY         pEntry
    )
{
    int               retVal = 0;
    DB_TXN*           pTxn = NULL;

    assert( pBECtx != NULL && pBECtx->pBEPrivate != NULL && pEntry != NULL );

    VmDirLog( LDAP_DEBUG_TRACE, "BdbDeleteEntry: Begin, entry DN = %s", pEntry->dn.lberbv.bv_val );

    pTxn = (DB_TXN*)pBECtx->pBEPrivate;

    // Delete child from the parentId index
    if ((retVal = DeleteParentIdIndex( pBECtx, &(pEntry->pdn), pEntry->eId )) != 0)
    {
        retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if ((retVal = BdbModifyEntry( pBECtx, pMods, pEntry)) != 0)
    {
        retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if ((retVal = CreateParentIdIndex( pBECtx, &(gVmdirServerGlobals.delObjsContainerDN), pEntry->eId )) != 0)
    {
        retVal = BdbToBackendError(retVal, ERROR_BACKEND_ENTRY_NOTFOUND, ERROR_BACKEND_PARENT_NOTFOUND, pBECtx);
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "BdbDeleteEntry: End" );
    return retVal;

error:
    goto cleanup;
}


/* BdbDNToEntry: For a given entry DN, reads an entry from the entry DB.
 *
 * Returns: BDB error codes.
 *
 */
DWORD
BdbDNToEntry(
    PVDIR_BACKEND_CTX           pBECtx,
    PVDIR_SCHEMA_CTX            pSchemaCtx,
    VDIR_BERVALUE*              pDn,
    PVDIR_ENTRY                 pEntry,
    VDIR_BACKEND_ENTRY_LOCKTYPE entryLockType)
{
    ENTRYID     eId = {0};
    int         retVal = LDAP_SUCCESS;

    VmDirLog( LDAP_DEBUG_TRACE, "BdbDNToEntry: Begin, DN: %s", pDn->lberbv.bv_val );

    retVal = BdbDNToEntryId( pBECtx, pDn, &eId );
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = BdbEIdToEntry( pBECtx, pSchemaCtx, eId, pEntry, entryLockType );
    BAIL_ON_VMDIR_ERROR( retVal );

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "BdbDNToEntry: End" );
    return retVal;

error:
    goto cleanup;
}

/* BdbEIdToEntry: For a given entry ID, reads an entry from the entry DB.
 *
 * Returns: BDB error codes.
 *
 */
DWORD
BdbEIdToEntry(
    PVDIR_BACKEND_CTX           pBECtx,
    PVDIR_SCHEMA_CTX            pSchemaCtx,
    ENTRYID                     eId,
    PVDIR_ENTRY                      pEntry,
    VDIR_BACKEND_ENTRY_LOCKTYPE entryLockType)
{
    int             retVal = 0;
    DB *            db = NULL;
    DB_TXN*         pTxn = NULL;
    DBT             key = {0};
    DBT             value = {0};
    DBC *           cursor = NULL;
    unsigned char   eIdBytes[sizeof( ENTRYID )];
    unsigned int    cursorGetFlags = (entryLockType == VDIR_BACKEND_ENTRY_LOCK_WRITE ?
                                      (DB_SET | DB_RMW) : DB_SET);

    VmDirLog( LDAP_DEBUG_TRACE, "BdbEIdToEntry: Begin" );

    assert(pBECtx && pSchemaCtx && pEntry);

    // BDB allow implicit txn.  (used in SimpleEIdToEntry case)
    pTxn = (pBECtx->pBEPrivate) ? (DB_TXN*)pBECtx->pBEPrivate : NULL;

    db = gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles[0].pDB;

    // Open cursor
    if ((retVal = db->cursor( db, pTxn, &cursor, BDB_FLAGS_ZERO )) != 0)
    {
        VmDirLog( LDAP_DEBUG_ANY, "BdbEIdToEntry: opening cursor failed with error code: %d, error string: %s", retVal,
                  db_strerror(retVal) );
        retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
        BAIL_ON_VMDIR_ERROR( retVal );;
    }

    // Get size of the encoded entry.

    // Set key
    key.data = &eIdBytes[0];
    EntryIdToDBT(eId, &key);

    memset(&value, 0, sizeof(DBT));
    value.flags = DB_DBT_USERMEM;
    value.ulen = 0;

    if ((retVal = cursor->get( cursor, &key, &value, cursorGetFlags )) != 0)
    {
        if (retVal == DB_NOTFOUND)
        {
            retVal = BdbToBackendError(retVal, DB_NOTFOUND, ERROR_BACKEND_ENTRY_NOTFOUND, pBECtx);
            BAIL_ON_VMDIR_ERROR( retVal );;
        }
        if (retVal != DB_BUFFER_SMALL)
        {
            VmDirLog( LDAP_DEBUG_ANY, "BdbEIdToEntry: get (DB_SET) cursor failed with error code: %d, error string: %s",
                      retVal, db_strerror(retVal) );
            retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
            BAIL_ON_VMDIR_ERROR( retVal );;
        }
    }
    else
    {
        VmDirLog( LDAP_DEBUG_ANY, "BdbEIdToEntry: get (DB_SET) cursor should have failed with DB_NOTFOUND or "
                  "DB_BUFFER_SMALL error, but it did not." );
        // SJ-TBD: DB_BUFFER_SMAL, This is a made up error for this situation.
        retVal = BdbToBackendError(DB_BUFFER_SMALL, 0, ERROR_BACKEND_ERROR, pBECtx);
        BAIL_ON_VMDIR_ERROR( retVal );;
    }

    // Now, get encoded entry.

    if (VmDirAllocateMemory( value.size, (PVOID *)&value.data ) != 0)
    {
        VmDirLog( LDAP_DEBUG_ANY, "BdbEIdToEntry: VmDirAllocateMemory failed" );
        retVal = ERROR_BACKEND_OPERATIONS;
        BAIL_ON_VMDIR_ERROR( retVal );
    }
    value.ulen = value.size;

    // It Seems cursor has to be reset (DB_SET), instead of using DB_CURRENT.
    // Previous cursor call does not really set the cursor (because we are just checking for size?).
    if ((retVal = cursor->get( cursor, &key, &value, cursorGetFlags )) != 0)
    {
        VmDirLog( LDAP_DEBUG_ANY, "BdbEIdToEntry: get (DB_SET) cursor failed with error code: %d, error string: %s",
                  retVal, db_strerror(retVal) );
        retVal = BdbToBackendError(retVal, DB_NOTFOUND, ERROR_BACKEND_ENTRY_NOTFOUND, pBECtx);
        BAIL_ON_VMDIR_ERROR( retVal );;
    }

    pEntry->encodedEntry = value.data;
    VmDirDecodeEntry(pSchemaCtx, pEntry );
    pEntry->eId = eId;
    retVal = 0;

cleanup:
    if (cursor != NULL)
    {
        cursor->c_close( cursor );
    }
    VmDirLog( LDAP_DEBUG_TRACE, "BdbEIdToEntry: End" );
    return retVal;

error:
    VmDirFreeEntryContent( pEntry );
    goto cleanup;
}

/* BdbModifyEntry: Updates an entry in the BDB DBs.
 *
 * Returns: BDB error codes.
 *
 */
DWORD
BdbModifyEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_MODIFICATION*  pMods,
    PVDIR_ENTRY         pEntry
    )
{
    int             retVal = 0;
    VDIR_BERVALUE        newEncodedEntry = VDIR_BERVALUE_INIT;
    VDIR_MODIFICATION *  mod = NULL;
    DB_TXN*         pTxn = NULL;

    assert( pBECtx && pBECtx->pBEPrivate && pMods != NULL && pEntry != NULL );

    VmDirLog( LDAP_DEBUG_TRACE, "BdbModifyEntry: Begin, entry DN = %s", pEntry->dn.lberbv.bv_val );

    pTxn = (DB_TXN*)pBECtx->pBEPrivate;
    VmDirEncodeEntry( pEntry, &newEncodedEntry );
    VMDIR_SAFE_FREE_MEMORY( pEntry->encodedEntry );
    pEntry->encodedEntry = (unsigned char *)newEncodedEntry.lberbv.bv_val; // entry takes over the responsibility to free
                                                                   // newEncodedEntry.lberbv.bv_val

    // Create/Delete appropriate indices for indexed attributes.

    for (mod = pMods; mod != NULL; mod = mod->next)
    {
        if (mod->ignore)
        {
            continue;
        }
        switch (mod->operation)
        {
            case MOD_OP_ADD:
                if ((retVal = UpdateIndicesForAttribute( pTxn, &(mod->attr.type), mod->attr.vals, mod->attr.numVals,
                                                         pEntry->eId, BDB_INDEX_OP_TYPE_CREATE)) != 0)
                {
                    retVal = BdbToBackendError( retVal, DB_KEYEXIST, ERROR_BACKEND_CONSTRAINT, pBECtx);
                    BAIL_ON_VMDIR_ERROR( retVal );
                }
                break;

            case MOD_OP_DELETE:
                if ((retVal = UpdateIndicesForAttribute( pTxn, &(mod->attr.type), mod->attr.vals, mod->attr.numVals,
                                                         pEntry->eId, BDB_INDEX_OP_TYPE_DELETE )) != 0)
                {
                    retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
                    BAIL_ON_VMDIR_ERROR( retVal );
                }
                break;

            case MOD_OP_REPLACE:
            default:
                assert( FALSE );
        }
        if ((retVal = UpdateAttributeMetaData( pTxn, &(mod->attr), pEntry->eId, BDB_INDEX_OP_TYPE_UPDATE )) != 0)
        {
            retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
            BAIL_ON_VMDIR_ERROR( retVal );
        }
    }

    // Update Entry DB.

    if ((retVal = CreateEntryIdIndex(pTxn, pEntry->eId, &newEncodedEntry, FALSE /* update current eId key */)) != 0)
    {
        retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
     VmDirLog( LDAP_DEBUG_TRACE, "BdbModifyEntry: End" );
     return retVal;

error:
     goto cleanup;
}

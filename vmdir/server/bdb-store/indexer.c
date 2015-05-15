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

#define CANDIDATES_START_ALLOC_SIZE 10

static int
CompareEntryIds(
    const void * eId1,
    const void * eId2);

static void
DBTToEntryId(
    DBT *      dbt,
    db_seq_t * eId);

static int
DeleteKeyValue(
    DB *            db,
    DB_TXN *        pTxn,
    DBT *           key,
    DBT *           value,
    BOOLEAN         uniqueVal);

static int
ScanIndex(
    DB_TXN *        pTxn,
    VDIR_BERVALUE * attrType,
    DBT *           key,
    VDIR_FILTER *   f,
    const char **   errText);

static int
UpdateKeyValue(
    DB *            db,
    DB_TXN *        txn,
    DBT *           key,
    DBT *           value,
    BOOLEAN         uniqueVal,
    ULONG           ulOPMask);

/*
 * BdbCheckIfALeafNode(): From parentid.db, check if the entryId has children.
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB, e.g. DB_NOTFOUND if entryId does not exist etc.
 */
DWORD
BdbCheckIfALeafNode(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID             entryId,
    PBOOLEAN            pIsLeafEntry)
{
    int                      retVal = 0;
    DBT                      key = {0};
    DBT                      value = {0};
    DB *                     db = NULL;
    VDIR_BERVALUE            parentIdAttr = { {ATTR_PARENT_ID_LEN, ATTR_PARENT_ID}, 0, 0, NULL };
    unsigned char            eIdBytes[sizeof( ENTRYID )];
    unsigned char            parentEIdBytes[sizeof( ENTRYID )];
    DB_TXN*                  pTxn = NULL;

    PVDIR_CFG_ATTR_INDEX_DESC   pIdxDesc = NULL;
    USHORT                      usVersion = 0;

    assert(pBECtx && pBECtx->pBEPrivate && pIsLeafEntry);
    VmDirLog( LDAP_DEBUG_TRACE, "BdbCheckIfALeafNode: Begin, entryId = %lld", entryId );

    pTxn = (DB_TXN*)pBECtx->pBEPrivate;

    *pIsLeafEntry = FALSE;

    pIdxDesc = VmDirAttrNameToWriteIndexDesc(parentIdAttr.lberbv.bv_val, usVersion, &usVersion);
    assert(pIdxDesc);

    db = gVdirBdbGlobals.bdbIndexDBs.pIndexDBs[pIdxDesc->iId].pBdbDataFiles[0].pDB;

    key.data = &parentEIdBytes[0];
    EntryIdToDBT(entryId, &key);

    memset(&value, 0, sizeof(DBT));
    value.flags = DB_DBT_USERMEM;
    value.data = &(eIdBytes[0]);
    value.ulen = sizeof( ENTRYID );

    if ((retVal = db->get( db, pTxn, &key, &value, BDB_FLAGS_ZERO )) == 0)
    {
        *pIsLeafEntry = FALSE;
    }
    else if (retVal == DB_NOTFOUND)
    {
        *pIsLeafEntry = TRUE;
        retVal = 0;
    }
    else
    {
        VmDirLog( LDAP_DEBUG_ANY, "BdbCheckIfALeafNode: db->get() failed with error code: %d, error string: %s", retVal,
                  db_strerror(retVal) );
        retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
    }

    VmDirLog( LDAP_DEBUG_TRACE, "BdbCheckIfALeafNode: End, retVal = %d", retVal );
    return retVal;
}

/*
 * BdbGetAttrMetaData(): Get attribute's meta data.
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB
 */
DWORD
BdbGetAttrMetaData(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_ATTRIBUTE *    attr,
    ENTRYID             entryId
    )
{
    DWORD                 retVal = 0;
    DBT                   key = {0};
    DBT                   value = {0};
    char *                keyData[ sizeof( db_seq_t ) + 1 + 2 ]; /* key format is: <entry ID>:<attribute ID (a short)> */
    DB *                  db = NULL;
    int                   indTypes = 0;
    BOOLEAN               uniqueVal = FALSE;
    VDIR_BERVALUE         attrMetaDataAttr = { {ATTR_ATTR_META_DATA_LEN, ATTR_ATTR_META_DATA}, 0, 0, NULL };
    unsigned char *       writer = NULL;

    PVDIR_CFG_ATTR_INDEX_DESC   pIdxDesc = NULL;
    USHORT                      usVersion = 0;
    DB_TXN*                     pTxn = NULL;

    VmDirLog( LDAP_DEBUG_TRACE, "BdbGetAttrMetaData: Begin, attrType: %s", attr->type.lberbv.bv_val );

    assert( pBECtx && pBECtx->pBEPrivate );
    pTxn = (DB_TXN*)pBECtx->pBEPrivate;

    pIdxDesc = VmDirAttrNameToWriteIndexDesc(attrMetaDataAttr.lberbv.bv_val, usVersion, &usVersion);
    assert(pIdxDesc);

    db = gVdirBdbGlobals.bdbIndexDBs.pIndexDBs[pIdxDesc->iId].pBdbDataFiles[0].pDB;
    indTypes = pIdxDesc->iTypes;
    assert( indTypes == INDEX_TYPE_EQUALITY );
    uniqueVal = pIdxDesc->bIsUnique;
    assert( uniqueVal );

    key.data = keyData;
    EntryIdToDBT( entryId, &key );
    *(unsigned char *)((unsigned char *)key.data + key.size) = ':';
    key.size++;
    writer = ((unsigned char *)key.data + key.size);
    VmDirEncodeShort( &writer, attr->pATDesc->usAttrID );
    key.size += 2;

    memset(&value, 0, sizeof(DBT));
    value.flags = DB_DBT_USERMEM;
    value.data = attr->metaData;
    value.ulen = VMDIR_MAX_ATTR_META_DATA_LEN;

    retVal = db->get( db, pTxn, &key, &value, BDB_FLAGS_ZERO );
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "BdbGetAttrMetaData: End, retVal = %d", retVal );
    return retVal;

error:
    if (retVal != DB_NOTFOUND)
    {
        VmDirLog( LDAP_DEBUG_ANY, "BdbGetAttrMetaData failed with error code: %d, error string: %s", retVal,
                  db_strerror(retVal) );
    }
    retVal = BdbToBackendError(retVal, DB_NOTFOUND, ERROR_BACKEND_ATTR_META_DATA_NOTFOUND, pBECtx);
    goto cleanup;
}

#define START_ATTR_META_DATA_NODE_ALLOC_SIZE    100
#define INC_ATTR_META_DATA_NODE_ALLOC_SIZE      50

/*
 * BdbGetAllAttrsMetaData(): Get attribute's meta data for all the attributes of an entry.
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB
 */
DWORD
BdbGetAllAttrsMetaData(
    PVDIR_BACKEND_CTX           pBECtx,
    ENTRYID                     entryId,
    PATTRIBUTE_META_DATA_NODE * ppAttrMetaDataNode,
    int *                       pNumAttrMetaData
    )
{
    DWORD                 retVal = 0;
    DBT                   key = {0};
    DBT                   value = {0};
    char *                keyData[ sizeof( db_seq_t ) + 1 + 2 ]; /* key format is: <entry ID>:<attribute ID (a short)> */
    DB *                  db = NULL;
    int                   indTypes = 0;
    BOOLEAN               uniqueVal = FALSE;
    VDIR_BERVALUE         attrMetaDataAttr = { {ATTR_ATTR_META_DATA_LEN, ATTR_ATTR_META_DATA}, 0, 0, NULL };
    int                   currMaxAttrMetaDataNodeAllocSize = 0;
    DB_TXN *              localTxn = NULL;
    DBC *                 cursor = NULL;
    unsigned int          cursorFlags;
    DBT                   currKey = {0};
    unsigned char *       reader = NULL;

    PVDIR_CFG_ATTR_INDEX_DESC   pIdxDesc = NULL;
    USHORT                      usVersion = 0;
    DB_TXN*                     txn = NULL;
    PATTRIBUTE_META_DATA_NODE   pNodeBuf = NULL;
    int                         iNodeCnt = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "BdbGetAllAttrsMetaData: Begin, entryId: %lld", entryId );

    assert( pBECtx && pBECtx->pBEPrivate );
    txn = (DB_TXN*)pBECtx->pBEPrivate;

    pIdxDesc = VmDirAttrNameToWriteIndexDesc(attrMetaDataAttr.lberbv.bv_val, usVersion, &usVersion);
    assert(pIdxDesc);

    db = gVdirBdbGlobals.bdbIndexDBs.pIndexDBs[pIdxDesc->iId].pBdbDataFiles[0].pDB;
    indTypes = pIdxDesc->iTypes;
    assert( indTypes == INDEX_TYPE_EQUALITY );
    uniqueVal = pIdxDesc->bIsUnique;
    assert( uniqueVal );

    key.data = keyData;
    EntryIdToDBT( entryId, &key );
    *(unsigned char *)((unsigned char *)key.data + key.size) = ':';
    key.size++;

    currMaxAttrMetaDataNodeAllocSize = START_ATTR_META_DATA_NODE_ALLOC_SIZE;
    if (VmDirAllocateMemory( currMaxAttrMetaDataNodeAllocSize * sizeof( ATTRIBUTE_META_DATA_NODE ),
                             (PVOID *)&pNodeBuf ) != 0)
    {
        retVal = ERROR_BACKEND_OPERATIONS; // SJ-TBD: In this function, this error code should really be in DB_ error space
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    /* If no bigger transaction going on/passed-in */
    if (txn == NULL)
    {
        /* Transaction, "required" (because it spans multiple calls) for cursor operations. */
        retVal = gVdirBdbGlobals.bdbEnv->txn_begin (gVdirBdbGlobals.bdbEnv, BDB_PARENT_TXN_NULL, &localTxn,
                                                    DB_READ_COMMITTED);
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    else
    {
        localTxn = txn;
    }

    retVal = db->cursor( db, localTxn, &cursor, BDB_FLAGS_ZERO );
    BAIL_ON_VMDIR_ERROR(retVal);

    memset(&currKey, 0, sizeof(DBT));
    currKey.flags = DB_DBT_USERMEM | DB_DBT_PARTIAL;
    currKey.ulen = currKey.dlen = currKey.size = key.size;
    currKey.ulen = currKey.dlen = 4 + 1 + sizeof (short); // SJ-TBD: To be explained.
    if (VmDirAllocateMemory( currKey.ulen, (PVOID *)&currKey.data ) != 0)
    {
        retVal = ERROR_BACKEND_OPERATIONS; // SJ-TBD: In this function, this error code should really be in DB_ error space
        BAIL_ON_VMDIR_ERROR( retVal );
    }
    memcpy( currKey.data, key.data, key.size);
    currKey.doff = 0;

    memset(&value, 0, sizeof(DBT));
    value.flags = DB_DBT_USERMEM;
    value.ulen = VMDIR_MAX_ATTR_META_DATA_LEN;

    cursorFlags = DB_SET_RANGE;
    do
    {
        currKey.size = key.size; // Reset to the original size we are looking for, cursor->get would have changed it
        value.data = pNodeBuf[iNodeCnt].metaData;

        if ((retVal = cursor->get( cursor, &currKey, &value, cursorFlags )) != 0)
        {
            if (retVal == DB_NOTFOUND)
            {
                retVal = 0;
            }
            else
            {
                BAIL_ON_VMDIR_ERROR( retVal );
            }
            break;
        }
        if (memcmp(key.data, currKey.data, key.size) != 0)
        {
            break;
        }
        reader = &((unsigned char *)currKey.data)[key.size];
        pNodeBuf[iNodeCnt].attrID = VmDirDecodeShort( &reader );
        iNodeCnt++;
        if (iNodeCnt == currMaxAttrMetaDataNodeAllocSize)
        {
            currMaxAttrMetaDataNodeAllocSize += INC_ATTR_META_DATA_NODE_ALLOC_SIZE;
            if (VmDirReallocateMemory( (PVOID)pNodeBuf, (PVOID *)&pNodeBuf,
                                       currMaxAttrMetaDataNodeAllocSize * sizeof( ATTRIBUTE_META_DATA_NODE )) != 0)
            {
                retVal = ERROR_BACKEND_OPERATIONS; // SJ-TBD: In this function, this error code should really be in DB_ error space
                BAIL_ON_VMDIR_ERROR( retVal );
            }
        }
        cursorFlags = DB_NEXT;
    }
    while (TRUE);

    *ppAttrMetaDataNode = pNodeBuf;
    *pNumAttrMetaData = iNodeCnt;

cleanup:
    VmDirFreeMemory( currKey.data );
    if (cursor != NULL)
    {
        cursor->c_close( cursor );
    }
    if (txn == NULL && localTxn != NULL) /* commit/abort local transaction */
    {
        if (retVal == 0 || retVal == DB_NOTFOUND)
        {
            retVal = localTxn->commit (localTxn, BDB_FLAGS_ZERO);
        }
        else
        {
            localTxn->abort(localTxn);
        }
    }
    VmDirLog( LDAP_DEBUG_TRACE, "BdbGetAllAttrsMetaData: End, retVal = %d, #of numAttrMetaData = %d",
              retVal, *pNumAttrMetaData );
    return retVal;

error:
    VMDIR_SAFE_FREE_MEMORY( pNodeBuf );
    VmDirLog( LDAP_DEBUG_ANY, "BdbGetAllAttrsMetaData failed with error code: %d, error string: %s", retVal,
              db_strerror(retVal) );
    retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
    goto cleanup;
}

/* BdbGetCandidates: Get candidates for individual filter components where filter attribute is indexed.
 *
 */
DWORD
BdbGetCandidates(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_FILTER*        pFilter)
{
    PCSTR       pszStaticErrText = NULL;    // points to static string
    int         retVal = 0;
    DBT         key = {0};
    char *      keyData = NULL;
    db_seq_t    parentId;
    DB_TXN*     pTxn = NULL;

    assert (pBECtx && pBECtx->pBEPrivate && pFilter);

    VmDirLog( LDAP_DEBUG_TRACE, "BdbGetCandidates: Begin" );

    pTxn = (DB_TXN*)pBECtx->pBEPrivate;

    switch ( pFilter->choice )
    {
        case LDAP_FILTER_EQUALITY:
        case LDAP_FILTER_GE:
        {
            char *    normVal = BERVAL_NORM_VAL(pFilter->filtComp.ava.value);
            ber_len_t normValLen = BERVAL_NORM_LEN(pFilter->filtComp.ava.value);

            VmDirLog( LDAP_DEBUG_FILTER, (pFilter->choice == LDAP_FILTER_EQUALITY) ? "LDAP_FILTER_EQUALITY" :
                                                                                     "LDAP_FILTER_GE" );

            memset(&key, 0, sizeof(DBT));
            retVal = VmDirAllocateMemory( normValLen + 1, (PVOID *)&keyData );
            BAIL_ON_VMDIR_ERROR(retVal); // ERROR_NO_MEMORY error will get mapped to ERROR_BACKEND_ERROR in BdbToBackendError below.
            key.data = keyData;
            *(char *)(key.data) = INDEX_KEY_TYPE_FWD;
            memcpy(((char *)key.data + 1), normVal, normValLen);
            key.size = normValLen + 1;

            retVal = ScanIndex( pTxn, &(pFilter->filtComp.ava.type), &key, pFilter, &pszStaticErrText );
            BAIL_ON_VMDIR_ERROR(retVal);
            break;
        }
        case LDAP_FILTER_SUBSTRINGS:
        {
            int       j = 0;
            int       k = 0;
            char *    normVal = NULL;
            ber_len_t normValLen = 0;

            VmDirLog( LDAP_DEBUG_FILTER, "LDAP_FILTER_SUBSTRINGS" );

            // SJ-TBD: It can be both and INITIAL and FINAL instead of one or the other.
            if (pFilter->filtComp.subStrings.initial.lberbv.bv_len != 0)
            {
                normVal = BERVAL_NORM_VAL(pFilter->filtComp.subStrings.initial);
                normValLen = BERVAL_NORM_LEN(pFilter->filtComp.subStrings.initial);

                memset(&key, 0, sizeof(DBT));
                retVal = VmDirAllocateMemory( normValLen + 1, (PVOID *)&keyData );
                BAIL_ON_VMDIR_ERROR(retVal); // ERROR_NO_MEMORY error will get mapped to ERROR_BACKEND_ERROR in BdbToBackendError below.
                key.data = keyData;
                *(char *)(key.data) = INDEX_KEY_TYPE_FWD;
                memcpy(((char *)key.data + 1), normVal, normValLen);
                key.size = normValLen + 1;
            }
            else if (pFilter->filtComp.subStrings.final.lberbv.bv_len != 0)
            {
                normVal = BERVAL_NORM_VAL(pFilter->filtComp.subStrings.final);
                normValLen = BERVAL_NORM_LEN(pFilter->filtComp.subStrings.final);

                memset(&key, 0, sizeof(DBT));
                retVal = VmDirAllocateMemory( normValLen + 1, (PVOID *)&keyData );
                BAIL_ON_VMDIR_ERROR(retVal); // ERROR_NO_MEMORY error will get mapped to ERROR_BACKEND_ERROR in BdbToBackendError below.
                key.data = keyData;
                *(char *)(key.data) = INDEX_KEY_TYPE_REV;
                // Reverse copy from f->filtComp.subStrings.final.lberbv.bv_val to &(key.data[1])
                for (j=normValLen - 1, k=1; j >= 0; j--, k++)
                {
                    *((char *)key.data + k) = normVal[j];
                }
                key.size = normValLen + 1;
            }
            else
            {
                assert( FALSE );
            }

            retVal = ScanIndex( pTxn, &(pFilter->filtComp.subStrings.type), &key, pFilter, &pszStaticErrText );
            BAIL_ON_VMDIR_ERROR(retVal);
            break;
        }

        case FILTER_ONE_LEVEL_SEARCH:
        {
            VDIR_BERVALUE       parentIdAttr = { {ATTR_PARENT_ID_LEN, ATTR_PARENT_ID }, 0, 0, NULL };
            unsigned char  parentEIdBytes[sizeof( db_seq_t )];

            VmDirLog( LDAP_DEBUG_FILTER, "LDAP_FILTER_ONE_LEVEL_SRCH" );

            retVal = BdbDNToEntryId( pBECtx, &(pFilter->filtComp.parentDn), &parentId );
            BAIL_ON_VMDIR_ERROR(retVal);

            key.data = &parentEIdBytes[0];
            EntryIdToDBT(parentId, &key);

            retVal = ScanIndex( pTxn, &(parentIdAttr), &key, pFilter, &pszStaticErrText );
            BAIL_ON_VMDIR_ERROR(retVal);
            break;
        }

        default:
            assert( FALSE );
            break;
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY( keyData );
    if (retVal != 0 && pszStaticErrText != NULL)
    {
        VMDIR_SAFE_FREE_MEMORY(pBECtx->pszBEErrorMsg);
        VmDirAllocateStringA(pszStaticErrText, &pBECtx->pszBEErrorMsg);
    }

    VmDirLog( LDAP_DEBUG_TRACE, "BdbGetCandidates: End" );
    return retVal;

error:
    retVal = BdbToBackendError(retVal, DB_NOTFOUND, ERROR_BACKEND_ENTRY_NOTFOUND, pBECtx);
    goto cleanup;
}

/*
 * CreateParentIdIndex(): In parentid.db, create parentId => entryId index. => mainly used in one-level searches.
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB, e.g. DB_NOTFOUND if parentDN does not exist in the DN DB.
 */
int
CreateParentIdIndex(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE *          pdn,
    db_seq_t            entryId)
{
    int                      retVal = 0;
    DBT                      key = {0};
    DBT                      value = {0};
    DB *                     db = NULL;
    BOOLEAN                  uniqueVal = FALSE;
    VDIR_BERVALUE            parentIdAttr = { {ATTR_PARENT_ID_LEN, ATTR_PARENT_ID}, 0, 0, NULL };
    db_seq_t                 parentId = 0;
    unsigned char            eIdBytes[sizeof( db_seq_t )];
    unsigned char            parentEIdBytes[sizeof( db_seq_t )];

    PVDIR_CFG_ATTR_INDEX_DESC   pIdxDesc = NULL;
    USHORT                      usVersion = 0;

    assert(pBECtx && pBECtx->pBEPrivate && pdn);
    VmDirLog( LDAP_DEBUG_TRACE, "CreateParentIdIndex: Begin, pdn = %s, entryId = %lld", pdn->lberbv.bv_val, entryId );

    retVal = BdbDNToEntryId( pBECtx, pdn, &parentId );
    BAIL_ON_VMDIR_ERROR( retVal );

    // Update parentId => entryId index in parentid.db.

    pIdxDesc = VmDirAttrNameToWriteIndexDesc(parentIdAttr.lberbv.bv_val, usVersion, &usVersion);
    assert(pIdxDesc);

    db = gVdirBdbGlobals.bdbIndexDBs.pIndexDBs[pIdxDesc->iId].pBdbDataFiles[0].pDB;
    uniqueVal = pIdxDesc->bIsUnique;
    assert( uniqueVal == FALSE );

    key.data = &parentEIdBytes[0];
    EntryIdToDBT(parentId, &key);

    value.data = &eIdBytes[0];
    EntryIdToDBT(entryId, &value);

    if ((retVal = db->put (db, (DB_TXN*)pBECtx->pBEPrivate, &key, &value, uniqueVal ? DB_NOOVERWRITE : BDB_FLAGS_ZERO)) != 0)
    {
        VmDirLog( LDAP_DEBUG_ANY, "CreateParentIdIndex: For entryId: %lld, db->put() failed with error code: %d, error "
                  "string: %s", entryId, retVal, db_strerror(retVal) );

        retVal = BdbToBackendError(retVal, 0, ERROR_BACKEND_ERROR, pBECtx);
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "CreateParentIdIndex: End, retVal = %d", retVal );
    return retVal;

error:
    goto cleanup;
}

/*
 * CreateEntryIdIndex(): In entry DB, create entryId => encodedEntry entry.
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB.
 */
int
CreateEntryIdIndex(
    DB_TXN *         txn,
    db_seq_t         eId,
    VDIR_BERVALUE *  encodedEntry,
    BOOLEAN          new
    ) // Creating a new EntryId => encodedEntry map or updating an existing one.
{
    int                      retVal = 0;
    DBT                      key = {0};
    DBT                      value = {0};
    DB *                     db = NULL;
    BOOLEAN                  uniqueVal = FALSE;
    unsigned char            eIdBytes[sizeof( db_seq_t )] = {0};

    VmDirLog( LDAP_DEBUG_TRACE, "CreateEntryIdIndex: Begin, entryId = %lld", eId );

    db = gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles[0].pDB;
    uniqueVal = gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles[0].bIsUnique;
    assert( uniqueVal );

    key.data = &eIdBytes[0];
    EntryIdToDBT(eId, &key);

    value.data = encodedEntry->lberbv.bv_val;
    value.size = encodedEntry->lberbv.bv_len;

    retVal = db->put( db,
                      txn,
                      &key,
                      &value,
                      new ? DB_NOOVERWRITE : BDB_FLAGS_ZERO );
    BAIL_ON_VMDIR_ERROR( retVal );

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "CreateEntryIdIndex: End, retVal = %d", retVal );
    return retVal;

error:
    VmDirLog( LDAP_DEBUG_ANY, "CreateEntryIdIndex failed for entryId: %lld, error code: %d, error string: %s",
              eId, retVal, db_strerror(retVal) );
    goto cleanup;
}

/*
 * DeleteEntryIdIndex(): In entry DB, delete entryId => encodedEntry entry.
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB.
 */
int
DeleteEntryIdIndex(
    DB_TXN *    txn,
    db_seq_t    entryId)
{
    int                      retVal = 0;
    DBT                      key = {0};
    DB *                     db = NULL;
    BOOLEAN                  uniqueVal = FALSE;
    unsigned char            eIdBytes[sizeof( db_seq_t )];

    VmDirLog( LDAP_DEBUG_TRACE, "DeleteEntryIdIndex: Begin, entryId = %lld", entryId );

    db = gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles[0].pDB;
    uniqueVal = gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles[0].bIsUnique;
    assert( uniqueVal );

    key.data = &eIdBytes[0];
    EntryIdToDBT(entryId, &key);

    retVal = DeleteKeyValue(db, txn, &key, NULL /* not used for unique keys */, uniqueVal);

    VmDirLog( LDAP_DEBUG_TRACE, "DeleteEntryIdIndex: End, retVal = %d", retVal );
    return retVal;
}


/*
 * DeleteParentIdIndex(): In parentid.db, delete parentId => entryId index.
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB, e.g. DB_NOTFOUND if parentDN does not exist in the DN DB.
 */
int
DeleteParentIdIndex(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE *     pdn,
    db_seq_t            entryId)
{
    int                      retVal = 0;
    DBT                      key = {0};
    DBT                      value = {0};
    DB *                     db = NULL;
    BOOLEAN                  uniqueVal = FALSE;
    VDIR_BERVALUE            parentIdAttr = { {ATTR_PARENT_ID_LEN, ATTR_PARENT_ID}, 0, 0, NULL };
    db_seq_t                 parentId = 0;
    unsigned char            parentEIdBytes[sizeof( db_seq_t )];
    unsigned char            entryIdBytes[sizeof( db_seq_t )];

    PVDIR_CFG_ATTR_INDEX_DESC   pIdxDesc = NULL;
    USHORT                      usVersion = 0;

    assert(pBECtx && pBECtx->pBEPrivate && pdn);

    VmDirLog( LDAP_DEBUG_TRACE, "DeleteParentIdIndex: Begin, pdn = %s, entryId = %lld", pdn->lberbv.bv_val, entryId );

    retVal = BdbDNToEntryId( pBECtx, pdn, &parentId );
    BAIL_ON_VMDIR_ERROR( retVal );

    pIdxDesc = VmDirAttrNameToWriteIndexDesc(parentIdAttr.lberbv.bv_val, usVersion, &usVersion);
    assert(pIdxDesc);

    db = gVdirBdbGlobals.bdbIndexDBs.pIndexDBs[pIdxDesc->iId].pBdbDataFiles[0].pDB;
    uniqueVal = pIdxDesc->bIsUnique;
    assert( uniqueVal == FALSE );

    key.data = &parentEIdBytes[0];
    EntryIdToDBT(parentId, &key);

    value.data = &entryIdBytes[0];
    EntryIdToDBT(entryId, &value);

    retVal = DeleteKeyValue(db, (DB_TXN*)pBECtx->pBEPrivate, &key, &value, uniqueVal);
    BAIL_ON_VMDIR_ERROR( retVal );

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "DeleteParentIdIndex: End, retVal = %d", retVal );
    return retVal;

error:
    goto cleanup;
}


/*
 * BdbDNToEntryId(). Given a DN, get the entryId from DN DB.
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB e.g. DB_NOTFOUND if DN does not exist in the DN DB.
 */
DWORD
BdbDNToEntryId(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE*      pDn,
    ENTRYID*            pEId)
{
    int                   retVal = 0;
    VDIR_BERVALUE         dnAttr = { {ATTR_DN_LEN, ATTR_DN}, 0, 0, NULL};
    DBT                   key = {0};
    char *                keyData = NULL;
    DBT                   value = {0};
    DB *                  db = NULL;
    unsigned char         eIdBytes[sizeof( ENTRYID )];
    char *                normDn = BERVAL_NORM_VAL(*pDn);
    ber_len_t             normDnLen = BERVAL_NORM_LEN(*pDn);
    DB_TXN*               pTxn = NULL;

    PVDIR_CFG_ATTR_INDEX_DESC   pIdxDesc = NULL;
    USHORT                      usVersion = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "BdbDNToEntryId: Begin, dn = %s", pDn->lberbv.bv_val );

    assert(pBECtx && pDn && pEId);

    // BDB allow implicit txn.  (used in SimpleDnToEntry case)
    pTxn = (pBECtx->pBEPrivate) ? (DB_TXN*)pBECtx->pBEPrivate : NULL;

    if (normDnLen == 0)
    {
        *pEId = DSE_ROOT_ENTRY_ID;
    }
    else
    {
        pIdxDesc = VmDirAttrNameToWriteIndexDesc(dnAttr.lberbv.bv_val, usVersion, &usVersion);
        assert( pIdxDesc );

        db = gVdirBdbGlobals.bdbIndexDBs.pIndexDBs[pIdxDesc->iId].pBdbDataFiles[0].pDB;

        memset(&key, 0, sizeof(DBT));
        if (VmDirAllocateMemory( normDnLen + 1, (PVOID *)&keyData ) != 0)
        {
            VmDirLog( LDAP_DEBUG_ANY, "BdbDNToEntryId: VmDirAllocateMemory failed" );
            retVal = ERROR_BACKEND_OPERATIONS;
            BAIL_ON_VMDIR_ERROR( retVal );
        }
        key.data = keyData;
        *(char *)(key.data) = INDEX_KEY_TYPE_FWD;
        memcpy(((char *)key.data + 1), normDn, normDnLen);
        key.size = normDnLen + 1;

        memset(&value, 0, sizeof(DBT));
        value.flags = DB_DBT_USERMEM;
        value.ulen = sizeof(db_seq_t);
        value.data = &(eIdBytes[0]);

        if ((retVal = db->get( db, pTxn, &key, &value, BDB_FLAGS_ZERO )) != 0)
        {
            VmDirLog( LDAP_DEBUG_ANY, "BdbDNToEntryId: failed for Dn: %s, with error code: %d, error string: %s",
                      pDn->lberbv.bv_val, retVal, db_strerror(retVal) );
            retVal = BdbToBackendError(retVal, DB_NOTFOUND, ERROR_BACKEND_ENTRY_NOTFOUND, pBECtx);
            BAIL_ON_VMDIR_ERROR( retVal );
        }
        DBTToEntryId( &value, pEId);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY( keyData );
    VmDirLog( LDAP_DEBUG_TRACE, "BdbDNToEntryId: End, retVal = %d, entry ID = %lld", retVal, *pEId );
    return retVal;

error:
    goto cleanup;
}

/* EntryIdToDBT: Convert EntryId (db_seq_t/long long) to sequence of bytes (going from high order bytes to lower
 * order bytes) to be stored in BDB.
 *
 * Motivation: So that BDB can store EntryId data values in a sorted way (with DB_DUPSORT flag set for the DB) using
 * its default sorting scheme ("If no comparison function is specified, the data items are compared lexically, with
 * shorter data items collating before longer data items.")
 */
void
EntryIdToDBT(
    db_seq_t  eId,
    DBT *     dbt)
{
    db_seq_t tmpEId = eId;
    int      i = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "EntryIdToDBT: Begin, EntryId = %lld", eId );

    dbt->size = BE_REAL_EID_SIZE(eId);

    for (i = dbt->size - 1, tmpEId = eId; i >= 0; i-- )
    {
        ((unsigned char *)dbt->data)[i] = (unsigned char) tmpEId;
        tmpEId >>= 8;
    }

    VmDirLog( LDAP_DEBUG_TRACE, "EntryIdToDBT: End" );
}

/*
 * UpdateAttributeMetaData(): Update attribute's meta data.
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB
 */
int
UpdateAttributeMetaData(
    DB_TXN *    txn,
    VDIR_ATTRIBUTE * attr,
    db_seq_t    entryId,
    ULONG       ulOPMask)
{
    int                   retVal = 0;
    DBT                   key = {0};
    DBT                   value = {0};
    char *                keyData[ sizeof( db_seq_t ) + 1 + 2 ] = {0}; /* key format is: <entry ID>:<attribute ID (a short)> */
    DB *                  db = NULL;
    int                   indTypes = 0;
    BOOLEAN               uniqueVal = FALSE;
    VDIR_BERVALUE         attrMetaDataAttr = { {ATTR_ATTR_META_DATA_LEN, ATTR_ATTR_META_DATA}, 0, 0, NULL };
    unsigned char *       writer = NULL;

    PVDIR_CFG_ATTR_INDEX_DESC   pIdxDesc = NULL;
    USHORT                      usVersion = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "UpdateAttributeMetaData: Begin, attrType: %s", attr->type.lberbv.bv_val );

    // E.g. while deleting a user, and therefore updating the member attribute of the groups to which the user belongs,
    // member attrMetaData of the group object is left unchanged (at least in the current design, SJ-TBD).
    if (ulOPMask == BDB_INDEX_OP_TYPE_UPDATE && VmDirStringLenA( attr->metaData ) == 0)
    {
        goto error;
    }

    pIdxDesc = VmDirAttrNameToWriteIndexDesc(attrMetaDataAttr.lberbv.bv_val, usVersion, &usVersion);
    assert(pIdxDesc);

    db = gVdirBdbGlobals.bdbIndexDBs.pIndexDBs[pIdxDesc->iId].pBdbDataFiles[0].pDB;
    indTypes = pIdxDesc->iTypes;
    assert( indTypes == INDEX_TYPE_EQUALITY );
    uniqueVal = pIdxDesc->bIsUnique;
    assert( uniqueVal );

    key.data = keyData;
    EntryIdToDBT( entryId, &key );
    *(unsigned char *)((unsigned char *)key.data + key.size) = ':';
    key.size++;
    writer = ((unsigned char *)key.data + key.size);
    VmDirEncodeShort( &writer, attr->pATDesc->usAttrID );
    key.size += 2;

    value.data = attr->metaData;
    value.size = VmDirStringLenA(attr->metaData);

    retVal = UpdateKeyValue( db, txn, &key, &value, uniqueVal, ulOPMask );
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "UpdateAttributeMetaData: End, retVal = %d", retVal );
    return retVal;

error:
    goto cleanup;
}


/*
 * UpdateIndicesForAttribute(): If the given attribute is indexed, create/delete the required indices.
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB
 *
 *     Returns Success if the attribute is not indexed. => nothing to be done.
 */
int
UpdateIndicesForAttribute(
    DB_TXN *         txn,
    VDIR_BERVALUE *  attrType,
    VDIR_BERVARRAY   attrVals, // Normalized Attribute Values
    unsigned         numVals,
    db_seq_t         entryId,
    ULONG            ulOPMask
    )
{
    int                   retVal = 0;
    DBT                   key = {0};
    DBT                   value = {0};
    int                   maxRqdKeyLen = 0;
    char *                keyData = NULL;
    DB *                  db = NULL;
    int                   indTypes = 0;
    BOOLEAN               uniqueVal = FALSE;
    int                   i = 0;
    int                   j = 0;
    int                   k = 0;
    unsigned char         eIdBytes[sizeof( db_seq_t )];

    PVDIR_CFG_ATTR_INDEX_DESC   pIdxDesc = NULL;
    USHORT                      usVersion = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "UpdateIndicesForAttribute: Begin, attrType: %s", attrType->lberbv.bv_val );

    if ((pIdxDesc = VmDirAttrNameToWriteIndexDesc(attrType->lberbv.bv_val, usVersion, &usVersion)) != NULL)
    {
        db = gVdirBdbGlobals.bdbIndexDBs.pIndexDBs[pIdxDesc->iId].pBdbDataFiles[0].pDB;
        indTypes = pIdxDesc->iTypes;
        uniqueVal = pIdxDesc->bIsUnique;

        // Calculate required maximum length of the key.
        for (i=0; i<numVals; i++)
        {
            if (BERVAL_NORM_LEN(attrVals[i]) > maxRqdKeyLen)
            {
                maxRqdKeyLen = BERVAL_NORM_LEN(attrVals[i]);
            }
        }
        maxRqdKeyLen += 1; // For adding the Key type in front

        if (VmDirAllocateMemory( maxRqdKeyLen, (PVOID *)&keyData ) != 0)
        {
            // SJ-TBD: In this function, this error code should really be in DB_ error space
            retVal = ERROR_BACKEND_OPERATIONS;
            BAIL_ON_VMDIR_ERROR( retVal );
        }
        assert (keyData != NULL);

        value.data = &eIdBytes[0];
        EntryIdToDBT(entryId, &value);

        for (i=0; i<numVals; i++)
        {
            char* pNormVal = BERVAL_NORM_VAL(attrVals[i]);
            ber_len_t normValLen = BERVAL_NORM_LEN(attrVals[i]);

            // Create a normal index
            if (indTypes & INDEX_TYPE_EQUALITY)
            {
                memset(&key, 0, sizeof(DBT));
                key.data = keyData;
                *(char *)(key.data) = INDEX_KEY_TYPE_FWD;
                memcpy(((char *)key.data + 1),
                        pNormVal,
                        normValLen);
                key.size = normValLen + 1;
                key.flags = DB_DBT_USERMEM;
                key.ulen = key.dlen = key.size;
                key.doff = 0;

                retVal = UpdateKeyValue( db, txn, &key, &value, uniqueVal, ulOPMask );
                BAIL_ON_VMDIR_ERROR( retVal );
            }

            // At least create a reverse index. => Normal index and reverse index should take care of initial substring
            // and final substring filters.
            if (indTypes & INDEX_TYPE_SUBSTR)
            {
                memset(&key, 0, sizeof(DBT));
                key.data = keyData;
                *(char *)key.data = INDEX_KEY_TYPE_REV;
                // Reverse copy from attrVals[i]->lberbv.bv_val to &(key.data[1])
                for (j=normValLen - 1, k=1; j >= 0; j--, k++)
                {
                    *((char *)key.data + k) = pNormVal[j];
                }
                key.size = normValLen + 1;
                key.flags = DB_DBT_USERMEM;
                key.ulen = key.dlen = key.size;
                key.doff = 0;

                retVal = UpdateKeyValue( db, txn, &key, &value, uniqueVal, ulOPMask );
                BAIL_ON_VMDIR_ERROR( retVal );
            }
        }
    }

cleanup:
    if (keyData != NULL)
    {
        free (keyData);
    }
    VmDirLog( LDAP_DEBUG_TRACE, "UpdateIndicesForAttribute: End, retVal = %d", retVal );
    return retVal;

error:
    VmDirLog( LDAP_DEBUG_ANY, "UpdateIndicesForAttribute failed, attribute: %s, error code: %d, error string: %s",
              attrType->lberbv.bv_val, retVal, db_strerror(retVal) );
    goto cleanup;
}


static int
CompareEntryIds(
    const void * eId1,
    const void * eId2)
{
    return ( *(db_seq_t *)eId1 < *(db_seq_t *)eId2 ? -1 : *(db_seq_t *)eId1 == *(db_seq_t *)eId2 ? 0 : 1);
}

/* DBTToEntryId: Convert DBT data bytes sequence to EntryId (db_seq_t/long long).
 *
 */
static void
DBTToEntryId(
    DBT *      dbt,
    db_seq_t * eId)
{
    int      i = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "DBTToEntryId: Begin" );

    *eId = 0;
    for (i = 0; i < dbt->size; i++ )
    {
        *eId <<= 8;
        *eId |= (unsigned char) ((unsigned char *)(dbt->data))[i];
    }

    VmDirLog( LDAP_DEBUG_TRACE, "DBTToEntryId: End, EntryId = %lld", *eId );
}

/*
 * DeleteKeyValue(). If it is a unique index, just delete the key, otherwise using cursor, go to the desired
 * value for the key, and delete that particular key-value pair.
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB, e.g. DB_NOTFOUND if no match was found.
 */

static int
DeleteKeyValue(
    DB *            db,
    DB_TXN *        txn,
    DBT *           key,
    DBT *           value,
    BOOLEAN         uniqueVal)
{
    int                      retVal = 0;
    DBC *                    cursor = NULL;

    VmDirLog( LDAP_DEBUG_TRACE, "DeleteKeyValue: Begin" );

    if (uniqueVal)
    {
        retVal = db->del( db, txn, key, BDB_FLAGS_ZERO );
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    else
    {
        retVal = db->cursor( db, txn, &cursor, BDB_FLAGS_ZERO );
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = cursor->get( cursor, key, value, DB_GET_BOTH );
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = cursor->del( cursor, BDB_FLAGS_ZERO );
        BAIL_ON_VMDIR_ERROR(retVal);
    }

cleanup:
    if (cursor != NULL)
    {
        cursor->c_close( cursor );
    }
    VmDirLog( LDAP_DEBUG_TRACE, "DeleteKeyValue: End, retVal = %d", retVal );
    return retVal;

error:
    VmDirLog( LDAP_DEBUG_ANY, "DeleteKeyValue failed with error code: %d, error string: %s", retVal, db_strerror(retVal) );
    goto cleanup;
}

/*
 * ScanIndex(). For the given attrType and key, get the entryIds (return them in candidates) that match that key.
 * Handles the following 2 cases differently:
 *     - When a unique entryId is expected. Just uses db->get()
 *     - When multiple entryIds are expected. Uses cursor. This can happen in the following cases:
 *          - partial (substring) match
 *          - non-unique key values
 *          - GE match
 *          - combinations of above options
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB, e.g. DB_NOTFOUND if no match was found.
 */
static int
ScanIndex(
    DB_TXN *        txn,
    VDIR_BERVALUE * attrType,
    DBT *           key,
    VDIR_FILTER *  f,
    const char **   errText)
{
    int                      retVal = 0;
    DB *                     db = NULL;
    BOOLEAN                  uniqueVal = FALSE;
    DBT                      value = {0};
    db_seq_t                 eId = 0;
    DB_TXN *                 localTxn = NULL;
    DBC *                    cursor = NULL;
    unsigned int             cursorFlags;
    DBT                      currKey = {0};
    unsigned char            eIdBytes[sizeof( db_seq_t )];

    PVDIR_CFG_ATTR_INDEX_DESC   pIdxDesc = NULL;
    USHORT                      usVersion = 0;

    // GE filter is neither exactMatch nor a partialMatch
    BOOLEAN                 exactMatch = (f->choice == LDAP_FILTER_EQUALITY ||  f->choice == FILTER_ONE_LEVEL_SEARCH);
    BOOLEAN                 partialMatch = (f->choice == LDAP_FILTER_SUBSTRINGS);

    VmDirLog( LDAP_DEBUG_TRACE, "ScanIndex: Begin, attrType = %s, f->choice = %d", attrType->lberbv.bv_val, f->choice );

    *errText = NULL;

    pIdxDesc = VmDirAttrNameToReadIndexDesc(attrType->lberbv.bv_val, usVersion, &usVersion);
    if (!pIdxDesc)
    {
        VmDirLog( LDAP_DEBUG_ANY, "ScanIndex: non-indexed attribute. attrType = %s", attrType->lberbv.bv_val);
        goto cleanup;
    }

    f->candidates = NewCandidates(CANDIDATES_START_ALLOC_SIZE, TRUE);

    db = gVdirBdbGlobals.bdbIndexDBs.pIndexDBs[pIdxDesc->iId].pBdbDataFiles[0].pDB;
    uniqueVal = pIdxDesc->bIsUnique;

    eId = 0;
    memset(&value, 0, sizeof(DBT));
    value.flags = DB_DBT_USERMEM;
    value.data = &(eIdBytes[0]);
    value.ulen = sizeof( db_seq_t );

    /* If no bigger transaction going on/passed-in */
    if (txn == NULL)
    {
        /* Transaction, "required" (because it spans multiple calls) for cursor operations. */
        if ((retVal = gVdirBdbGlobals.bdbEnv->txn_begin (gVdirBdbGlobals.bdbEnv, BDB_PARENT_TXN_NULL, &localTxn,
                                                         DB_READ_COMMITTED)) != 0)
        {
            *errText = "txn_begin DB lookup failed.";
            BAIL_ON_VMDIR_ERROR(retVal);
        }
    }
    else
    {
        localTxn = txn;
    }

    if ( exactMatch && uniqueVal )
    {
        if ((retVal = db->get( db, localTxn, key, &value, DB_READ_COMMITTED )) != 0)
        {
            if (retVal != DB_NOTFOUND)
            {
                *errText = "Index DB lookup failed.";
            }
            BAIL_ON_VMDIR_ERROR(retVal);
        }

        DBTToEntryId( &value, &eId);
        retVal = VmDirAddToCandidates( f->candidates, eId);
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    else
    {
        if ((retVal = db->cursor( db, localTxn, &cursor, BDB_FLAGS_ZERO )) != 0)
        {
            *errText = "Opening the cursor failed.";
            BAIL_ON_VMDIR_ERROR(retVal);

        }

        memset(&currKey, 0, sizeof(DBT));
        currKey.flags = DB_DBT_USERMEM | DB_DBT_PARTIAL;
        currKey.size = key->size;
        if (VmDirAllocateMemory( key->size, (PVOID *)&currKey.data ) != 0)
        {
            *errText = "VmDirAllocateMemory failed.";
            retVal = ERROR_BACKEND_OPERATIONS; // SJ-TBD: In this function, this error code should really be in DB_ error space
            BAIL_ON_VMDIR_ERROR( retVal );
        }
        memcpy( currKey.data, key->data, key->size);
        currKey.ulen = currKey.dlen = currKey.size;
        currKey.doff = 0;

        cursorFlags = exactMatch ? DB_SET : DB_SET_RANGE;
        do
        {
            if ((retVal = cursor->get( cursor, &currKey, &value, cursorFlags )) != 0)
            {
                if (retVal == DB_NOTFOUND)
                {
                    if (f->candidates->size > 0) // We had found something.)
                    {
                        retVal = 0;
                    }
                }
                else
                {
                    VmDirLog( LDAP_DEBUG_ANY, "ScanIndex: cursor->get(DB_SET) failed with error code: %d, "
                              "error string: %s", retVal, db_strerror(retVal) );
                    *errText = "get (DB_SET) cursor failed.";
                    BAIL_ON_VMDIR_ERROR( retVal );
                }
                break;
            }
            // In case of partial match, check if we are passed the partial key match that we are looking for.
            if (partialMatch && memcmp(key->data, currKey.data, key->size) != 0)
            {
                break;
            }
            DBTToEntryId( &value, &eId);
            retVal = VmDirAddToCandidates( f->candidates, eId);
            BAIL_ON_VMDIR_ERROR(retVal);
            eId = 0;
            cursorFlags = exactMatch ? DB_NEXT_DUP : DB_NEXT;
        }
        while (TRUE);

        if (VmDirStringCompareA( attrType->lberbv.bv_val, ATTR_USN_CHANGED, FALSE ) == 0)
        {
            // For search on usnChanged attribute, we want to keep the sort order based on usnChanged.
        }
        else
        {
            qsort ( f->candidates->eIds, f->candidates->size, sizeof( db_seq_t ), CompareEntryIds );
        }
    }

cleanup:
    VmDirFreeMemory( currKey.data );
    if (cursor != NULL)
    {
        cursor->c_close( cursor );
    }
    if (txn == NULL && localTxn != NULL) /* commit/abort local transaction */
    {
        if (retVal == 0 || retVal == DB_NOTFOUND)
        {
            retVal = localTxn->commit (localTxn, BDB_FLAGS_ZERO);
        }
        else
        {
            localTxn->abort(localTxn);
        }
    }
    VmDirLog( LDAP_DEBUG_TRACE, "ScanIndex: End, retVal = %d, #of candidates = %d",
              retVal, f->candidates == NULL ? 0 : f->candidates->size );
    return retVal;

error:
    if (retVal != DB_NOTFOUND)
    {
        DeleteCandidates( &(f->candidates) );
    }
    VmDirLog( LDAP_DEBUG_ANY, "ScanIndex failed with error code: %d, error string: %s", retVal, db_strerror(retVal) );
    goto cleanup;
}

/*
 * UpdateKeyValue(). If it is a unique index, just delete the key, otherwise using cursor, go to the desired
 * entryId value for the key, and delete that particular key-value pair.
 *
 * Return values:
 *     On Success: 0
 *     On Error: Error codes returned by BDB, e.g. DB_NOTFOUND if no match was found.
 */

static int
UpdateKeyValue(
    DB *            db,
    DB_TXN *        txn,
    DBT *           key,
    DBT *           value,
    BOOLEAN         uniqueVal,
    ULONG           ulOPMask)
{
    int  retVal = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "UpdateKeyValue: Begin" );

    switch ( ulOPMask )
    {
        case BDB_INDEX_OP_TYPE_CREATE:
            retVal = db->put (db, txn, key, value, uniqueVal ? DB_NOOVERWRITE : BDB_FLAGS_ZERO);
            BAIL_ON_VMDIR_ERROR( retVal );
            break;
        case BDB_INDEX_OP_TYPE_UPDATE:
            retVal = db->put (db, txn, key, value, BDB_FLAGS_ZERO);
            BAIL_ON_VMDIR_ERROR( retVal );
            break;
        case BDB_INDEX_OP_TYPE_DELETE:
            retVal = DeleteKeyValue(db, txn, key, value, uniqueVal);
            BAIL_ON_VMDIR_ERROR( retVal );
            break;
        default:
            assert(FALSE);
    }

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "UpdateKeyValue: End" );
    return retVal;

error:
    VmDirLog( LDAP_DEBUG_ANY, "UpdateKeyValue: failed with error code: %d, error string: %s", retVal, db_strerror(retVal) );
    goto cleanup;
}


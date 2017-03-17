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

static
int
_VmDirDeleteOldValueMetaData(
    PVDIR_BACKEND_CTX pBECtx,
    ENTRYID entryId,
    short attrId,
    PVDIR_BERVALUE pAVmetaToAdd
    );

/*
 * MDBUpdateKeyValue(). If it is a unique index, just delete the key, otherwise using cursor, go to the desired
 * entryId value for the key, and delete that particular key-value pair.
 *
 * Return values:
 *     On Success: 0
 *     On Error: MDB error
 */
DWORD
MdbUpdateKeyValue(
   VDIR_DB             mdbDBi,
   PVDIR_DB_TXN        pTxn,
   PVDIR_DB_DBT        pKey,
   PVDIR_DB_DBT        pValue,
   BOOLEAN             bIsUniqueVal,
   ULONG               ulOPMask)
{
    DWORD   dwError = 0;

    switch ( ulOPMask )
    {
        case BE_INDEX_OP_TYPE_CREATE:
            dwError = mdb_put(pTxn, mdbDBi, pKey, pValue, bIsUniqueVal ? MDB_NOOVERWRITE : BE_DB_FLAGS_ZERO);
            BAIL_ON_VMDIR_ERROR( dwError );
            break;
        case BE_INDEX_OP_TYPE_UPDATE:
            dwError = mdb_put(pTxn, mdbDBi, pKey, pValue, BE_DB_FLAGS_ZERO);
            BAIL_ON_VMDIR_ERROR( dwError );
            break;
        case BE_INDEX_OP_TYPE_DELETE:
            dwError = MdbDeleteKeyValue(mdbDBi, pTxn, pKey, pValue, bIsUniqueVal);
            BAIL_ON_VMDIR_ERROR( dwError );
            break;
        default:
            assert(FALSE);
    }

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( LDAP_DEBUG_BACKEND, "MDBUpdateKeyValue: failed with error code: %d, error string: %s",
              dwError, mdb_strerror(dwError) );

    goto cleanup;
}

/*
 * DeleteKeyValue(). If it is a unique index, just delete the key, otherwise using cursor, go to the desired
 * value for the key, and delete that particular key-value pair.
 *
 * Return values:
 *     On Success: 0
 *     On Error: MDB error
*/
DWORD
MdbDeleteKeyValue(
    VDIR_DB             mdbDBi,
    PVDIR_DB_TXN        pTxn,
    PVDIR_DB_DBT        pKey,
    PVDIR_DB_DBT        pValue,
    BOOLEAN             bIsUniqueVal)
{
    DWORD   dwError = 0;

    if (bIsUniqueVal)
    {   // unique key case, no need to match pValue parameter
        dwError = mdb_del(pTxn, mdbDBi, pKey, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {   // delete matched key and value record only.
        dwError = mdb_del(pTxn, mdbDBi, pKey, pValue);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( LDAP_DEBUG_BACKEND, "DeleteKeyValue failed with error code: %d, error string: %s",
              dwError, mdb_strerror(dwError) );

    goto cleanup;
}

/*
 * MDBDeleteEIdIndex(): In blob.db, delete entryid => blob index.
 *
 * Return values:
 *     On Success: 0
 *     On Error: BE error
 */
DWORD
MDBDeleteEIdIndex(
    PVDIR_DB_TXN    pTxn,
    ENTRYID         entryId
    )
{
    DWORD               dwError = 0;
    VDIR_DB_DBT         key = {0};
    VDIR_DB             mdbDBi = 0;
    unsigned char       EIdBytes[sizeof( ENTRYID )] = {0};

    assert(pTxn);

    mdbDBi = gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].mdbDBi;

    key.mv_data = &EIdBytes[0];
    MDBEntryIdToDBT(entryId, &key);
    // entry DB is guarantee to be unique key.
    dwError = MdbDeleteKeyValue(mdbDBi, pTxn, &key, NULL, TRUE);
    BAIL_ON_VMDIR_ERROR( dwError );

cleanup:

    return dwError;

error:

    VMDIR_SET_BACKEND_ERROR(dwError);

    goto cleanup;
}

/*
 * UpdateAttributeMetaData(): Update attribute's meta data.
 *
 * Return values:
 *     On Success: 0
 *     On Error: MDB error
 */
DWORD
MdbUpdateAttrMetaData(
    PVDIR_DB_TXN     pTxn,
    VDIR_ATTRIBUTE * attr,
    ENTRYID          entryId,
    ULONG            ulOPMask)
{
    DWORD                 dwError = 0;
    VDIR_DB_DBT           key = {0};
    VDIR_DB_DBT           value = {0};
    char                  keyData[ sizeof( ENTRYID ) + 1 + 2 ] = {0}; /* key format is: <entry ID>:<attribute ID (a short)> */
    VDIR_DB               mdbDBi = 0;
    int                   indTypes = 0;
    BOOLEAN               bIsUniqueVal = FALSE;
    VDIR_BERVALUE         attrMetaDataAttr = { {ATTR_ATTR_META_DATA_LEN, ATTR_ATTR_META_DATA}, 0, 0, NULL };
    unsigned char *       pWriter = NULL;
    PVDIR_INDEX_CFG       pIndexCfg = NULL;

    // E.g. while deleting a user, and therefore updating the member attribute of the groups to which the user belongs,
    // member attrMetaData of the group object is left unchanged (at least in the current design, SJ-TBD).
    if (ulOPMask == BE_INDEX_OP_TYPE_UPDATE && VmDirStringLenA( attr->metaData ) == 0)
    {
        goto cleanup;
    }

    dwError = VmDirIndexCfgAcquire(
            attrMetaDataAttr.lberbv.bv_val, VDIR_INDEX_WRITE, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);

    indTypes = pIndexCfg->iTypes;
    assert( indTypes == INDEX_TYPE_EQUALITY );
    bIsUniqueVal = pIndexCfg->bGlobalUniq;
    assert( bIsUniqueVal );

    key.mv_data = &keyData[0];
    MDBEntryIdToDBT( entryId, &key );
    *(unsigned char *)((unsigned char *)key.mv_data + key.mv_size) = ':';
    key.mv_size++;
    pWriter = ((unsigned char *)key.mv_data + key.mv_size);
    VmDirEncodeShort( &pWriter, attr->pATDesc->usAttrID );
    key.mv_size += 2;

    value.mv_data = attr->metaData;
    value.mv_size = VmDirStringLenA(attr->metaData);

    dwError = MdbUpdateKeyValue( mdbDBi, pTxn, &key, &value, bIsUniqueVal, ulOPMask );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    return dwError;

error:
    VMDIR_LOG_ERROR(LDAP_DEBUG_BACKEND,
             "UpdateAttributeMetaData failed: error=%d,eid=%ld", dwError, entryId);

    VMDIR_LOG_VERBOSE(LDAP_DEBUG_BACKEND,
             "UpdateAttributeMetaData failed: key=(%p)(%.*s), value=(%p)(%.*s)\n",
             key.mv_data,   VMDIR_MIN(key.mv_size,   VMDIR_MAX_LOG_OUTPUT_LEN), (char *) key.mv_data,
             value.mv_data, VMDIR_MIN(value.mv_size, VMDIR_MAX_LOG_OUTPUT_LEN), (char *) value.mv_data);

    goto cleanup;
}

/*
 * UpdateIndicesForAttribute(): If the given attribute is indexed, create/delete the required indices.
 *
 * Return values:
 *     On Success: 0
 *     On Error: MDB error
 *
 *     Returns Success if the attribute is not indexed. => nothing to be done.
 */
DWORD
MdbUpdateIndicesForAttr(
    PVDIR_DB_TXN        pTxn,
    VDIR_BERVALUE *     entryDN,
    VDIR_BERVALUE *     attrType,
    VDIR_BERVARRAY      attrVals, // Normalized Attribute Values
    unsigned            numVals,
    ENTRYID             entryId,
    ULONG               ulOPMask
    )
{
    DWORD               dwError = 0;
    VDIR_DB_DBT         value = {0};
    VDIR_DB_DBT         key = {0};
    ber_len_t           maxRqdKeyLen = 0;
    PSTR                pKeyData = NULL;
    VDIR_DB             mdbDBi = 0;
    int                 indTypes = 0;
    BOOLEAN             bIsUniqueVal = FALSE;
    unsigned char       eIdBytes[sizeof( ENTRYID )] = {0};
    PVDIR_INDEX_CFG     pIndexCfg = NULL;

    dwError = VmDirIndexCfgAcquire(
            attrType->lberbv.bv_val, VDIR_INDEX_WRITE, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pIndexCfg)
    {
        unsigned int    i = 0;
        PSTR            pszDN = BERVAL_NORM_VAL(*entryDN);

        indTypes = pIndexCfg->iTypes;
        bIsUniqueVal = pIndexCfg->bGlobalUniq;

        dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
        BAIL_ON_VMDIR_ERROR(dwError);

        // Calculate required maximum length of the key.
        for (i=0; i<numVals; i++)
        {
            if (BERVAL_NORM_LEN(attrVals[i]) > maxRqdKeyLen)
            {
                maxRqdKeyLen = BERVAL_NORM_LEN(attrVals[i]);
            }
        }
        maxRqdKeyLen += 1; // For adding the Key type in front

        if (VmDirAllocateMemory( maxRqdKeyLen, (PVOID *)&pKeyData ) != 0)
        {
            dwError = ERROR_BACKEND_OPERATIONS;
            BAIL_ON_VMDIR_ERROR( dwError );
        }
        assert (pKeyData != NULL);

        value.mv_data = &eIdBytes[0];
        MDBEntryIdToDBT(entryId, &value);

        for (i=0; i<numVals; i++)
        {
            char*       pNormVal   = BERVAL_NORM_VAL(attrVals[i]);
            ber_len_t   normValLen = BERVAL_NORM_LEN(attrVals[i]);

            dwError = MdbValidateAttrUniqueness(pIndexCfg, pNormVal, pszDN, ulOPMask);
            BAIL_ON_VMDIR_ERROR( dwError );

            key.mv_size = 0;
            key.mv_data = pKeyData;

            // Create a normal index
            if (indTypes & INDEX_TYPE_EQUALITY)
            {
                *(char *)(key.mv_data) = BE_INDEX_KEY_TYPE_FWD;
                dwError = VmDirCopyMemory(((char *)key.mv_data + 1), normValLen, pNormVal, normValLen);
                BAIL_ON_VMDIR_ERROR(dwError);

                key.mv_size = normValLen + 1;

                dwError = MdbUpdateKeyValue( mdbDBi, pTxn, &key, &value, bIsUniqueVal, ulOPMask );
                BAIL_ON_VMDIR_ERROR( dwError );
            }

            // At least create a reverse index. => Normal index and reverse index should take care of initial substring
            // and final substring filters.
            if (indTypes & INDEX_TYPE_SUBSTR)
            {
                ber_len_t     j = 0;
                ber_len_t     k = 0;

                *(char *)key.mv_data = BE_INDEX_KEY_TYPE_REV;
                // Reverse copy from attrVals[i]->lberbv.bv_val to &(key.data[1])
                for (j=normValLen, k=1; j > 0; j--, k++)
                {
                    *((char *)key.mv_data + k) = pNormVal[j-1];
                }

                key.mv_size = normValLen + 1;

                dwError = MdbUpdateKeyValue( mdbDBi, pTxn, &key, &value, bIsUniqueVal, ulOPMask );
                BAIL_ON_VMDIR_ERROR( dwError );
            }
        }
    }

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    VMDIR_SAFE_FREE_MEMORY(pKeyData);

    return dwError;

error:
    VMDIR_LOG_ERROR(LDAP_DEBUG_BACKEND,
             "MDBUpdateIndicesForAttr failed: error=%d,eid=%ld,attr=%s",
             dwError, entryId, VDIR_SAFE_STRING(attrType->lberbv.bv_val));

    VMDIR_LOG_VERBOSE(LDAP_DEBUG_BACKEND,
             "MDBUpdateIndicesForAttr failed: key=(%p)(%.*s), value=(%p)(%.*s)",
             key.mv_data,   VMDIR_MIN(key.mv_size,   VMDIR_MAX_LOG_OUTPUT_LEN),  (char *) key.mv_data,
             value.mv_data, VMDIR_MIN(value.mv_size, VMDIR_MAX_LOG_OUTPUT_LEN),  (char *) value.mv_data);

    goto cleanup;
}

DWORD
MdbValidateAttrUniqueness(
    PVDIR_INDEX_CFG     pIndexCfg,
    PSTR                pszAttrVal,
    PSTR                pszEntryDN,
    ULONG               ulOPMask
    )
{
    DWORD   dwError = 0;
    size_t  entryDnLen = 0;
    PVDIR_BACKEND_INDEX_ITERATOR    pIterator = NULL;
    PLW_HASHMAP             pOccupiedScopes = NULL;
    LW_HASHMAP_ITER         iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR         pair = {NULL, NULL};
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    PSTR        pszVal = NULL;
    ENTRYID     eId = 0;
    VDIR_ENTRY  entry = {0};
    PSTR        pszScope = NULL;
    PSTR        pszScopeCopy = NULL;
    PSTR        pszDN = NULL;
    PSTR        pszDNCopy = NULL;
    PVMDIR_MUTEX    pMutex = NULL;
    BOOLEAN         bInLock = FALSE;

    if (!pIndexCfg || IsNullOrEmptyString(pszAttrVal))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = LwRtlCreateHashMap(&pOccupiedScopes,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    // delete cannot violate uniqueness
    if (ulOPMask == BE_INDEX_OP_TYPE_DELETE)
    {
        goto cleanup;
    }

    // no uniqueness enforced
    if (LwRtlHashMapGetCount(pIndexCfg->pUniqScopes) == 0)
    {
        // use pNewUniqScopes if VDIR_INDEXING_VALIDATING_SCOPES
        if (pIndexCfg->status != VDIR_INDEXING_VALIDATING_SCOPES ||
            VmDirLinkedListIsEmpty(pIndexCfg->pNewUniqScopes))
        {
            goto cleanup;
        }
    }

    dwError = VmDirMDBIndexIteratorInit(pIndexCfg, pszAttrVal, &pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMutex = pIndexCfg->mutex;
    VMDIR_LOCK_MUTEX(bInLock, pMutex);

    // find all uniqueness scopes that are already occupied
    while (pIterator->bHasNext)
    {
        dwError = VmDirMDBIndexIterate(pIterator, &pszVal, &eId);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (VmDirStringCompareA(pszAttrVal, pszVal, FALSE) != 0)
        {
            break;
        }

        dwError = VmDirMDBSimpleEIdToEntry(eId, &entry);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszDN = BERVAL_NORM_VAL(entry.dn);

        // find occupied scopes in pUniqScopes
        LwRtlHashMapResetIter(&iter);
        while (LwRtlHashMapIterate(pIndexCfg->pUniqScopes, &iter, &pair))
        {
            pszScope = pair.pKey;

            if (VmDirStringCompareA(PERSISTED_DSE_ROOT_DN, pszScope, FALSE) == 0 ||
                VmDirStringEndsWith(pszDN, pszScope, FALSE))
            {
                if (LwRtlHashMapFindKey(pOccupiedScopes, NULL, pszScope) != 0)
                {
                    // create and store copies in the map
                    dwError = VmDirAllocateStringA(pszScope, &pszScopeCopy);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirAllocateStringA(pszDN, &pszDNCopy);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = LwRtlHashMapInsert(
                            pOccupiedScopes, pszScopeCopy, pszDNCopy, NULL);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    pszScopeCopy = NULL;
                    pszDNCopy = NULL;
                }
                else
                {
                    // uniqueness is already violated, no recovery plan
                    assert(FALSE);
                }
            }
        }

        // find occupied scopes in pNewUniqScopes
        if (pIndexCfg->status == VDIR_INDEXING_VALIDATING_SCOPES)
        {
            pNode = pIndexCfg->pNewUniqScopes->pTail;
            while (pNode)
            {
                pszScope = (PSTR)pNode->pElement;

                if (VmDirStringCompareA(PERSISTED_DSE_ROOT_DN, pszScope, FALSE) == 0 ||
                    VmDirStringEndsWith(pszDN, pszScope, FALSE))
                {
                    if (LwRtlHashMapFindKey(pOccupiedScopes, NULL, pszScope) != 0)
                    {
                        // create and store copies in the map
                        dwError = VmDirAllocateStringA(pszScope, &pszScopeCopy);
                        BAIL_ON_VMDIR_ERROR(dwError);

                        dwError = VmDirAllocateStringA(pszDN, &pszDNCopy);
                        BAIL_ON_VMDIR_ERROR(dwError);

                        dwError = LwRtlHashMapInsert(
                                pOccupiedScopes, pszScopeCopy, pszDNCopy, NULL);
                        BAIL_ON_VMDIR_ERROR(dwError);

                        pszScopeCopy = NULL;
                        pszDNCopy = NULL;
                    }
                }

                pNode = pNode->pNext;
            }
        }

        VMDIR_SAFE_FREE_MEMORY(pszVal);
        VmDirFreeEntryContent(&entry);
    }

    // check if the new entry dn matches any occupied scope
    entryDnLen = VmDirStringLenA(pszEntryDN);

    LwRtlHashMapResetIter(&iter);
    while (LwRtlHashMapIterate(pOccupiedScopes, &iter, &pair))
    {
        pszScope = (PSTR)pair.pKey;
        pszDN = (PSTR)pair.pValue;

        if (VmDirStringCompareA(pszEntryDN, pszDN, FALSE) != 0 &&
            (VmDirStringCompareA(PERSISTED_DSE_ROOT_DN, pszScope, FALSE) == 0 ||
             VmDirStringEndsWith(pszEntryDN, pszScope, FALSE)))
        {
            // found conflict
            // reject in order to preserve uniqueness
            dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;

            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                    "%s detected that attr '%s' value '%s' "
                    "already exists in scope '%s', "
                    "will return error %d",
                    __FUNCTION__,
                    pIndexCfg->pszAttrName,
                    pszAttrVal,
                    pszScope,
                    dwError );

            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pMutex);
    VmDirMDBIndexIteratorFree(pIterator);
    if (pOccupiedScopes)
    {
        LwRtlHashMapClear(pOccupiedScopes, VmDirSimpleHashMapPairFree, NULL);
        LwRtlFreeHashMap(&pOccupiedScopes);
    }
    VMDIR_SAFE_FREE_MEMORY(pszVal);
    VmDirFreeEntryContent(&entry);
    return dwError;

error:
    if (dwError != VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)", __FUNCTION__, dwError );
    }

    VMDIR_SAFE_FREE_MEMORY(pszScopeCopy);
    VMDIR_SAFE_FREE_MEMORY(pszDNCopy);
    goto cleanup;
}

/*
 * CreateParentIdIndex(): In parentid.db, create parentId => entryId index. => mainly used in one-level searches.
 *
 * Return values:
 *     On Success: 0
 *     On Error: BE error
 */
DWORD
MDBCreateParentIdIndex(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE *     pdn,
    ENTRYID             entryId)
{
    DWORD               dwError = 0;
    VDIR_DB_DBT         key = {0};
    VDIR_DB_DBT         value = {0};
    VDIR_DB             mdbDBi = 0;
    BOOLEAN             bIsUniqueVal = FALSE;
    VDIR_BERVALUE       parentIdAttr = { {ATTR_PARENT_ID_LEN, ATTR_PARENT_ID}, 0, 0, NULL };
    ENTRYID             parentId = 0;
    unsigned char       eIdBytes[sizeof( ENTRYID )] = {0};
    unsigned char       parentEIdBytes[sizeof( ENTRYID )] = {0};
    PVDIR_INDEX_CFG     pIndexCfg = NULL;
    PSTR                pszLocalErrMsg = NULL;

    assert(pBECtx && pBECtx->pBEPrivate && pdn);

    dwError = VmDirMDBDNToEntryId( pBECtx, pdn, &parentId );
    BAIL_ON_VMDIR_ERROR( dwError );

    // Update parentId => entryId index in parentid.db.
    dwError = VmDirIndexCfgAcquire(
            parentIdAttr.lberbv.bv_val, VDIR_INDEX_WRITE, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);

    bIsUniqueVal = pIndexCfg->bGlobalUniq;
    assert( bIsUniqueVal == FALSE );

    key.mv_data = &parentEIdBytes[0];
    MDBEntryIdToDBT(parentId, &key);

    value.mv_data = &eIdBytes[0];
    MDBEntryIdToDBT(entryId, &value);

    if ((dwError = mdb_put((PVDIR_DB_TXN)pBECtx->pBEPrivate, mdbDBi, &key, &value, BE_DB_FLAGS_ZERO)) != 0)
    {
        DWORD   dwTmp = dwError;
        dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx, VDIR_SAFE_STRING(pdn->lberbv.bv_val));
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                        "CreateParentIdIndex: For entryId: %lld, mdb_put failed with error code: %d, error "
                        "string: %s", entryId, dwTmp, VDIR_SAFE_STRING(pBECtx->pszBEErrorMsg) );
    }

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return dwError;

error:

    VMDIR_LOG_ERROR( LDAP_DEBUG_BACKEND, VDIR_SAFE_STRING(pszLocalErrMsg));

    VMDIR_SET_BACKEND_ERROR(dwError);

    goto cleanup;
}

/*
 * DeleteParentIdIndex(): In parentid.db, delete parentId => entryId index.
 *
 * Return values:
 *     On Success: 0
 *     On Error: BE error
 */
DWORD
MDBDeleteParentIdIndex(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE *     pdn,
    ENTRYID             entryId)
{
    DWORD               dwError = 0;
    VDIR_DB_DBT         key = {0};
    VDIR_DB_DBT         value = {0};
    VDIR_DB             mdbDBi = 0;
    BOOLEAN             bIsUniqueVal = FALSE;
    VDIR_BERVALUE       parentIdAttr = { {ATTR_PARENT_ID_LEN, ATTR_PARENT_ID}, 0, 0, NULL };
    ENTRYID             parentId = 0;
    unsigned char       parentEIdBytes[sizeof( ENTRYID )] = {0};
    unsigned char       entryIdBytes[sizeof( ENTRYID )] = {0};
    PVDIR_INDEX_CFG     pIndexCfg = NULL;

    assert(pBECtx && pBECtx->pBEPrivate && pdn);

    dwError = VmDirMDBDNToEntryId( pBECtx, pdn, &parentId );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = VmDirIndexCfgAcquire(
            parentIdAttr.lberbv.bv_val, VDIR_INDEX_WRITE, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);

    bIsUniqueVal = pIndexCfg->bGlobalUniq;
    assert( bIsUniqueVal == FALSE );

    key.mv_data = &parentEIdBytes[0];
    MDBEntryIdToDBT(parentId, &key);

    value.mv_data = &entryIdBytes[0];
    MDBEntryIdToDBT(entryId, &value);

    dwError = MdbDeleteKeyValue(mdbDBi, (VDIR_DB_TXN*)pBECtx->pBEPrivate, &key, &value, bIsUniqueVal);
    BAIL_ON_VMDIR_ERROR( dwError );

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    return dwError;

error:

    VMDIR_SET_BACKEND_ERROR(dwError);

    goto cleanup;
}

/*
 * CreateEntryIdIndex(): In entry DB, create entryId => encodedEntry entry.
 *
 * Return values:
 *     On Success: 0
 *     On Error: MDB error
 */
DWORD
MdbCreateEIDIndex(
    PVDIR_DB_TXN     pTxn,
    ENTRYID          eId,
    VDIR_BERVALUE *  pEncodedEntry,
    BOOLEAN          bIsCreateIndex // Creating a new or updating an existing index recrod.
    )
{
    int             dwError = 0;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};
    VDIR_DB         mdbDBi = 0;
    BOOLEAN         bIsUniqueVal = FALSE;
    unsigned char   eIdBytes[sizeof( ENTRYID )] = {0};

    assert(pTxn && pEncodedEntry);

    mdbDBi = gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].mdbDBi;
    bIsUniqueVal = gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].bIsUnique;
    assert( bIsUniqueVal );

    key.mv_data = &eIdBytes[0];
    MDBEntryIdToDBT(eId, &key);

    value.mv_data = pEncodedEntry->lberbv.bv_val;
    value.mv_size = pEncodedEntry->lberbv.bv_len;

    // new index case    - MDB_NOOVERWRITE
    // update index case - MDB_FLAGS_ZERO
    dwError = mdb_put( pTxn, mdbDBi, &key, &value, bIsCreateIndex ? MDB_NOOVERWRITE : BE_DB_FLAGS_ZERO);
    BAIL_ON_VMDIR_ERROR( dwError );

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( LDAP_DEBUG_BACKEND, "CreateEntryIdIndex failed for entryId: %lld, error code: %d, error string: %s",
              eId, dwError, mdb_strerror(dwError) );

    goto cleanup;
}

/*
 * VmDirUpdateAttributeValueMetaData(): Update attribute's value meta data.
 * The attribute meta value data to be added or deleted are stored in
 * valueMetaData.
 * ulOPMask is BE_INDEX_OP_TYPE_UPDATE (for adding) or BE_INDEX_OP_TYPE_DELETE
 * (for deleting) the attribute's value meta data.
 * After consuming the attribute's value meta data or any error, contents in
 * valueMetaData are removed and freed from the queue.
 */
DWORD
VmDirMdbUpdateAttrValueMetaData(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID             entryId,
    short               attrId,
    ULONG               ulOPMask,
    PDEQUE              valueMetaData
    )
{
    DWORD           dwError = 0;
    PVDIR_DB_TXN    pTxn = NULL;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};
    char            keyData[ sizeof( ENTRYID ) + 1 + 2 ] = {0}; /* key format is: <entry ID>:<attribute ID (a short)> */
    VDIR_DB         mdbDBi = 0;
    int             indTypes = 0;
    VDIR_BERVALUE   attrValueMetaDataAttr = { {ATTR_ATTR_VALUE_META_DATA_LEN, ATTR_ATTR_VALUE_META_DATA}, 0, 0, NULL };
    unsigned char * pWriter = NULL;
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    VDIR_BERVALUE * pAVmeta = NULL;

    if (!VDIR_CONCURRENT_ATTR_VALUE_UPDATE_ENABLED)
    {
        goto cleanup;
    }

    if (dequeIsEmpty(valueMetaData))
    {
        goto cleanup;
    }

    assert( pBECtx && pBECtx->pBEPrivate );
    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;

    dwError = VmDirIndexCfgAcquire(
            attrValueMetaDataAttr.lberbv.bv_val, VDIR_INDEX_READ, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);

    indTypes = pIndexCfg->iTypes;
    assert( indTypes == INDEX_TYPE_EQUALITY );

    key.mv_data = &keyData[0];
    MDBEntryIdToDBT( entryId, &key );
    *(unsigned char *)((unsigned char *)key.mv_data + key.mv_size) = ':';
    key.mv_size++;
    pWriter = ((unsigned char *)key.mv_data + key.mv_size);
    VmDirEncodeShort( &pWriter, attrId );
    key.mv_size += 2;

    while(!dequeIsEmpty(valueMetaData))
    {
        dequePopLeft(valueMetaData, (PVOID*)&pAVmeta);
        if (ulOPMask == BE_INDEX_OP_TYPE_UPDATE)
        {
            dwError = _VmDirDeleteOldValueMetaData(pBECtx, entryId, attrId, pAVmeta);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        value.mv_data = pAVmeta->lberbv.bv_val;
        value.mv_size = pAVmeta->lberbv.bv_len;
        dwError = MdbUpdateKeyValue( mdbDBi, pTxn, &key, &value, FALSE, ulOPMask );
        BAIL_ON_VMDIR_ERROR(dwError);
        VmDirFreeBerval(pAVmeta);
        pAVmeta = NULL;
    }

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    VmDirFreeBerval(pAVmeta);
    VmDirFreeAttrValueMetaDataContent(valueMetaData);
    return dwError;

error:
    VMDIR_LOG_ERROR(LDAP_DEBUG_BACKEND,
             "UpdateAttributeMetaData failed: error=%d,eid=%ld", dwError, entryId);

    VMDIR_LOG_VERBOSE(LDAP_DEBUG_BACKEND,
             "UpdateAttributeMetaData failed: key=(%p)(%.*s), value=(%p)(%.*s)\n",
             key.mv_data,   VMDIR_MIN(key.mv_size,   VMDIR_MAX_LOG_OUTPUT_LEN), (char *) key.mv_data,
             value.mv_data, VMDIR_MIN(value.mv_size, VMDIR_MAX_LOG_OUTPUT_LEN), (char *) value.mv_data);

    goto cleanup;
}

/*
 * VmDirMdbDeleteAllAttrValueMetaData(): delete all attribute meta value data for the entryId
 * Return values:
 *     On Success: 0
 *     On Error: MDB error
 */
DWORD
VmDirMdbDeleteAllAttrValueMetaData(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    ENTRYID             entryId
)
{
    DWORD           dwError = 0;
    PVDIR_DB_TXN    pTxn = NULL;
    VDIR_DB         mdbDBi = 0;
    int             indTypes = 0;
    VDIR_BERVALUE   attrValueMetaDataAttr = { {ATTR_ATTR_VALUE_META_DATA_LEN, ATTR_ATTR_VALUE_META_DATA}, 0, 0, NULL };
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PVDIR_BERVALUE  pAVmeta = NULL;
    DEQUE           valueMetaData = {0};

    if (!VDIR_CONCURRENT_ATTR_VALUE_UPDATE_ENABLED)
    {
        goto cleanup;
    }

    dwError = VmDirMDBGetAllAttrValueMetaData(pBECtx, entryId, &valueMetaData);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dequeIsEmpty(&valueMetaData))
    {
        goto cleanup;
    }

    assert( pBECtx && pBECtx->pBEPrivate );
    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;

    dwError = VmDirIndexCfgAcquire(
            attrValueMetaDataAttr.lberbv.bv_val, VDIR_INDEX_READ, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);

    indTypes = pIndexCfg->iTypes;
    assert( indTypes == INDEX_TYPE_EQUALITY );

    while(!dequeIsEmpty(&valueMetaData))
    {
        VDIR_DB_DBT key = {0};
        VDIR_DB_DBT value = {0};
        char keyData[ sizeof( ENTRYID ) + 1 + 2 ] = {0}; /* key format is: <entry ID>:<attribute ID (a short)> */
        unsigned char * pWriter = NULL;
        PVDIR_SCHEMA_AT_DESC pATDesc = NULL;
        char *p = NULL;

        dequePopLeft(&valueMetaData, (PVOID*)&pAVmeta);
        p = VmDirStringChrA(pAVmeta->lberbv.bv_val, ':');
        if (!p)
        {
           VMDIR_LOG_ERROR(LDAP_DEBUG_BACKEND, "VmDirMdbDeleteAllAttrValueMetaData: invalid attr-value-meta-data %s",
                            VDIR_SAFE_STRING(pAVmeta->lberbv.bv_val));
           dwError = ERROR_BACKEND_OPERATIONS;
           BAIL_ON_VMDIR_ERROR( dwError );
        }
        *p = '\0';
        pATDesc = VmDirSchemaAttrNameToDesc(pSchemaCtx, pAVmeta->lberbv.bv_val);
        *p = ':';
        if (pATDesc == NULL)
        {
            VMDIR_LOG_ERROR(LDAP_DEBUG_BACKEND, "VmDirMdbDeleteAllAttrValueMetaData: VmDirSchemaAttrNameToDesc failed for attr %s",
                            VDIR_SAFE_STRING(pAVmeta->lberbv.bv_val));
            dwError = ERROR_BACKEND_OPERATIONS;
            BAIL_ON_VMDIR_ERROR( dwError );
        }

        key.mv_data = &keyData[0];
        MDBEntryIdToDBT( entryId, &key );
        *(unsigned char *)((unsigned char *)key.mv_data + key.mv_size) = ':';
        key.mv_size++;
        pWriter = ((unsigned char *)key.mv_data + key.mv_size);
        VmDirEncodeShort( &pWriter, pATDesc->usAttrID );
        key.mv_size += 2;
        value.mv_data = pAVmeta->lberbv.bv_val;
        value.mv_size = pAVmeta->lberbv.bv_len;
        dwError = MdbUpdateKeyValue( mdbDBi, pTxn, &key, &value, FALSE, BE_INDEX_OP_TYPE_DELETE );
        BAIL_ON_VMDIR_ERROR(dwError);
        VmDirFreeBerval(pAVmeta);
        pAVmeta = NULL;
    }

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    VmDirFreeBerval(pAVmeta);
    VmDirFreeAttrValueMetaDataContent(&valueMetaData);
    return dwError;

error:
    VMDIR_LOG_ERROR(LDAP_DEBUG_BACKEND,
             "VmDirMdbDeleteAllAttrValueMetaData failed: error=%d,eid=%ld", dwError, entryId);
    goto cleanup;
}

/* Sanity check valueMetaData
 * format of an attr-value-meta-data item:
 *       <attr-name>:<local-usn>:<version-no>:<originating-server-id>:<value-change-originating-server-id>
 *       :<value-change-originating time>:<value-change-originating-usn>:<opcode>:<value-size>:<value>
 *  return TRUE if the value_meta_data is a valid attr-value-meta-data item
 */
BOOLEAN
VmDirValidValueMetaEntry(
    PVDIR_BERVALUE  pValueMetaData
    )
{
    BOOLEAN isValid = FALSE;
    int i = 0;
    char *p = NULL, *pp = NULL;

    if (pValueMetaData == NULL)
    {
        goto cleanup;
    }

    for(i=0,p=pValueMetaData->lberbv.bv_val; i<8 && p; pp=VmDirStringChrA(p, ':')+1,p=pp,i++);
    if (!p)
    {
        goto cleanup;
    }
    pp = VmDirStringChrA(p, ':');
    if(pp == NULL)
    {
        goto cleanup;
    }
    *pp = '\0';
    i = VmDirStringToIA(p);
    *pp = ':';

    //check the attr-value length against the total length of value-meta-data
    if (pValueMetaData->lberbv.bv_len != pp - pValueMetaData->lberbv.bv_val + i + 1)
    {
        goto cleanup;
    }
    isValid = TRUE;

cleanup:
    return isValid;
}

/*
 * Remove the attr-value-meta-data item that matches the entryid, attrId
 * and pAVmetaToAdd's value part from the index database.
 * It is called before the new attr-value-meta-data (pAVmetaToAdd's) with the same value
 * (but with different opeartion or orignating server) is to be inserted into the index.
 * The purpose is to remove obsolete attr-value-meta-data to save storage, though it wouldn't
 * have impact on the correctness of the replication and conflict resolotion.
 * E.g. if an attr is added, and then removed on the same value, only attr-value-meta-data for
 * the removing needs to be kept for replication.
 */
static
int
_VmDirDeleteOldValueMetaData(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID             entryId,
    short               attrId,
    PVDIR_BERVALUE      pAVmetaToAdd
    )
{
    DWORD dwError = 0;
    DEQUE cur_value_meta = {0};
    DEQUE valueMetaDataToDelete = {0};
    char *ps = NULL, *pps = NULL, *ps_ts = NULL;
    char *pc = NULL, *ppc = NULL;
    int rc = 0;
    int psv_len = 0;
    VDIR_BERVALUE *pAVmeta = NULL;

    dwError = VmDirMDBGetAttrValueMetaData(pBECtx, entryId, attrId, &cur_value_meta);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dequeIsEmpty(&cur_value_meta))
    {
        goto cleanup;
    }

    ps = pAVmetaToAdd->lberbv.bv_val;
    VALUE_META_TO_NEXT_FIELD(ps, 8);
    pps = VmDirStringChrA(ps, ':');
    //ps points to <value-size>:<value>
    *pps = '\0';
    psv_len = VmDirStringToIA(ps);
    *pps = ':';
    VALUE_META_TO_NEXT_FIELD(ps, 1);
    //ps now points to <value> of attr-value-meta-data
    ps_ts = pAVmetaToAdd->lberbv.bv_val;
    VALUE_META_TO_NEXT_FIELD(ps_ts, 5);
    //ps_ts points to <value-change-originating time>
    while(!dequeIsEmpty(&cur_value_meta))
    {
        int pcv_len = 0;
        char *pc_ts = NULL;

        VmDirFreeBerval(pAVmeta);
        pAVmeta = NULL;

        dequePopLeft(&cur_value_meta, (PVOID*)&pAVmeta);
        pc = pAVmeta->lberbv.bv_val;
        VALUE_META_TO_NEXT_FIELD(pc, 8);
        ppc = VmDirStringChrA(pc, ':');
        *ppc = '\0';
        pcv_len = VmDirStringToIA(pc);
        *ppc = ':';
        if (psv_len != pcv_len)
        {
            continue;
        }
        VALUE_META_TO_NEXT_FIELD(pc, 1);
        if (memcmp(ps, pc, pcv_len))
        {
            continue;
        }

        // now found attr-value-meta-data with the same attribute value.
        // Compare their timestamps
        pc_ts = pAVmeta->lberbv.bv_val;
        VALUE_META_TO_NEXT_FIELD(pc_ts, 5);
        rc = strncmp(pc_ts, ps_ts, VMDIR_ORIG_TIME_STR_LEN);
        if (rc > 0)
        {
            // don't delete newer attr-value-meta-data
            continue;
        }
        // Remember those obsolete attr-value-meta-data with that entryid/attr-id/attr-value to be deleted
        dwError = dequePush(&valueMetaDataToDelete, pAVmeta);
        BAIL_ON_VMDIR_ERROR(dwError);
        //valueMetaDataToDelete owns pAVmeta
        pAVmeta = NULL;
    }

    if (!dequeIsEmpty(&valueMetaDataToDelete))
    {
        dwError = VmDirMdbUpdateAttrValueMetaData(pBECtx, entryId, attrId,
                                                 BE_INDEX_OP_TYPE_DELETE, &valueMetaDataToDelete);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VmDirFreeBerval(pAVmeta);
    VmDirFreeAttrValueMetaDataContent(&valueMetaDataToDelete);
    VmDirFreeAttrValueMetaDataContent(&cur_value_meta);
    return dwError;

error:
    VMDIR_LOG_ERROR(LDAP_DEBUG_BACKEND,
             "_VmDirDeleteOldValueMetaData failed: error=%d,eid=%ld,attrId=%d", dwError, entryId, attrId);
    goto cleanup;
}

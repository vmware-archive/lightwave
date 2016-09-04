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
DWORD
MdbScanIndex(
    PVDIR_DB_TXN        pTxn,
    VDIR_BERVALUE *     attrType,
    PVDIR_DB_DBT        pKey,
    VDIR_FILTER *       pFilter,
    int                 maxScanForSizeLimit,
    int *               partialCandidates,
    ENTRYID             eStartingId
    );

/*
 * BdbCheckIfALeafNode(): From parentid.db, check if the entryId has children.
 *
 * Return values:
 *     On Success: 0
 *     On Error: BE error code
 */
DWORD
VmDirMDBCheckIfALeafNode(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID             entryId,
    PBOOLEAN            pIsLeafEntry)
{
    DWORD               dwError = 0;
    VDIR_DB_DBT         key = {0};
    VDIR_DB_DBT         value = {0};
    VDIR_DB             mdbDBi = 0;
    VDIR_BERVALUE       parentIdAttr = { {ATTR_PARENT_ID_LEN, ATTR_PARENT_ID}, 0, 0, NULL };
    unsigned char       EIDBytes[sizeof( ENTRYID )] = {0};
    PVDIR_DB_TXN        pTxn = NULL;
    PVDIR_INDEX_CFG     pIndexCfg = NULL;

    assert(pBECtx && pBECtx->pBEPrivate && pIsLeafEntry);

    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;

    *pIsLeafEntry = FALSE;

    dwError = VmDirIndexCfgAcquire(
            parentIdAttr.lberbv.bv_val, VDIR_INDEX_READ, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);

    key.mv_data = &EIDBytes[0];
    MDBEntryIdToDBT(entryId, &key);

    dwError = mdb_get(pTxn, mdbDBi, &key, &value);
    if (dwError == 0)
    {
        *pIsLeafEntry = FALSE;
    }
    else if (dwError == MDB_NOTFOUND)
    {
        *pIsLeafEntry = TRUE;
        dwError = 0;
    }
    else
    {
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    return dwError;

error:

    VMDIR_LOG_ERROR( LDAP_DEBUG_BACKEND, "MDBCheckIfALeafNode, eId(%u) failed (%d)(%s)",
              entryId, dwError, mdb_strerror(dwError));
    dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx, "LeafNodeCheck");

    goto cleanup;
}

/*
 * BdbGetAttrMetaData(): Get attribute's meta data.
 *
 * Return values:
 *     On Success: 0
 *     On Error: BE error code
 */
DWORD
VmDirMDBGetAttrMetaData(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_ATTRIBUTE *    attr,
    ENTRYID             entryId
    )
{
    DWORD                 dwError = 0;
    VDIR_DB_DBT           key = {0};
    VDIR_DB_DBT           value = {0};
    unsigned char         keyData[ sizeof( ENTRYID ) + 1 + 2 ] = {0}; /* key format is: <entry ID>:<attribute ID (a short)> */
    unsigned char *       pWriter = NULL;
    VDIR_DB               mdbDBi = 0;
    int                   indTypes = 0;
    BOOLEAN               bIsUniqueVal = FALSE;
    VDIR_BERVALUE         attrMetaDataAttr = { {ATTR_ATTR_META_DATA_LEN, ATTR_ATTR_META_DATA}, 0, 0, NULL };

    PVDIR_DB_TXN          pTxn = NULL;
    PVDIR_INDEX_CFG       pIndexCfg = NULL;

    assert( pBECtx && pBECtx->pBEPrivate );

    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;

    dwError = VmDirIndexCfgAcquire(
            attrMetaDataAttr.lberbv.bv_val, VDIR_INDEX_READ, &pIndexCfg);
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

    dwError = mdb_get(pTxn, mdbDBi, &key, &value);
    BAIL_ON_VMDIR_ERROR(dwError);

    assert(value.mv_size <= sizeof(attr->metaData));
    dwError = VmDirCopyMemory(&attr->metaData[0], value.mv_size, value.mv_data, value.mv_size);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    return dwError;

error:

    if (dwError != MDB_NOTFOUND)
    {
        VMDIR_LOG_ERROR( LDAP_DEBUG_REPL_ATTR, "BdbGetAttrMetaData failed with error code: %d, error string: %s",
                    dwError, mdb_strerror(dwError) );
        dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx, "VmDirMDBGetAttrMetaData");
    }
    else
    {
        dwError = ERROR_BACKEND_ATTR_META_DATA_NOTFOUND;
    }

    goto cleanup;
}

/*
 * BdbGetAllAttrsMetaData(): Get attribute's meta data for all the attributes of an entry.
 *
 * Return values:
 *     On Success: 0
 *     On Error: BE error
 */
DWORD
VmDirMDBGetAllAttrsMetaData(
    PVDIR_BACKEND_CTX           pBECtx,
    ENTRYID                     entryId,
    PATTRIBUTE_META_DATA_NODE * ppAttrMetaDataNode,
    int *                       pNumAttrMetaData
    )
{
    DWORD                 dwError = 0;
    VDIR_DB_DBT           key = {0};
    VDIR_DB_DBT           value = {0};
    VDIR_DB_DBT           currKey = {0};
    char                  keyData[ sizeof( ENTRYID ) + 1 + 2 ] = {0}; /* key format is: <entry ID>:<attribute ID (a short)> */
    unsigned char *       pReader = NULL;
    VDIR_DB               mdbDBi = 0;
    int                   indTypes = 0;
    BOOLEAN               bIsUniqueVal = FALSE;
    VDIR_BERVALUE         attrMetaDataAttr = { {ATTR_ATTR_META_DATA_LEN, ATTR_ATTR_META_DATA}, 0, 0, NULL };
    PVDIR_DB_DBC          pCursor = NULL;
    unsigned int          cursorFlags;

    PVDIR_DB_TXN                pTxn = NULL;
    PVDIR_INDEX_CFG             pIndexCfg = NULL;
    PATTRIBUTE_META_DATA_NODE   pDataNode = NULL;
    int                         iNumNode = 0;
    int                         iMaxNumNode = 0;

    assert( pBECtx && pBECtx->pBEPrivate );

    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;

    dwError = VmDirIndexCfgAcquire(
            attrMetaDataAttr.lberbv.bv_val, VDIR_INDEX_READ, &pIndexCfg);
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

    dwError = mdb_cursor_open(pTxn, mdbDBi, &pCursor);
    BAIL_ON_VMDIR_ERROR(dwError);

    memset(&currKey, 0, sizeof(currKey));
    currKey.mv_size = key.mv_size;
    currKey.mv_data = key.mv_data;

    cursorFlags = MDB_SET_RANGE;

    iMaxNumNode = BE_DB_META_NODE_INIT_SIZE;
    if (VmDirAllocateMemory( iMaxNumNode * sizeof( ATTRIBUTE_META_DATA_NODE ),
                             (PVOID *)&pDataNode ) != 0)
    {
        dwError = ERROR_BACKEND_OPERATIONS;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    do
    {
        if ((dwError = mdb_cursor_get(pCursor, &currKey, &value, cursorFlags )) != 0)
        {
            if (dwError == MDB_NOTFOUND)
            {
                dwError = 0;
            }
            else
            {
                BAIL_ON_VMDIR_ERROR( dwError );
            }
            break;
        }

        if (memcmp(key.mv_data, currKey.mv_data, key.mv_size) != 0)
        {
            break;
        }

        // set metaData
        assert(value.mv_size <= sizeof(pDataNode->metaData));
        dwError = VmDirCopyMemory(   &(pDataNode[iNumNode].metaData[0]),
                                    value.mv_size, value.mv_data, value.mv_size);
        BAIL_ON_VMDIR_ERROR(dwError);

        // set attrID
        pReader = &((unsigned char *)currKey.mv_data)[key.mv_size];
        pDataNode[iNumNode].attrID = VmDirDecodeShort( &pReader );

        iNumNode++;
        if (iNumNode == iMaxNumNode)
        {
            iMaxNumNode += BE_DB_META_NODE_INC_SIZE;
            if (VmDirReallocateMemory(  (PVOID)pDataNode,
                                        (PVOID *)&pDataNode,
                                        iMaxNumNode * sizeof( ATTRIBUTE_META_DATA_NODE )) != 0)
            {
                dwError = ERROR_BACKEND_OPERATIONS;
                BAIL_ON_VMDIR_ERROR( dwError );
            }
        }

        cursorFlags = MDB_NEXT;

    } while (TRUE);

    *ppAttrMetaDataNode = pDataNode;
    *pNumAttrMetaData = iNumNode;

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    if (pCursor)
    {
        mdb_cursor_close( pCursor );
    }

    return dwError;

error:

    ppAttrMetaDataNode = NULL;
    pNumAttrMetaData = 0;

    VMDIR_SAFE_FREE_MEMORY(pDataNode);

    VMDIR_LOG_ERROR( LDAP_DEBUG_REPL_ATTR, "MdbGetAllAttrsMetaData: error (%d),(%s)",
              dwError, mdb_strerror(dwError) );

    dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx, "GetAllAttrsMetaData");

    goto cleanup;
}

/* BdbGetCandidates: Get candidates for individual filter components where filter attribute is indexed.
 *
 * Return: BE error
 */
DWORD
VmDirMDBGetCandidates(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_FILTER*        pFilter,
    ENTRYID             eStartingId
    )
{
    DWORD           dwError = 0;
    VDIR_DB_DBT     key = {0};
    PSTR            pszkeyData = NULL;
    ENTRYID         parentId = 0;
    PVDIR_DB_TXN    pTxn = NULL;

    assert(pBECtx && pBECtx->pBEPrivate && pFilter);

    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;

    switch ( pFilter->choice )
    {
        case LDAP_FILTER_EQUALITY:
        case LDAP_FILTER_GE:
        case LDAP_FILTER_LE:
        {
            char *    normVal    = BERVAL_NORM_VAL(pFilter->filtComp.ava.value);
            ber_len_t normValLen = BERVAL_NORM_LEN(pFilter->filtComp.ava.value);

            VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, (pFilter->choice == LDAP_FILTER_EQUALITY) ?
                                                    "LDAP_FILTER_EQUALITY" :"LDAP_FILTER_GE" );

            dwError = VmDirAllocateMemory( normValLen + 1, (PVOID *)&pszkeyData );
            BAIL_ON_VMDIR_ERROR(dwError);

            key.mv_data = pszkeyData;
            *(char *)(key.mv_data) = BE_INDEX_KEY_TYPE_FWD; //TODO, do we need this?
            memcpy(((char *)key.mv_data + 1), normVal, normValLen);
            dwError = VmDirCopyMemory(((char*)key.mv_data + 1), normValLen, normVal, normValLen);
            BAIL_ON_VMDIR_ERROR(dwError);

            key.mv_size = normValLen + 1;

            dwError = MdbScanIndex(pTxn, &(pFilter->filtComp.ava.type), &key, pFilter, pBECtx->iMaxScanForSizeLimit, &pBECtx->iPartialCandidates, eStartingId);
            if ( pFilter->candidates )
            {
                VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "scan %s, result set size (%d), bad filter (%d)",
                                   VDIR_SAFE_STRING(pFilter->filtComp.ava.type.lberbv.bv_val),
                                   pFilter->candidates->size,
                                   (pFilter->iMaxIndexScan && pFilter->candidates->size > pFilter->iMaxIndexScan) ? 1:0 );
            }

            BAIL_ON_VMDIR_ERROR(dwError);
            break;
        }
        case LDAP_FILTER_SUBSTRINGS:
        {
            char *    normVal = NULL;
            ber_len_t normValLen = 0;

            VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "LDAP_FILTER_SUBSTRINGS" );

            // SJ-TBD: It can be both and INITIAL and FINAL instead of one or the other.
            if (pFilter->filtComp.subStrings.initial.lberbv.bv_len != 0)
            {
                normVal    = BERVAL_NORM_VAL(pFilter->filtComp.subStrings.initial);
                normValLen = BERVAL_NORM_LEN(pFilter->filtComp.subStrings.initial);

                dwError = VmDirAllocateMemory( normValLen + 1, (PVOID *)&pszkeyData );
                BAIL_ON_VMDIR_ERROR(dwError);

                key.mv_data = pszkeyData;
                *(char *)(key.mv_data) = BE_INDEX_KEY_TYPE_FWD;
                dwError = VmDirCopyMemory(((char*)key.mv_data + 1), normValLen, normVal, normValLen);
                BAIL_ON_VMDIR_ERROR(dwError);

                key.mv_size = normValLen + 1;
            }
            else if (pFilter->filtComp.subStrings.final.lberbv.bv_len != 0)
            {
                ber_len_t       j = 0;
                ber_len_t       k = 0;

                normVal    = BERVAL_NORM_VAL(pFilter->filtComp.subStrings.final);
                normValLen = BERVAL_NORM_LEN(pFilter->filtComp.subStrings.final);

                dwError = VmDirAllocateMemory( normValLen + 1, (PVOID *)&pszkeyData );
                BAIL_ON_VMDIR_ERROR(dwError);

                key.mv_data = pszkeyData;
                *(char *)(key.mv_data) = BE_INDEX_KEY_TYPE_REV;
                // Reverse copy from f->filtComp.subStrings.final.lberbv.bv_val to &(key.mv_data[1])
                for (j=normValLen, k=1; j > 0; j--, k++)
                {
                    *((char *)key.mv_data + k) = normVal[j-1];
                }

                key.mv_size = normValLen + 1;
            }
            else if (pFilter->filtComp.subStrings.anySize > 0)
            {
                // "any" indexing has not being implemented yet, and bypass index lookup
                break;
            } else
            {
                assert( FALSE );
            }

            dwError = MdbScanIndex(pTxn, &(pFilter->filtComp.subStrings.type), &key, pFilter, pBECtx->iMaxScanForSizeLimit, &pBECtx->iPartialCandidates, eStartingId);
            if ( pFilter->candidates )
            {
                VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "scan %s, result set size (%d), bad filter (%d)",
                                   VDIR_SAFE_STRING(pFilter->filtComp.subStrings.type.lberbv.bv_val),
                                   pFilter->candidates->size,
                                   (pFilter->iMaxIndexScan && pFilter->candidates->size > pFilter->iMaxIndexScan) ? 1:0 );
            }
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
        }

        case FILTER_ONE_LEVEL_SEARCH:
        {
            VDIR_BERVALUE   parentIdAttr = { {ATTR_PARENT_ID_LEN, ATTR_PARENT_ID}, 0, 0, NULL };
            unsigned char   parentEIdBytes[sizeof( VDIR_DB_SEQ_T )] = {0};

            VMDIR_LOG_INFO( LDAP_DEBUG_FILTER, "LDAP_FILTER_ONE_LEVEL_SRCH" );

            dwError = VmDirMDBDNToEntryId( pBECtx, &(pFilter->filtComp.parentDn), &parentId );
            BAIL_ON_VMDIR_ERROR(dwError);

            key.mv_data = &parentEIdBytes[0];
            MDBEntryIdToDBT(parentId, &key);

            dwError = MdbScanIndex(pTxn, &(parentIdAttr), &key, pFilter, pBECtx->iMaxScanForSizeLimit, &pBECtx->iPartialCandidates, eStartingId);
            if ( pFilter->candidates )
            {
                VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "scan %s, result set size (%d), bad filter (%d)",
                                   VDIR_SAFE_STRING(parentIdAttr.lberbv.bv_val),
                                   pFilter->candidates->size,
                                   (pFilter->iMaxIndexScan && pFilter->candidates->size > pFilter->iMaxIndexScan) ? 1:0 );
            }
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
        }

        default:
            assert( FALSE );
            break;
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszkeyData );

    return dwError;

error:

    if (dwError == MDB_NOTFOUND)
    {
        dwError = ERROR_BACKEND_ENTRY_NOTFOUND;
    }
    else
    {
        dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx, "GetCandidates");
    }

    goto cleanup;
}


/*
 * BdbDNToEntryId(). Given a DN, get the entryId from DN DB.
 *
 * Return values:
 *     On Success: 0
 *     On Error: BE error - BACKEND_ERROR, BACKEND_ENTRY_NOTFOUND
 */
DWORD
VmDirMDBDNToEntryId(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE*      pDn,
    ENTRYID*            pEId)
{
    DWORD                 dwError = 0;
    VDIR_BERVALUE         dnAttr = { {ATTR_DN_LEN, ATTR_DN}, 0, 0, NULL };
    VDIR_DB_DBT           key = {0};
    char *                pKeyData = NULL;
    VDIR_DB_DBT           value = {0};
    VDIR_DB               mdbDBi = 0;
    char *                normDn = NULL;
    ber_len_t             normDnLen = 0;
    VDIR_DB_TXN*          pTxn = NULL;
    PVDIR_INDEX_CFG       pIndexCfg = NULL;
    PSTR                  pszLocalErrMsg = NULL;

    assert(pBECtx && pBECtx->pBEPrivate && pDn && pEId);

    normDn    = BERVAL_NORM_VAL(*pDn);
    normDnLen = BERVAL_NORM_LEN(*pDn);

    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;

    if (normDnLen == 0)
    {
        *pEId = DSE_ROOT_ENTRY_ID;
    }
    else
    {
        dwError = VmDirIndexCfgAcquire(
                dnAttr.lberbv.bv_val, VDIR_INDEX_READ, &pIndexCfg);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateMemory( normDnLen + 1, (PVOID *)&pKeyData );
        BAIL_ON_VMDIR_ERROR(dwError);

        key.mv_data = pKeyData;
        *(char *)(key.mv_data) = BE_INDEX_KEY_TYPE_FWD;
        dwError = VmDirCopyMemory(((char *)key.mv_data + 1), normDnLen, normDn, normDnLen);
        BAIL_ON_VMDIR_ERROR(dwError);

        key.mv_size = normDnLen + 1;

        if ((dwError = mdb_get(pTxn, mdbDBi, &key, &value)) != 0)
        {
            DWORD   dwTmp = dwError;
            dwError = MDBToBackendError(dwError, MDB_NOTFOUND, ERROR_BACKEND_ENTRY_NOTFOUND, pBECtx,
                                        VDIR_SAFE_STRING(normDn));
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                            "BdbDNToEntryId: failed for Dn: %s, with error code: %d, error string: %s",
                            pDn->lberbv.bv_val, dwTmp, VDIR_SAFE_STRING(pBECtx->pszBEErrorMsg) );
        }

        MDBDBTToEntryId( &value, pEId);
    }

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    VMDIR_SAFE_FREE_MEMORY( pKeyData );
    VMDIR_SAFE_FREE_MEMORY( pszLocalErrMsg );

    return dwError;

error:

    VMDIR_LOG_ERROR( LDAP_DEBUG_BACKEND, VDIR_SAFE_STRING(pszLocalErrMsg) );

    VMDIR_SET_BACKEND_ERROR(dwError);

    goto cleanup;
}

/*
 * BdbObjectGUIDToEntryId(). Given an ObjectGUID, get the entryId from DN DB.
 *
 * Return values:
 *     On Success: 0
 *     On Error: BE error - BACKEND_ERROR, BACKEND_ENTRY_NOTFOUND
 */
DWORD
VmDirMDBObjectGUIDToEntryId(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszObjectGUID,
    ENTRYID*            pEId)
{
    DWORD                 dwError = 0;
    VDIR_BERVALUE         guidAttr = { {ATTR_OBJECT_GUID_LEN, ATTR_OBJECT_GUID}, 0, 0, NULL };
    VDIR_DB_DBT           key = {0};
    char *                pKeyData = NULL;
    VDIR_DB_DBT           value = {0};
    VDIR_DB               mdbDBi = 0;
    ber_len_t             Len = 0;
    VDIR_DB_TXN*          pTxn = NULL;
    PVDIR_INDEX_CFG       pIndexCfg = NULL;
    PSTR                  pszLocalErrMsg = NULL;

    assert(pBECtx && pBECtx->pBEPrivate && pszObjectGUID && pEId);

    Len = VmDirStringLenA(pszObjectGUID);

    pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;

    dwError = VmDirIndexCfgAcquire(
            guidAttr.lberbv.bv_val, VDIR_INDEX_READ, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory( Len + 1, (PVOID *)&pKeyData );
    BAIL_ON_VMDIR_ERROR(dwError);

    key.mv_data = pKeyData;
    *(char *)(key.mv_data) = BE_INDEX_KEY_TYPE_FWD;
    dwError = VmDirCopyMemory(((char *)key.mv_data + 1), Len, pszObjectGUID, Len);
    BAIL_ON_VMDIR_ERROR(dwError);

    key.mv_size = Len + 1;

    if ((dwError = mdb_get(pTxn, mdbDBi, &key, &value)) != 0)
    {
        DWORD   dwTmp = dwError;
        dwError = MDBToBackendError(dwError, MDB_NOTFOUND, ERROR_BACKEND_ENTRY_NOTFOUND, pBECtx,
                                    VDIR_SAFE_STRING(pszObjectGUID));
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                        "BdbObjectGUIDToEntryId: failed for ObjectGUID: %s, with error code: %d, error string: %s",
                        VDIR_SAFE_STRING(pszObjectGUID), dwTmp, VDIR_SAFE_STRING(pBECtx->pszBEErrorMsg) );
    }

    MDBDBTToEntryId( &value, pEId);

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    VMDIR_SAFE_FREE_MEMORY(pKeyData);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return dwError;

error:

    VMDIR_LOG_ERROR( LDAP_DEBUG_BACKEND, VDIR_SAFE_STRING(pszLocalErrMsg) );

    VMDIR_SET_BACKEND_ERROR(dwError);
    goto cleanup;
}

/* MDBEntryIdToDBT: Convert EntryId (db_seq_t/long long) to sequence of bytes (going from high order bytes to lower
 * order bytes) to be stored in BDB.
 *
 * Motivation: So that BDB can store EntryId data values in a sorted way (with DB_DUPSORT flag set for the DB) using
 * its default sorting scheme ("If no comparison function is specified, the data items are compared lexically, with
 * shorter data items collating before longer data items.")
 */
void
MDBEntryIdToDBT(
    ENTRYID         eId,
    PVDIR_DB_DBT    pDBT)
{
    ENTRYID     tmpEId = eId;
    size_t         i = 0;

    pDBT->mv_size = BE_REAL_EID_SIZE(eId);

    for (i = pDBT->mv_size , tmpEId = eId; i > 0; i-- )
    {
        ((unsigned char *)pDBT->mv_data)[i-1] = (unsigned char) tmpEId;
        tmpEId >>= 8;
    }
}

/* DBTToEntryId: Convert DBT data bytes sequence to EntryId (db_seq_t/long long).
 *
 */
void
MDBDBTToEntryId(
    PVDIR_DB_DBT    pDBT,
    ENTRYID*        pEID)
{
    int      i = 0;

    *pEID = 0;
    for (i = 0; i < pDBT->mv_size; i++ )
    {
        *pEID <<= 8;
        *pEID |= (unsigned char) ((unsigned char *)(pDBT->mv_data))[i];
    }
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
 *     On Error: MDB error
 */
static
DWORD
MdbScanIndex(
    PVDIR_DB_TXN        pTxn,
    VDIR_BERVALUE *     attrType,
    PVDIR_DB_DBT        pKey,
    VDIR_FILTER *       pFilter,
    int                 maxScanForSizeLimit,
    int *               partialCandidates,
    ENTRYID             eStartingId
    )
{
    DWORD               dwError = 0;
    VDIR_DB             mdbDBi = 0;
    BOOLEAN             bIsUniqueVal = FALSE;
    VDIR_DB_DBT         value = {0};
    VDIR_DB_DBT         currKey = {0};
    ENTRYID             eId = 0;
    PVDIR_DB_TXN        pLocalTxn = NULL;
    PVDIR_INDEX_CFG     pIndexCfg = NULL;
    PVDIR_DB_DBC        pCursor = NULL;
    unsigned int        cursorFlags;

    // GE filter is neither exactMatch nor a partialMatch
    BOOLEAN     bIsExactMatch = (pFilter->choice == LDAP_FILTER_EQUALITY || pFilter->choice == FILTER_ONE_LEVEL_SEARCH);
    BOOLEAN     bIsPartialMatch = (pFilter->choice == LDAP_FILTER_SUBSTRINGS);

    dwError = VmDirIndexCfgAcquire(
            attrType->lberbv.bv_val, VDIR_INDEX_READ, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pIndexCfg)
    {
        VMDIR_LOG_VERBOSE( LDAP_DEBUG_BACKEND, "ScanIndex: non-indexed attribute. attrType = %s", attrType->lberbv.bv_val);
        goto cleanup;
    }

    if ( pFilter->candidates )
    {
        DeleteCandidates( &(pFilter->candidates) );
    }
    pFilter->candidates = NewCandidates(BE_CANDIDATES_START_ALLOC_SIZE, TRUE);
    if (! pFilter->candidates)
    {
        dwError = ERROR_BACKEND_ERROR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);

    bIsUniqueVal = pIndexCfg->bGlobalUniq;

    if (pTxn == NULL)
    {
        dwError = mdb_txn_begin(    gVdirMdbGlobals.mdbEnv,
                                    BE_DB_PARENT_TXN_NULL,
                                    MDB_RDONLY,
                                    &pLocalTxn);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        pLocalTxn = pTxn;
    }

    if ( bIsExactMatch && bIsUniqueVal )
    {
        dwError = mdb_get( pLocalTxn, mdbDBi, pKey, &value);
        BAIL_ON_VMDIR_ERROR(dwError);

        MDBDBTToEntryId( &value, &eId);
        if (eId >= eStartingId)
        {
            dwError = VmDirAddToCandidates(pFilter->candidates, eId);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        dwError = mdb_cursor_open(pLocalTxn, mdbDBi, &pCursor);
        BAIL_ON_VMDIR_ERROR(dwError);

        memset(&currKey, 0, sizeof(currKey));
        currKey.mv_size = pKey->mv_size;
        currKey.mv_data = pKey->mv_data;

        cursorFlags = bIsExactMatch ? MDB_SET : MDB_SET_RANGE;

        if (pFilter->choice == LDAP_FILTER_LE                                   &&
            mdb_cursor_get(pCursor, &currKey, &value, cursorFlags) == MDB_NOTFOUND)
        {
            // Key value too big, set to last record instead.
            cursorFlags = MDB_LAST;
        }

        do
        {
            if ((dwError = mdb_cursor_get(pCursor, &currKey, &value, cursorFlags )) != 0)
            {
                if (dwError == MDB_NOTFOUND)
                {
                    if (pFilter->candidates->size > 0) // We had found something
                    {
                        dwError = 0;
                    }
                }
                else
                {
                    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "ScanIndex: cursor->get(DB_SET) failed with error code: %d, "
                              "error string: %s", dwError, mdb_strerror(dwError) );
                    BAIL_ON_VMDIR_ERROR( dwError );
                }

                break;  // break loop if no more cursor data
            }

            // In case of exact match, check if we passed the key that we are looking for
            if (bIsExactMatch)
            {
                if (pKey->mv_size != currKey.mv_size ||
                    memcmp(pKey->mv_data, currKey.mv_data, pKey->mv_size))
                {
                    // Note: this check is normally not necessary, but for some edge cases
                    // where mdb_cursor_get does not return MDB_NOTFOUND
                    break;
                }
            }

            // In case of partial match, check if we are passed the partial key match that we are looking for.
            if (bIsPartialMatch && memcmp(pKey->mv_data, currKey.mv_data, pKey->mv_size) != 0)
            {
                break;
            }

            MDBDBTToEntryId( &value, &eId);
            if (eId >= eStartingId)
            {
                dwError = VmDirAddToCandidates(pFilter->candidates, eId);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            if ( pFilter->iMaxIndexScan > 0 &&
                 pFilter->candidates->size > pFilter->iMaxIndexScan
               )
            {
                // Exceed max scan size, treats as data not found. BuildCandidateList logic will retry w/o limit.
                // When candidates is set to NULL, AndFilterResults() will treat it as a non-indexed attribute.
                DeleteCandidates( &(pFilter->candidates) );
                dwError = MDB_NOTFOUND;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            if (maxScanForSizeLimit > 0 && pFilter->candidates->size > maxScanForSizeLimit)
            {
                *partialCandidates = 1;
                break;
            }

            eId = 0;
            cursorFlags = bIsExactMatch ? MDB_NEXT_DUP :
                                          (pFilter->choice == LDAP_FILTER_LE) ? MDB_PREV : MDB_NEXT;
        }
        while (TRUE);
    }

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);

    if (pCursor != NULL)
    {
        mdb_cursor_close(pCursor);
    }

    if (pTxn == NULL && pLocalTxn != NULL) /* commit/abort local transaction */
    {
        if (dwError == 0 || dwError == MDB_NOTFOUND)
        {
            mdb_txn_commit(pLocalTxn);
        }
        else
        {
            mdb_txn_abort(pLocalTxn);
        }
    }

    VMDIR_LOG_VERBOSE( LDAP_DEBUG_BACKEND, "ScanIndex: retVal = %d, #of candidates = %d",
              dwError, pFilter->candidates == NULL ? 0 : pFilter->candidates->size );

    return dwError;

error:

    if (dwError != MDB_NOTFOUND)
    {
        DeleteCandidates( &(pFilter->candidates) );

        VMDIR_LOG_ERROR( LDAP_DEBUG_BACKEND, "ScanIndex failed with error code: %d, error string: %s",
                  dwError, mdb_strerror(dwError) );
    }

    goto cleanup;
}

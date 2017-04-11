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
_DeleteOldNormalizedDNFromIndex(
    PVDIR_ENTRY_ARRAY   pEntryArray,
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PVDIR_DB_TXN        pTxn
    )
{

    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;
    PVDIR_ENTRY     pEntry = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;

    if (!pEntryArray || !pSchemaCtx || !pTxn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaAttrNameToDescriptor(
            pSchemaCtx, ATTR_LABELED_URI, &pATDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    // set labeledURI's matching rules to case exact (IA5 String)
    dwError = VmDirSchemaATDescOverrideMR(
            pATDesc, "1.3.6.1.4.1.1466.115.121.1.26");
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pEntryArray->iSize; i++)
    {
        pEntry = &pEntryArray->pEntry[i];
        pAttr = VmDirFindAttrByName(pEntry, ATTR_DN);

        dwError = pAttr == NULL ? VMDIR_ERROR_INVALID_ENTRY : 0;
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirNormalizeDN(&pAttr->vals[0], pSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = MdbUpdateIndicesForAttr(
                pTxn,
                &pEntry->dn,
                &pAttr->type,
                pAttr->vals,
                pAttr->numVals,
                pEntry->eId,
                BE_INDEX_OP_TYPE_DELETE);

        dwError = dwError == MDB_NOTFOUND ? 0 : dwError;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    if (pATDesc)
    {
        // revert labeledURI's matching rules to original
        (VOID)VmDirSchemaATDescOverrideMR(pATDesc, pATDesc->pszSyntaxOid);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);

    goto cleanup;
}

static
DWORD
_WriteNewNormalizedDNToIndex(
    PVDIR_ENTRY_ARRAY   pEntryArray,
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PVDIR_DB_TXN        pTxn
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_ENTRY     pEntry = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;

    if (!pEntryArray || !pSchemaCtx || !pTxn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i = 0; i < pEntryArray->iSize; i++)
    {
        pEntry = &pEntryArray->pEntry[i];
        pAttr = VmDirFindAttrByName(pEntry, ATTR_DN);

        dwError = pAttr == NULL ? VMDIR_ERROR_INVALID_ENTRY : 0;
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pAttr->vals[0].bvnorm_val);
        pAttr->vals[0].bvnorm_len = 0;

        dwError = VmDirNormalizeDN(&pAttr->vals[0], pSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = MdbUpdateIndicesForAttr(
                pTxn,
                &pEntry->dn,
                &pAttr->type,
                pAttr->vals,
                pAttr->numVals,
                pEntry->eId,
                BE_INDEX_OP_TYPE_CREATE);

        dwError = dwError == MDB_KEYEXIST ? 0 : dwError;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);

    goto cleanup;
}

/*
 * Apply new matching rules to indices during schema upgrade
 * - Update index for dn of replication agreements
 */
DWORD
VmDirMdbApplyIndicesNewMR(
    VOID
    )
{
    DWORD   dwError = 0;
    PSTR    pszSearchBaseDN = NULL;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    PVDIR_DB_TXN    pTxn = NULL;

    dwError = VmDirAllocateStringPrintf(
            &pszSearchBaseDN,
            "cn=Sites,cn=Configuration,%s",
            gVmdirServerGlobals.systemDomainDN.bvnorm_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    // search replication agreement entries
    dwError = VmDirSimpleEqualFilterInternalSearch(
            pszSearchBaseDN,
            LDAP_SCOPE_SUBTREE,
            ATTR_OBJECT_CLASS,
            OC_REPLICATION_AGREEMENT,
            &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_txn_begin(
            gVdirMdbGlobals.mdbEnv,
            BE_DB_PARENT_TXN_NULL,
            BE_DB_FLAGS_ZERO,
            &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _DeleteOldNormalizedDNFromIndex(&entryArray, pSchemaCtx, pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _WriteNewNormalizedDNToIndex(&entryArray, pSchemaCtx, pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_txn_commit(pTxn);
    pTxn = NULL;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    VMDIR_SAFE_FREE_MEMORY(pszSearchBaseDN);
    VmDirSchemaCtxRelease(pSchemaCtx);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);

    mdb_txn_abort(pTxn);
    goto cleanup;
}

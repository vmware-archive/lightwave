/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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
 * Module Name: db cross checking Thread
 *
 */

#include "includes.h"

static
DWORD
_VmDirDBCrossChkThreadFun(
    PVOID pArg
    );

static
DWORD
_VmDirDBCrossChkBatchEntry(
    PVDIR_BACKEND_INTERFACE pBE,
    ENTRYID*    eIDArray,
    DWORD       dwEIDCnt
    );

static
VOID
_VmDirDBCrossChkDNRFC4514(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_BACKEND_CTX       pBECtx,
    PVDIR_ENTRY             pEntry
    );

static
VOID
_VmDirDBCrossChkMemberRFC4514(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_BACKEND_CTX       pBECtx,
    PVDIR_ENTRY             pEntry
    );

static
VOID
_VmDirDBCrossChkAttr(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_BACKEND_CTX       pBECtx,
    PVDIR_ENTRY             pEntry
    );

static
VOID
_VmDirDBCrossChkAttrVal(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_BACKEND_CTX       pBECtx,
    PVDIR_ENTRY             pEntry,
    PVDIR_ATTRIBUTE         pAttr,
    PVDIR_BERVALUE          pBVValue
    );

DWORD
VmDirInitDBCrossChkThread(
    VOID
    )
{
    DWORD dwError = 0;
    PVDIR_THREAD_INFO pThrInfo = NULL;

    dwError = VmDirSrvThrInit(&pThrInfo, NULL, NULL, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
                &pThrInfo->tid,
                pThrInfo->bJoinThr,
                _VmDirDBCrossChkThreadFun,
                pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error %d", __FUNCTION__, dwError);
    VmDirSrvThrFree(pThrInfo);
    goto cleanup;
}

/*
 * specific for RFC4514 compliance transition.
 * to report RFC4514 syntax and normalize issue in current db.
 */
static
VOID
_VmDirDBCrossChkDNRFC4514(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_BACKEND_CTX       pBECtx,
    PVDIR_ENTRY             pEntry
    )
{
    DWORD   dwError = 0;
    VDIR_ENTRY  rfc4514Entry = {0};
    VDIR_ENTRY  parentEntry = {0};

    /*
     * validate current DN value in BLOB compliance with RFC 4514
     */
    pEntry->ldapDN.dn.lberbv_val = pEntry->dn.lberbv_val;
    pEntry->ldapDN.dn.lberbv_len = pEntry->dn.lberbv_len;
    dwError = VmDirNormDN(&pEntry->ldapDN, pEntry->pSchemaCtx);
    if (dwError)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
            "%s str2dn error (%d) (%s) RFC4514 syntax violation",
            __FUNCTION__,
            dwError,
            pEntry->ldapDN.dn.lberbv_val);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    /*
     * DN table lookup with RFC 4514 normalized value should succeed
     */
    dwError = pBE->pfnBEDNToEntry(
        pBECtx, pEntry->pSchemaCtx, &pEntry->ldapDN.dn, &rfc4514Entry, VDIR_BACKEND_ENTRY_LOCK_READ);
    if (dwError)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s pfnBEDNToEntry not found (%d)",
                __FUNCTION__, dwError);
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s DN norm issue original value (%s)",
                __FUNCTION__, pEntry->dn.lberbv_val);
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s DN norm issue 4514 norm (%s)",
                __FUNCTION__, pEntry->ldapDN.dn.bvnorm_val);

        if (VmDirNormalizeDN(&pEntry->dn, pEntry->pSchemaCtx) == 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s DN norm issue curr norm (%s)",
                    __FUNCTION__, pEntry->dn.bvnorm_val);
        }
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    if (rfc4514Entry.eId != pEntry->eId)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
            "%s lookup has different entryid (%s)(%llu)(%s)(%llu)",
            __FUNCTION__,
            pEntry->dn.lberbv_val,
            pEntry->eId,
            rfc4514Entry.dn.lberbv_val,
            rfc4514Entry.eId);

        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    /*
     * validate ldapDN.pszParentNormDN lookup
     */
    if (pEntry->ldapDN.pszParentNormDN && pEntry->ldapDN.pszParentNormDN[0] != '\0')
    {
        VDIR_BERVALUE parentBV = {0};

        parentBV.lberbv_val = (PSTR)pEntry->ldapDN.pszParentNormDN;
        parentBV.lberbv_len = VmDirStringLenA(pEntry->ldapDN.pszParentNormDN);
        parentBV.bvnorm_val = parentBV.lberbv_val;
        parentBV.bvnorm_len = parentBV.lberbv_len;

        dwError = pBE->pfnBEDNToEntry(
            pBECtx, pEntry->pSchemaCtx, &parentBV, &parentEntry, VDIR_BACKEND_ENTRY_LOCK_READ);
        if (dwError)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s DN norm parent pfnBEDNToEntry failed (%d)",
                    __FUNCTION__, dwError);
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s DN norm parent (%s)(%s)",
                    __FUNCTION__,
                    pEntry->ldapDN.dn.lberbv_val,
                    pEntry->ldapDN.pszParentNormDN);
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        parentEntry.ldapDN.dn.lberbv_val = parentEntry.dn.lberbv_val;
        parentEntry.ldapDN.dn.lberbv_len = parentEntry.dn.lberbv_len;
        dwError = VmDirNormDN(&parentEntry.ldapDN, parentEntry.pSchemaCtx);
        if (dwError)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s DN norm parent entry DN failed (%d)",
                    __FUNCTION__, dwError);
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s DN norm parent entry (%s)",
                    __FUNCTION__,
                    parentEntry.ldapDN.dn.lberbv_val);
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        if (VmDirStringEndsWith(pEntry->ldapDN.dn.bvnorm_val, parentEntry.ldapDN.dn.bvnorm_val, TRUE) == FALSE)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s DN norm not end with parent (%s)(%s)",
                    __FUNCTION__,
                    pEntry->ldapDN.dn.bvnorm_val,
                    parentEntry.ldapDN.dn.bvnorm_val);
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
        }
    }

cleanup:
    VmDirFreeEntryContent(&rfc4514Entry);
    VmDirFreeEntryContent(&parentEntry);

    return;

error:
    goto cleanup;
}

/*
 * specific for RFC4514 compliance transition.
 * to report RFC4514 syntax and normalize issue in member table.
 */
static
VOID
_VmDirDBCrossChkMemberRFC4514(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_BACKEND_CTX       pBECtx,
    PVDIR_ENTRY             pEntry
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVDIR_ATTRIBUTE  pAttr = NULL;
    VDIR_BERVALUE   bvDN = {0};
    VDIR_LDAP_DN    ldapDN = {0};

    pAttr = VmDirFindAttrByName(pEntry, ATTR_MEMBER);
    if (!pAttr)
    {
        goto cleanup;
    }

    for (dwCnt=0; dwCnt < pAttr->numVals; dwCnt++)
    {
        VmDirFreeBervalContent(&bvDN);
        VmDirFreeLDAPDNContent(&ldapDN);

        bvDN.lberbv_val = pAttr->vals[dwCnt].lberbv_val;
        bvDN.lberbv_len = pAttr->vals[dwCnt].lberbv_len;
        dwError = VmDirNormalizeDN(&bvDN, pEntry->pSchemaCtx);
        if (dwError)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s Member norm failed (%d)(%s)(%s)",
                    __FUNCTION__,
                    dwError,
                    pEntry->dn.lberbv_val,
                    bvDN.lberbv_val);
            dwError = 0;
            continue;
        }

        ldapDN.dn.lberbv_val = pAttr->vals[dwCnt].lberbv_val;
        ldapDN.dn.lberbv_len = pAttr->vals[dwCnt].lberbv_len;
        dwError = VmDirNormDN(&ldapDN, pEntry->pSchemaCtx);
        if (dwError)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s Member RFC4514 norm failed (%d)(%s)(%s)",
                    __FUNCTION__,
                    dwError,
                    pEntry->dn.lberbv_val,
                    ldapDN.dn.lberbv_val);
            dwError = 0;
            continue;
        }

        if (VmDirStringCompareA(bvDN.bvnorm_val, ldapDN.dn.bvnorm_val, TRUE) !=0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s Member RFC4514 norm value mismatch (%s)(%s)(%s)",
                    __FUNCTION__,
                    pEntry->dn.lberbv_val,
                    bvDN.bvnorm_val,
                    ldapDN.dn.bvnorm_val);
            dwError = 0;
            continue;
        }
    }

cleanup:
    VmDirFreeBervalContent(&bvDN);
    VmDirFreeLDAPDNContent(&ldapDN);
    return;
}

/*
 * validate attribute normalized value index lookup - should match its entry EID.
 */
static
VOID
_VmDirDBCrossChkAttrVal(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_BACKEND_CTX       pBECtx,
    PVDIR_ENTRY             pEntry,
    PVDIR_ATTRIBUTE         pAttr,
    PVDIR_BERVALUE          pBVValue
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    VDIR_BERVALUE   bvKey = {0};

    for (dwCnt=0; dwCnt < pAttr->numVals; dwCnt++)
    {
        VmDirFreeBervalContent(&bvKey);

        bvKey.lberbv_val = pAttr->vals[dwCnt].lberbv_val;
        bvKey.lberbv_len = pAttr->vals[dwCnt].lberbv_len;
        dwError = VmDirSchemaBervalNormalize(
            pEntry->pSchemaCtx,
            pAttr->pATDesc,
            &bvKey);
        if (dwError)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s %d norm failed (%s)(%s)(%s)",
                    __FUNCTION__,
                    dwError,
                    pAttr->type.lberbv_val,
                    pEntry->dn.lberbv_val,
                    bvKey.lberbv_val);
            dwError = 0;
            continue;
        }

        dwError = pBE->pfnBEIndexTableRead(
            pBECtx,
            VDIR_BACKEND_KEY_ORDER_FORWARD,
            pAttr->type.lberbv_val,
            &bvKey,
            pBVValue);
        if (dwError)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s %d table read failed (%s)(%s)(%s)",
                    __FUNCTION__,
                    dwError,
                    pAttr->type.lberbv_val,
                    pEntry->dn.lberbv_val,
                    bvKey.lberbv_val);
            dwError = 0;
            continue;
        }
    }

    VmDirFreeBervalContent(&bvKey);
    return;
}


/*
 * validate index table integrity
 */
static
VOID
_VmDirDBCrossChkAttr(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_BACKEND_CTX       pBECtx,
    PVDIR_ENTRY             pEntry
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVDIR_ATTRIBUTE  pAttr = NULL;
    VDIR_BERVALUE   bvValue = {0};
    unsigned char   eIdBytes[sizeof(ENTRYID)] = {0};
    PCSTR    pszAttrCheckList[] = {ATTR_MEMBER};  // TODO, use registry key to configure this list

    bvValue.lberbv_val = &eIdBytes[0];
    bvValue.lberbv_len = sizeof(ENTRYID);
    dwError = VmDirEntryIdToBV(pEntry->eId, &bvValue); // encoded entry id stored in index table "record"
    if (dwError)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s VmDirEntryIdToBV failed (%d)(%llu)(%s)",
                __FUNCTION__,
                dwError,
                pEntry->eId,
                pEntry->dn.lberbv_val);
        goto cleanup;
    }

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        for (dwCnt=0; dwCnt<sizeof(pszAttrCheckList)/sizeof(pszAttrCheckList[0]); dwCnt++)
        {
            if (VmDirStringCompareA(pAttr->type.lberbv_val, pszAttrCheckList[dwCnt], FALSE) == 0)
            {
                _VmDirDBCrossChkAttrVal(pBE, pBECtx, pEntry, pAttr, &bvValue);
            }
        }
    }

cleanup:
    VmDirFreeBervalContent(&bvValue);
    return;
}

static
DWORD
_VmDirDBCrossChkBatchEntry(
    PVDIR_BACKEND_INTERFACE pBE,
    ENTRYID*    eIDArray,
    DWORD       dwEIDCnt
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    VDIR_ENTRY  entry = {0};
    VDIR_BACKEND_CTX    mdbBECtx = {0};
    BOOLEAN             bHasTxn = FALSE;

    dwError = VmDirMDBTxnBegin(&mdbBECtx, VDIR_BACKEND_TXN_READ);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    for (dwCnt=0; dwCnt<dwEIDCnt; dwCnt++)
    {
        VmDirFreeEntryContent(&entry);

        dwError = pBE->pfnBESimpleIdToEntry(eIDArray[dwCnt], &entry);
        if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND || dwError == VMDIR_ERROR_ENTRY_NOT_FOUND)
        {
            VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                "%s pfnBESimpleIdToEntry not found (%llu)",
                __FUNCTION__,
                eIDArray[dwCnt]);
            dwError = 0;
            continue;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        // RFC4514 transition specific
        _VmDirDBCrossChkDNRFC4514(pBE, &mdbBECtx, &entry);
        _VmDirDBCrossChkMemberRFC4514(pBE, &mdbBECtx, &entry);

        _VmDirDBCrossChkAttr(pBE, &mdbBECtx, &entry);
    }

cleanup:
    if (bHasTxn)
    {
        VmDirMDBTxnCommit(&mdbBECtx);
    }
    VmDirFreeEntryContent(&entry);
    mdbBECtx.pBEPrivate = NULL;
    VmDirBackendCtxContentFree(&mdbBECtx);

    return dwError;

error:
    if (bHasTxn)
    {
        VmDirMDBTxnAbort(&mdbBECtx);
        bHasTxn = FALSE;
    }
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

static
DWORD
_VmDirDBCrossChkBlob(
    VOID
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    DWORD   dwTotalCnt = 0;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    PVDIR_BACKEND_ENTRYBLOB_ITERATOR pIterator = NULL;
    ENTRYID eID = 0;
    ENTRYID eIDArray[VMDIR_SIZE_256] = {0};

    pBE = VmDirBackendSelect(NULL);

newEntryBlobIterator:

    dwError = pBE->pfnBEEntryBlobIteratorInit(eID, &pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    memset(eIDArray, 0, sizeof(ENTRYID) * VMDIR_SIZE_256);
    dwCnt = 0;

    while (pIterator->bHasNext && dwCnt < VMDIR_SIZE_256)
    {
        dwError = pBE->pfnBEEntryBlobIterate(pIterator, &eID);
        BAIL_ON_VMDIR_ERROR(dwError);

        eIDArray[dwCnt++] = eID;
        dwTotalCnt++;
    }

    if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
    {
        goto cleanup;
    }

    if (pIterator->bHasNext)
    {   // release mdb read lock
        pBE->pfnBEEntryBlobIteratorFree(pIterator);
        pIterator = NULL;
        eID++;
    }

    dwError = _VmDirDBCrossChkBatchEntry(pBE, eIDArray, dwCnt);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pIterator)
    {
        goto newEntryBlobIterator;
    }

cleanup:
    pBE->pfnBEEntryBlobIteratorFree(pIterator);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

static
DWORD
_VmDirDBCrossChkThreadFun(
    PVOID pArg
    )
{
    BOOLEAN     bInLock = FALSE;

    if (gVmdirServerGlobals.bPromoted)
    {
        VMDIR_LOCK_MUTEX(bInLock, gVmdirDBCrossCheck.pMutex);
        if (gVmdirDBCrossCheck.bInProgress == FALSE)
        {
            gVmdirDBCrossCheck.bInProgress = TRUE;
        }
        else
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s DB Cross Check in progress already.", __FUNCTION__);
            goto cleanup;
        }
        VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBCrossCheck.pMutex);

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s start.", __FUNCTION__);
        _VmDirDBCrossChkBlob();
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s finish.", __FUNCTION__);

        VMDIR_LOCK_MUTEX(bInLock, gVmdirDBCrossCheck.pMutex);
        gVmdirDBCrossCheck.bInProgress = FALSE;
        VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBCrossCheck.pMutex);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBCrossCheck.pMutex);
    return 0;
}

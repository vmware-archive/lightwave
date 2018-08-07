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
 * Module Name: Replication
 *
 * Filename: dbswap.c
 *
 * Abstract: swap in remote db
 *
 */

#include "includes.h"

static
DWORD
_VmGetHighestCommittedUSN(
    PSTR*   ppszHighestCommittedUSN
    );

static
DWORD
_VmDirComposeUtdVector(
    PVMDIR_SWAP_DB_INFO pSwapDBInfo
    );

static
DWORD
_VmDirSwapDBInternal(
    PCSTR   pszdbHomeDir,
    PCSTR   pszSwapDir
    );

static
DWORD
_VmDirComposeHighWaterMark(
    PVMDIR_SWAP_DB_INFO pSwapDBInfo
    );

DWORD
VmDirSwapDB(
    PCSTR dbHomeDir
    )
{
    DWORD       dwError = LDAP_SUCCESS;
    BOOLEAN     bLegacyDataLoaded = FALSE;

    dwError = _VmDirSwapDBInternal(dbHomeDir, LIGHTWAVE_TMP_DIR);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirdStateSet(VMDIRD_STATE_STARTUP);

    dwError = VmDirInitBackend(&bLegacyDataLoaded);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bLegacyDataLoaded)
    {
        dwError = VmDirPatchLocalSubSchemaSubEntry();
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirWriteSchemaObjects();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VmDirSetACLMode();

    VmDirdStateSet(VMDIRD_STATE_NORMAL);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, " error (%u)", dwError);
    goto cleanup;
}


DWORD
VmDirPrepareSwapDBInfo(
    PCSTR                   pszHostName,    // partner server object cn
    PVMDIR_SWAP_DB_INFO*    ppSwapDBInfo
    )
{
    DWORD       dwError = 0;
    PVMDIR_SWAP_DB_INFO pLocalSwapDBInfo = NULL;

    if (!ppSwapDBInfo)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_SWAP_DB_INFO), (PVOID*)&pLocalSwapDBInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszHostName)
    {
        dwError = VmDirAllocateStringA(pszHostName, &pLocalSwapDBInfo->pszPartnerServerName);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "My partner %s", pLocalSwapDBInfo->pszPartnerServerName);
    }

    dwError = VmDirInternalGetDSERootServerCN(&pLocalSwapDBInfo->pszOrgDBServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "DB was from %s", pLocalSwapDBInfo->pszOrgDBServerName);

    dwError = _VmDirComposeUtdVector(pLocalSwapDBInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pLocalSwapDBInfo->pszPartnerServerName ||      // no partner, DR case
        VmDirStringCompareA(                            // DB copied from joining partner
            pLocalSwapDBInfo->pszPartnerServerName,
            pLocalSwapDBInfo->pszOrgDBServerName,
            FALSE) == 0)
    {
        dwError = VmDirAllocateStringA(pLocalSwapDBInfo->pszOrgDBMaxUSN, &pLocalSwapDBInfo->pszMyHighWaterMark);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {   // DB from one node but join to another
        dwError = _VmDirComposeHighWaterMark(pLocalSwapDBInfo);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "My High Water Mark %s", pLocalSwapDBInfo->pszMyHighWaterMark);

    *ppSwapDBInfo = pLocalSwapDBInfo;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, " error (%u)", dwError);
    VmDirFreeSwapDBInfo(pLocalSwapDBInfo);
    goto cleanup;
}

VOID
VmDirFreeSwapDBInfo(
    PVMDIR_SWAP_DB_INFO pSwapDBInfo
    )
{
    if (pSwapDBInfo)
    {
        VMDIR_SAFE_FREE_MEMORY(pSwapDBInfo->pszMyHighWaterMark);
        VmDirFreeUTDVectorCache(pSwapDBInfo->pMyUTDVector);
        VMDIR_SAFE_FREE_MEMORY(pSwapDBInfo->pszOrgDBMaxUSN);
        VMDIR_SAFE_FREE_MEMORY(pSwapDBInfo->pszOrgDBServerName);
        VMDIR_SAFE_FREE_MEMORY(pSwapDBInfo->pszPartnerServerName);
        VMDIR_SAFE_FREE_MEMORY(pSwapDBInfo);
    }
}

static
DWORD
_VmDirComposeHighWaterMark(
    PVMDIR_SWAP_DB_INFO pSwapDBInfo
    )
{
    DWORD               dwError = 0;
    PVDIR_ENTRY         pServerEntry = NULL;
    PVDIR_ATTRIBUTE     pAttrInvocationId = NULL;
    VDIR_OPERATION      searchOp = {0};
    USN                 hwmUSN = 0;

    dwError = VmDirInitStackOperation(&searchOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_SEARCH, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInternalSearchSeverObj(pSwapDBInfo->pszPartnerServerName, &searchOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    pServerEntry = searchOp.internalSearchEntryArray.pEntry;
    pAttrInvocationId = VmDirEntryFindAttribute(ATTR_INVOCATION_ID, pServerEntry);

    // use this node max originating usn as high water mark
    dwError = VmDirUTDVectorCacheLookup(
            pSwapDBInfo->pMyUTDVector, pAttrInvocationId->vals[0].lberbv_val, &hwmUSN);
    if (dwError == LW_STATUS_NOT_FOUND)
    {
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
            "Partner (%s,%s) not found in ORG DB UTDVector (%s).  Join scenario NOT supported.",
            pSwapDBInfo->pszPartnerServerName,
            pAttrInvocationId->vals[0].lberbv_val,
            pSwapDBInfo->pMyUTDVector->pszUtdVector);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pSwapDBInfo->pszMyHighWaterMark, "%" PRId64, hwmUSN);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeOperationContent(&searchOp);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "error (%u)", dwError);
    goto cleanup;
}

/*
 * After copying DB from partner, we need to derive UTDVector for new node from partner.
 * i.e. <whatever partner node current UTDVector> + <partner invoactionid:partner max commited USN>
 */
static
DWORD
_VmDirComposeUtdVector(
    PVMDIR_SWAP_DB_INFO pSwapDBInfo
    )
{
    DWORD               dwError = 0;
    PVDIR_ENTRY         pServerEntry = NULL;
    PVDIR_ATTRIBUTE     pAttrUTDVector = NULL;
    PVDIR_ATTRIBUTE     pAttrInvocationId = NULL;
    VDIR_OPERATION      searchOp = {0};

    dwError = VmDirInitStackOperation(&searchOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_SEARCH, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInternalSearchSeverObj(pSwapDBInfo->pszOrgDBServerName, &searchOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    pServerEntry = searchOp.internalSearchEntryArray.pEntry;
    pAttrUTDVector    = VmDirEntryFindAttribute(ATTR_UP_TO_DATE_VECTOR, pServerEntry);
    pAttrInvocationId = VmDirEntryFindAttribute(ATTR_INVOCATION_ID, pServerEntry);

    dwError = _VmGetHighestCommittedUSN(&pSwapDBInfo->pszOrgDBMaxUSN);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "DB maxCommittedUSN %s", pSwapDBInfo->pszOrgDBMaxUSN);

    dwError = VmDirUTDVectorCacheInit(&pSwapDBInfo->pMyUTDVector);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pAttrUTDVector)
    {
        dwError = VmDirUTDVectorCacheReplace(pSwapDBInfo->pMyUTDVector, pAttrUTDVector->vals[0].lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "DB UTDVector %s", pSwapDBInfo->pMyUTDVector->pszUtdVector);
    }

    dwError = VmDirUTDVectorCacheAdd(
                pSwapDBInfo->pMyUTDVector,
                pAttrInvocationId->vals[0].lberbv.bv_val,
                pSwapDBInfo->pszOrgDBMaxUSN);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "My UTDVector %s", pSwapDBInfo->pMyUTDVector->pszUtdVector);

cleanup:
    VmDirFreeOperationContent(&searchOp);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "error (%u)", dwError);
    goto cleanup;
}

static
DWORD
_VmGetHighestCommittedUSN(
    PSTR*   ppszHighestCommittedUSN
    )
{
    DWORD               dwError = 0;
    USN                 usn = 0;
    USN                 nextUSN = 0;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PSTR                pszUSN = NULL;
    VDIR_BACKEND_CTX    beCtx = {0};

    beCtx.pBE = VmDirBackendSelect(NULL);
    assert(beCtx.pBE);

    dwError = beCtx.pBE->pfnBEGetNextUSN(&beCtx, &nextUSN);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (usn=nextUSN; usn > 1LL; usn--)
    {
        VMDIR_SAFE_FREE_MEMORY(pszUSN);
        VmDirFreeEntryArrayContent(&entryArray);

        dwError = VmDirAllocateStringPrintf(&pszUSN, "%" PRId64, usn);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSimpleEqualFilterInternalSearch(
                    "", LDAP_SCOPE_SUBTREE, ATTR_USN_CHANGED, pszUSN, &entryArray);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (entryArray.iSize == 1 )
        {
            break;
        }
    }

    if (usn == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    *ppszHighestCommittedUSN = pszUSN;
    pszUSN = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszUSN);
    VmDirFreeEntryArrayContent(&entryArray);
    VmDirBackendCtxContentFree(&beCtx);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "error (%u), start USN %" PRId64, dwError, nextUSN);
    goto cleanup;
}

/*
 * TODO, Should chg db dir to /var/lib/vmware/vmdir/db.
 * Then we can just swap partner to db directly.
 */
static
DWORD
_VmDirSwapDBInternal(
    PCSTR   pszdbHomeDir,   // e.g. /var/lib/vmware/vmdir
    PCSTR   pszSwapDir      // e.g. /var/lib/vmware/lightwave_tmp
    )
{
    DWORD   dwError = 0;
    int     errorCode = 0;
    CHAR    cmdBuf[VMDIR_MAX_FILE_NAME_LEN] = {0};
    CHAR    partnerDBdBuf[VMDIR_MAX_FILE_NAME_LEN] = {0};
    CHAR    swapDirBuf[VMDIR_MAX_FILE_NAME_LEN] = {0};

    // /var/lib/vmware/lightwave_tmp/partner
    dwError = VmDirStringPrintFA(swapDirBuf, VMDIR_MAX_FILE_NAME_LEN,
        "%s/%s", pszSwapDir, LOCAL_PARTNER_DIR);
    BAIL_ON_VMDIR_ERROR(dwError);

    // /var/lib/vmware/vmdir/partner
    dwError = VmDirStringPrintFA(partnerDBdBuf, VMDIR_MAX_FILE_NAME_LEN,
        "%s/%s", pszdbHomeDir, LOCAL_PARTNER_DIR);
    BAIL_ON_VMDIR_ERROR(dwError);

    // rm -rf /var/lib/vmware/lightwave_tmp/partner
    dwError = VmDirStringPrintFA(cmdBuf, VMDIR_MAX_FILE_NAME_LEN,
        "rm -rf %s", swapDirBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRun(cmdBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    // mv /var/lib/vmware/vmdir/partner /var/lib/vmware/lightwave_tmp/partner
    dwError = VmDirStringPrintFA(cmdBuf, VMDIR_MAX_FILE_NAME_LEN,
        "mv %s %s", partnerDBdBuf, swapDirBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRun(cmdBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    // rm -rf /var/lib/vmware/vmdir/*
    dwError = VmDirStringPrintFA(cmdBuf, VMDIR_MAX_FILE_NAME_LEN,
        "rm -rf %s/*", pszdbHomeDir);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRun(cmdBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    // mv /var/lib/vmware/lightwave_tmp/partner/* /var/lib/vmware/vmdir/
    dwError = VmDirStringPrintFA(cmdBuf, VMDIR_MAX_FILE_NAME_LEN,
        "mv %s/* %s/", swapDirBuf, pszdbHomeDir);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRun(cmdBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s, DB Swapped", __FUNCTION__);

cleanup:

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "Error %d, errno %d", dwError, errorCode);
    goto cleanup;
}

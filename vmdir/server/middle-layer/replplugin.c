/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

// Handle (ADD to replication agreements in-memory cache) MY replication agreements.
DWORD
VmDirPluginReplAgrPostAddCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD   dwError = 0;
    PCSTR   pszErrorContext = NULL;
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr = NULL;

    if (gVmdirServerGlobals.serverObjDN.bvnorm_val) // Skip processing "initial" objects.
    {
        if (pEntry->dn.bvnorm_val == NULL)
        {
            pszErrorContext = "Normalize DN";
            dwError = VmDirNormalizeDN(&pEntry->dn, pEntry->pSchemaCtx);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (!VmDirEntryIsObjectclass(pEntry, OC_REPLICATION_AGREEMENT))
        {
            goto cleanup;
        }

        if ((pEntry->dn.bvnorm_len > gVmdirServerGlobals.serverObjDN.bvnorm_len) &&
            (VmDirStringCompareA(gVmdirServerGlobals.serverObjDN.bvnorm_val,
                    pEntry->dn.bvnorm_val + (pEntry->dn.bvnorm_len - gVmdirServerGlobals.serverObjDN.bvnorm_len), TRUE) == 0))
        {
            pszErrorContext = "Add Replication Agreement into cache";
            if (VmDirReplAgrEntryToInMemory(pEntry, &pReplAgr) != 0)
            {
                dwError = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            VmDirInsertRAToCache(pReplAgr);
            pReplAgr = NULL;  // gVmdirReplAgrs take over pReplAgr
        }
    }

cleanup:
    VmDirFreeReplicationAgreement(pReplAgr);
    return dwError;

error:
    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);
    goto cleanup;
}

// Mark RA isDeleted = TRUE if present in gVmdirReplAgrs.
DWORD
VmDirPluginReplAgrPostDeleteCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PCSTR   pszErrorContext = NULL;
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr = NULL;

    if (pEntry->dn.bvnorm_val == NULL)
    {
        pszErrorContext = "Normalize DN";
        dwError = VmDirNormalizeDN(&pEntry->dn, pEntry->pSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!VmDirEntryIsObjectclass(pEntry, OC_REPLICATION_AGREEMENT))
    {
        goto cleanup;
    }

    VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.replAgrsMutex);

    for (pReplAgr = gVmdirReplAgrs; pReplAgr; pReplAgr = pReplAgr->next)
    {
        if (VmDirStringCompareA(pReplAgr->dn.bvnorm_val, pEntry->dn.bvnorm_val, TRUE) == 0)
        {
            pReplAgr->isDeleted = TRUE;
            break;
        }
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.replAgrsMutex);
    return dwError;

error:
    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);
    goto cleanup;
}

DWORD
VmDirPluginServerEntryPostAddCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD   dwError = 0;
    PCSTR   pszErrorContext = NULL;

    if (VmDirEntryIsObjectclass(pEntry, OC_DIR_SERVER))
    {
        pszErrorContext = "Add a new set of replication metrics";
        dwError = VmDirReplMetricsCacheAdd(pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);
    goto cleanup;
}

DWORD
VmDirPluginServerEntryPostDeleteCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD   dwError = 0;
    PCSTR   pszHostname = NULL;
    PCSTR   pszErrorContext = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;

    if (VmDirEntryIsObjectclass(pEntry, OC_DIR_SERVER))
    {
        pszErrorContext = "Remove a set of replication metrics";

        pAttr = VmDirFindAttrByName(pEntry, ATTR_CN);
        dwError = pAttr ? 0 : VMDIR_ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);

        pszHostname = pAttr->vals[0].lberbv_val;
        dwError = VmDirReplMetricsCacheRemove(pszHostname);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);
    goto cleanup;
}

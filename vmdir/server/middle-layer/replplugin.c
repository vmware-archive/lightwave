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

        if (VmDirIsTombStoneObject(pEntry->dn.lberbv_val))
        {
            goto cleanup;
        }

        if (VmDirEntryIsObjectclass(pEntry, OC_DIR_SERVER))
        {
            VmDirClusterSetCacheReload(); //Recalculate Cluster State Cache for any replication topology change.
        }

        if (!VmDirEntryIsObjectclass(pEntry, OC_REPLICATION_AGREEMENT))
        {
            goto cleanup;
        }

        // check if it's a direct partner
        if (VmDirStringEndsWith(pEntry->dn.bvnorm_val, gVmdirServerGlobals.serverObjDN.bvnorm_val, TRUE))
        {
            pszErrorContext = "Add Replication Agreement into cache";
            if (VmDirReplAgrEntryToInMemory(pEntry, &pReplAgr) != 0)
            {
                dwError = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            VmDirInsertRAToCache(pReplAgr);
            pReplAgr = NULL;  // gVmdirReplAgrs take over pReplAgr

            VmDirPopulateInvocationIdInReplAgr();
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

    if (VmDirIsTombStoneObject(pEntry->dn.lberbv_val))
    {
        goto cleanup;
    }

    if (VmDirEntryIsObjectclass(pEntry, OC_DIR_SERVER))
    {
        //Check whether to mark the node as inActive and reload Cluster State Cache
        VmDirClusterDeleteNode(pEntry);
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

    if (VmDirIsTombStoneObject(pEntry->dn.lberbv_val))
    {
        goto cleanup;
    }

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

    if (VmDirIsTombStoneObject(pEntry->dn.lberbv_val))
    {
        goto cleanup;
    }

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

DWORD
VmDirPluginDCAccountPostModifyCommit(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    BOOLEAN bOnehop = FALSE;
    PSTR    pszDCContainer = NULL;
    PSTR    pszHostname = NULL;
    PSTR    pszModifyTime = NULL;
    PCSTR   pszErrorContext = NULL;
    LONG    modified = 0;
    LONG    now = 0;
    LONG    duration = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    PVMDIR_REPLICATION_METRICS      pReplMetrics = NULL;
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr = NULL;

    dwError = VmDirAllocateStringPrintf(
            &pszDCContainer,
            "%s=%s,%s",
            ATTR_OU,
            VMDIR_DOMAIN_CONTROLLERS_RDN_VAL,
            gVmdirServerGlobals.systemDomainDN.bvnorm_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirStringEndsWith(pEntry->dn.bvnorm_val, pszDCContainer, FALSE))
    {
        pszErrorContext = "Extracting hostname (cn) from DCAccount";

        pAttr = VmDirFindAttrByName(pEntry, ATTR_CN);
        dwError = pAttr ? 0 : VMDIR_ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);

        pszHostname = pAttr->vals[0].lberbv_val;

        pszErrorContext = "Extracting modifyTimeStamp from DCAccount";

        pAttr = VmDirFindAttrByName(pEntry, ATTR_MODIFYTIMESTAMP);
        dwError = pAttr ? 0 : VMDIR_ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);

        pszModifyTime = pAttr->vals[0].lberbv_val;

        pszErrorContext = "Convert timestamps to epoch";

        dwError = VmDirConvertTimestampToEpoch(pszModifyTime, &modified);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirConvertTimestampToEpoch(NULL, &now);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszErrorContext = "Compute epoch difference and collect metrics";

        duration = modified < now ? now - modified : 0;

        if (VmDirReplMetricsCacheFind(pszHostname, &pReplMetrics) == 0)
        {
            VmMetricsGaugeSet(pReplMetrics->pTimeConverge, duration);

            // collect one-hop time if direct partner
            VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.replAgrsMutex);

            for (pReplAgr = gVmdirReplAgrs; pReplAgr; pReplAgr = pReplAgr->next)
            {
                if (pReplAgr->isDeleted == FALSE &&
                    VmDirStringCompareA(pReplAgr->pszHostname, pszHostname, FALSE) == 0)
                {
                    bOnehop = TRUE;
                    break;
                }
            }

            VmMetricsGaugeSet(pReplMetrics->pTimeOnehop, bOnehop ? duration : 0);
        }
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.replAgrsMutex);
    VMDIR_SAFE_FREE_MEMORY(pszDCContainer);
    return dwError;

error:
    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszErrorContext);
    goto cleanup;
}

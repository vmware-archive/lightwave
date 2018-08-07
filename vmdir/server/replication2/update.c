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
 * Module Name: Replication update
 *
 * Filename: update.c
 *
 * Abstract:
 *
 */

#include "includes.h"

static
VOID
_VmDirReplicationUpdateTombStoneEntrySyncState(
    PVMDIR_REPLICATION_UPDATE   pUpdate
    );

int
VmDirReplUpdateCreate(
    LDAP*                           pLd,
    LDAPMessage*                    pEntry,
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr,
    PVMDIR_REPLICATION_UPDATE*      ppUpdate
    )
{
    int                       retVal = LDAP_SUCCESS;
    PVMDIR_REPLICATION_UPDATE pReplUpdate = NULL;
    LDAPControl**             ppCtrls = NULL;

    retVal = VmDirAllocateMemory(sizeof(*pReplUpdate), (PVOID) &pReplUpdate);
    if (retVal)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    retVal = ldap_get_entry_controls(pLd, pEntry, &ppCtrls);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    pReplUpdate->syncState = -1;
    pReplUpdate->partnerUsn = 0;

    retVal = ParseAndFreeSyncStateControl(&ppCtrls, &pReplUpdate->syncState, &pReplUpdate->partnerUsn);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = VmDirAllocateStringA(pReplAgr->dcConn.pszHostname, &pReplUpdate->pszPartner);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = VmDirParseBerToEntry(pEntry->lm_ber, &pReplUpdate->pEntry, NULL, NULL);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    *ppUpdate = pReplUpdate;

cleanup:
    if (ppCtrls)
    {
        ldap_controls_free(ppCtrls);
    }
    return retVal;

ldaperror:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: error = (%d)",
            __FUNCTION__,
            retVal);

    VmDirFreeReplUpdate(pReplUpdate);
    goto cleanup;
}

VOID
VmDirFreeReplUpdate(
    PVMDIR_REPLICATION_UPDATE   pUpdate
    )
{
    if (pUpdate)
    {
        VMDIR_SAFE_FREE_MEMORY(pUpdate->pszPartner);
        VmDirFreeEntry(pUpdate->pEntry);
    }

    VMDIR_SAFE_FREE_MEMORY(pUpdate);
}

VOID
VmDirReplUpdateApply(
    PVMDIR_REPLICATION_UPDATE   pReplUpdate
    )
{
    int errVal = 0;
    int entryState = 0;

    if (!pReplUpdate)
    {
        errVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(errVal);
    }

    _VmDirReplicationUpdateTombStoneEntrySyncState(pReplUpdate);

    entryState = pReplUpdate->syncState;

    if (entryState == LDAP_SYNC_ADD)
    {
        errVal = ReplAddEntry(pReplUpdate);
    }
    else if (entryState == LDAP_SYNC_MODIFY)
    {
        errVal = ReplModifyEntry(pReplUpdate);
    }
    else if (entryState == LDAP_SYNC_DELETE)
    {
        errVal = ReplDeleteEntry(pReplUpdate);
    }
    else
    {
        errVal = LDAP_OPERATIONS_ERROR;
    }

    BAIL_ON_SIMPLE_LDAP_ERROR(errVal);

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
                    "%s: sync_state = (%d) error = (%d)",
                    __FUNCTION__,
                    pReplUpdate->syncState,
                    errVal);

cleanup:
    return;

ldaperror:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: sync_state = (%d) error = (%d)",
                    __FUNCTION__,
                    pReplUpdate->syncState,
                    errVal);

    goto cleanup;
}

static
VOID
_VmDirReplicationUpdateTombStoneEntrySyncState(
    PVMDIR_REPLICATION_UPDATE pUpdate
    )
{
    DWORD               dwError = 0;
    PSTR                pszObjectGuid = NULL;
    PSTR                pszTempString = NULL;
    PSTR                pszContext = NULL;
    PSTR                pszDupDn = NULL;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    VDIR_BERVALUE       bvParentDn = VDIR_BERVALUE_INIT;

    dwError = VmDirGetParentDN(&pUpdate->pEntry->dn, &bvParentDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirIsDeletedContainer(bvParentDn.lberbv_val) &&
        pUpdate->syncState == LDAP_SYNC_ADD)
    {
        dwError = VmDirAllocateStringOfLenA(pUpdate->pEntry->dn.lberbv_val,
                                            pUpdate->pEntry->dn.lberbv_len,
                                            &pszDupDn);
        BAIL_ON_VMDIR_ERROR(dwError);

        // Tombstone DN format: cn=<cn value>#objectGUID:<objectguid value>,<DeletedObjectsContainer>
        pszTempString = VmDirStringStrA(pszDupDn, "#objectGUID:");
        pszObjectGuid = VmDirStringTokA(pszTempString, ",", &pszContext);
        pszObjectGuid = pszObjectGuid + VmDirStringLenA("#objectGUID:");

        dwError = VmDirSimpleEqualFilterInternalSearch("", LDAP_SCOPE_SUBTREE, ATTR_OBJECT_GUID, pszObjectGuid, &entryArray);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (entryArray.iSize == 1)
        {
            pUpdate->syncState = LDAP_SYNC_DELETE;
            VMDIR_LOG_INFO(
                VMDIR_LOG_MASK_ALL,
                "%s: (tombstone handling) change sync state to delete: (%s)",
                __FUNCTION__,
                pUpdate->pEntry->dn.lberbv_val);
        }
    }

cleanup:
    VmDirFreeBervalContent(&bvParentDn);
    VMDIR_SAFE_FREE_MEMORY(pszDupDn);
    VmDirFreeEntryArrayContent(&entryArray);
    return;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: error = (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}

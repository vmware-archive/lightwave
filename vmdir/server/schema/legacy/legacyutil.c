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

#include "../includes.h"

DWORD
VmDirReadSubSchemaSubEntry(
    PVDIR_ENTRY*    ppSubSchemaSubEntry
    )
{
    DWORD   dwError = 0;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    PVDIR_ENTRY pEntry = NULL;

    // bootstrap indication is required if and
    // only if we search subschema subentry
    gVdirSchemaGlobals.pVdirSchema->bIsBootStrapSchema = TRUE;

    pBE = VmDirBackendSelect(NULL);
    assert(pBE);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pBE->pfnBESimpleIdToEntry(SUB_SCEHMA_SUB_ENTRY_ID, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSubSchemaSubEntry = pEntry;

cleanup:
    gVdirSchemaGlobals.pVdirSchema->bIsBootStrapSchema = FALSE;
    return dwError;

error:
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
            "%s subschema subentry not found (%d)", __FUNCTION__, dwError );

    VmDirFreeEntry(pEntry);
    goto cleanup;
}

static
DWORD
_AddOperationMod(
    PVDIR_OPERATION     pOperation,
    int                 modOp,
    PSTR                modType,
    PVMDIR_STRING_LIST  modVals
    )
{
    DWORD   dwError = 0, i = 0;
    PVDIR_BERVALUE  pBerv = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BERVALUE) * (modVals->dwCount + 1),
            (PVOID*)&pBerv);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < modVals->dwCount; i++)
    {
        pBerv[i].lberbv_val = (PSTR)modVals->pStringList[i];
        pBerv[i].lberbv_len = VmDirStringLenA(pBerv[i].lberbv_val);
    }

    dwError = VmDirOperationAddModReq(pOperation,
            modOp, modType, pBerv, modVals->dwCount);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pBerv);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirUpdateSubSchemaSubEntry(
    PVDIR_LEGACY_SCHEMA_MOD pLegacySchemaMod
    )
{
    DWORD   dwError = 0;
    VDIR_OPERATION  ldapOp = {0};

    dwError = VmDirInitStackOperation(&ldapOp,
            VDIR_OPERATION_TYPE_INTERNAL,
            LDAP_REQ_MODIFY,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    ldapOp.reqDn.lberbv.bv_val = SUB_SCHEMA_SUB_ENTRY_DN;
    ldapOp.reqDn.lberbv.bv_len = VmDirStringLenA(SUB_SCHEMA_SUB_ENTRY_DN);

    ldapOp.request.modifyReq.dn.lberbv.bv_val = ldapOp.reqDn.lberbv.bv_val;
    ldapOp.request.modifyReq.dn.lberbv.bv_len = ldapOp.reqDn.lberbv.bv_len;

    if (pLegacySchemaMod->pAddCr->dwCount > 0)
    {
        dwError = _AddOperationMod(&ldapOp,
                LDAP_MOD_ADD,
                ATTR_DITCONTENTRULES,
                pLegacySchemaMod->pAddCr);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pLegacySchemaMod->pDelCr->dwCount > 0)
    {
        dwError = _AddOperationMod(&ldapOp,
                LDAP_MOD_DELETE,
                ATTR_DITCONTENTRULES,
                pLegacySchemaMod->pDelCr);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pLegacySchemaMod->pAddOc->dwCount > 0)
    {
        dwError = _AddOperationMod(&ldapOp,
                LDAP_MOD_ADD,
                ATTR_OBJECTCLASSES,
                pLegacySchemaMod->pAddOc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pLegacySchemaMod->pDelOc->dwCount > 0)
    {
        dwError = _AddOperationMod(&ldapOp,
                LDAP_MOD_DELETE,
                ATTR_OBJECTCLASSES,
                pLegacySchemaMod->pDelOc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pLegacySchemaMod->pAddAt->dwCount > 0)
    {
        dwError = _AddOperationMod(&ldapOp,
                LDAP_MOD_ADD,
                ATTR_ATTRIBUTETYPES,
                pLegacySchemaMod->pAddAt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pLegacySchemaMod->pDelAt->dwCount > 0)
    {
        dwError = _AddOperationMod(&ldapOp,
                LDAP_MOD_DELETE,
                ATTR_ATTRIBUTETYPES,
                pLegacySchemaMod->pDelAt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (ldapOp.request.modifyReq.numMods > 0)
    {
        dwError = VmDirInternalModifyEntry(&ldapOp);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                "%s Updated local subschema subentry",
                __FUNCTION__ );
    }
    else
    {
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                "%s local subschema subentry is up-to-date",
                __FUNCTION__ );
    }

cleanup:
    VmDirFreeOperationContent(&ldapOp);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

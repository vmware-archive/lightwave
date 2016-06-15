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
_CreateNewLocalSchemaObject(
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pObjDiff
    )
{
    DWORD   dwError = 0, i = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_BERVALUE  pBerv = NULL;
    PVDIR_ENTRY     pEntry = NULL;
    VDIR_OPERATION  ldapOp = {0};

    assert(pObjDiff);

    dwError = VmDirInitStackOperation(&ldapOp,
            VDIR_OPERATION_TYPE_INTERNAL,
            LDAP_REQ_ADD,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    ldapOp.reqDn.lberbv_val = pObjDiff->pszDN;
    ldapOp.reqDn.lberbv_len = VmDirStringLenA(pObjDiff->pszDN);

    pEntry = ldapOp.request.addReq.pEntry;
    pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pEntry->pSchemaCtx = VmDirSchemaCtxClone(ldapOp.pSchemaCtx);
    pEntry->dn.lberbv_val = ldapOp.reqDn.lberbv_val;
    pEntry->dn.lberbv_len = ldapOp.reqDn.lberbv_len;

    while (LwRtlHashMapIterate(pObjDiff->mods, &iter, &pair))
    {
        PVDIR_LDAP_MOD pMod = (PVDIR_LDAP_MOD)pair.pValue;

        dwError = VmDirAllocateMemory(
                sizeof(VDIR_BERVALUE) * (pMod->pVals->dwCount + 1),
                (PVOID*)&pBerv);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (i = 0; i < pMod->pVals->dwCount; i++)
        {
            pBerv[i].lberbv_val = (PSTR)pMod->pVals->pStringList[i];
            pBerv[i].lberbv_len = VmDirStringLenA(pBerv[i].lberbv_val);
        }

        dwError = VmDirEntryAddBervArrayAttribute(pEntry,
                pMod->pszType, pBerv, (USHORT)pMod->pVals->dwCount);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pBerv);
    }

    dwError = VmDirInternalAddEntry(&ldapOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeOperationContent(&ldapOp);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pBerv);
    goto cleanup;
}

static
DWORD
_ModifyExistingLocalSchemaObject(
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pObjDiff
    )
{
    DWORD   dwError = 0, i = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_BERVALUE  pBerv = NULL;
    VDIR_OPERATION  ldapOp = {0};

    assert(pObjDiff);

    dwError = VmDirInitStackOperation(&ldapOp,
            VDIR_OPERATION_TYPE_INTERNAL,
            LDAP_REQ_MODIFY,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    ldapOp.reqDn.lberbv_val = pObjDiff->pszDN;
    ldapOp.reqDn.lberbv_len = VmDirStringLenA(pObjDiff->pszDN);

    ldapOp.request.modifyReq.dn.lberbv_val = ldapOp.reqDn.lberbv_val;
    ldapOp.request.modifyReq.dn.lberbv_len = ldapOp.reqDn.lberbv_len;

    while (LwRtlHashMapIterate(pObjDiff->mods, &iter, &pair))
    {
        PVDIR_LDAP_MOD pMod = (PVDIR_LDAP_MOD)pair.pValue;

        dwError = VmDirAllocateMemory(
                sizeof(VDIR_BERVALUE) * (pMod->pVals->dwCount + 1),
                (PVOID*)&pBerv);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (i = 0; i < pMod->pVals->dwCount; i++)
        {
            pBerv[i].lberbv_val = (PSTR)pMod->pVals->pStringList[i];
            pBerv[i].lberbv_len = VmDirStringLenA(pBerv[i].lberbv_val);
        }

        dwError = VmDirOperationAddModReq(&ldapOp,
                pMod->op, pMod->pszType, pBerv, pMod->pVals->dwCount);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pBerv);
    }

    dwError = VmDirInternalModifyEntry(&ldapOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeOperationContent(&ldapOp);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pBerv);
    goto cleanup;
}

DWORD
VmDirPatchLocalSchemaObjects(
    PVDIR_SCHEMA_CTX    pOldCtx,
    PVDIR_SCHEMA_CTX    pNewCtx
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff = NULL;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    VDIR_BACKEND_CTX    beCtx = {0};

    if (!pNewCtx)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSchemaGetDiff(
            pOldCtx ? pOldCtx->pLdapSchema : NULL,
            pNewCtx->pLdapSchema,
            &pSchemaDiff);
    BAIL_ON_VMDIR_ERROR(dwError);

    beCtx.pBE = VmDirBackendSelect(NULL);

    dwError = beCtx.pBE->pfnBEConfigureFsync(FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNode = pSchemaDiff->attrToAdd->pHead;
    while (pNode)
    {
        dwError = _CreateNewLocalSchemaObject(
                (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNode->pPrev;
    }

    pNode = pSchemaDiff->attrToModify->pHead;
    while (pNode)
    {
        dwError = _ModifyExistingLocalSchemaObject(
                (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNode->pPrev;
    }

    pNode = pSchemaDiff->classToAdd->pHead;
    while (pNode)
    {
        dwError = _CreateNewLocalSchemaObject(
                (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNode->pPrev;
    }

    pNode = pSchemaDiff->classToModify->pHead;
    while (pNode)
    {
        dwError = _ModifyExistingLocalSchemaObject(
                (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNode->pPrev;
    }

error:
    VmDirFreeLdapSchemaDiff(pSchemaDiff);
    dwError = beCtx.pBE->pfnBEConfigureFsync(TRUE);
    return dwError;
}

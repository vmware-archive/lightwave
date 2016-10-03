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
VOID
_FreeLDAPModArray(
    LDAPMod**   mods
    )
{
    DWORD   i = 0;

    for (i = 0; mods && mods[i]; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(mods[i]);
    }
    VMDIR_SAFE_FREE_MEMORY(mods);
}

static
DWORD
_CreateLDAPModArrayFromObjDiff(
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pObjDiff,
    LDAPMod***                      pMods)
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    DWORD       dwNumMod = 0;
    LDAPMod**   mods = NULL;

    assert(pObjDiff && pMods);

    dwNumMod = LwRtlHashMapGetCount(pObjDiff->mods);
    if (dwNumMod)
    {
        dwError = VmDirAllocateMemory(
                sizeof(LDAPMod*) * (dwNumMod + 1), (PVOID*)&mods);
        BAIL_ON_VMDIR_ERROR(dwError);

        while (LwRtlHashMapIterate(pObjDiff->mods, &iter, &pair))
        {
            PVDIR_LDAP_MOD pMod = (PVDIR_LDAP_MOD)pair.pValue;

            dwError = VmDirAllocateMemory(
                    sizeof(LDAPMod), (PVOID*)&mods[i]);
            BAIL_ON_VMDIR_ERROR(dwError);

            mods[i]->mod_op = pMod->op;
            mods[i]->mod_type = pMod->pszType;
            mods[i]->mod_vals.modv_strvals =
                    (PSTR*)pMod->pVals->pStringList;
            i++;
        }
    }

    *pMods = mods;

cleanup:
    return dwError;

error:
    _FreeLDAPModArray(mods);
    goto cleanup;
}

DWORD
VmDirPatchRemoteSchemaObjects(
    LDAP*               pLd,
    PVDIR_LDAP_SCHEMA   pNewSchema
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_SCHEMA               pCurSchema = NULL;
    PVDIR_LDAP_SCHEMA               pMergedSchema = NULL;
    PVDIR_LDAP_SCHEMA_DIFF          pSchemaDiff = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pObjDiff = NULL;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    LDAPMod**   mods = NULL;

    if (!pLd || !pNewSchema)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSchemaInit(&pCurSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaLoadRemoteSchema(pCurSchema, pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaMerge(pCurSchema, pNewSchema, &pMergedSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaGetDiff(pCurSchema, pMergedSchema, &pSchemaDiff);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNode = pSchemaDiff->attrToAdd->pHead;
    while (pNode)
    {
        pObjDiff = (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement;

        dwError = _CreateLDAPModArrayFromObjDiff(pObjDiff, &mods);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = ldap_add_ext_s(pLd, pObjDiff->pszDN, mods, NULL, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        _FreeLDAPModArray(mods);
        mods = NULL;
        pNode = pNode->pPrev;
    }

    pNode = pSchemaDiff->attrToModify->pHead;
    while (pNode)
    {
        pObjDiff = (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement;

        dwError = _CreateLDAPModArrayFromObjDiff(pObjDiff, &mods);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = ldap_modify_ext_s(pLd, pObjDiff->pszDN, mods, NULL, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        _FreeLDAPModArray(mods);
        mods = NULL;
        pNode = pNode->pPrev;
    }

    pNode = pSchemaDiff->classToAdd->pHead;
    while (pNode)
    {
        pObjDiff = (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement;

        dwError = _CreateLDAPModArrayFromObjDiff(pObjDiff, &mods);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = ldap_add_ext_s(pLd, pObjDiff->pszDN, mods, NULL, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        _FreeLDAPModArray(mods);
        mods = NULL;
        pNode = pNode->pPrev;
    }

    pNode = pSchemaDiff->classToModify->pHead;
    while (pNode)
    {
        pObjDiff = (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement;

        dwError = _CreateLDAPModArrayFromObjDiff(pObjDiff, &mods);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = ldap_modify_ext_s(pLd, pObjDiff->pszDN, mods, NULL, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        _FreeLDAPModArray(mods);
        mods = NULL;
        pNode = pNode->pPrev;
    }

error:
    _FreeLDAPModArray(mods);
    VmDirFreeLdapSchema(pCurSchema);
    VmDirFreeLdapSchema(pMergedSchema);
    VmDirFreeLdapSchemaDiff(pSchemaDiff);
    return dwError;
}

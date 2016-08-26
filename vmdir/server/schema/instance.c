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

DWORD
VmDirSchemaInstanceCreate(
    PVDIR_LDAP_SCHEMA           pLdapSchema,
    PVDIR_SCHEMA_INSTANCE*      ppInstance
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap = NULL;
    PVDIR_SCHEMA_INSTANCE   pInstance = NULL;
    LW_HASHMAP_ITER atIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER ocIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER crIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER srIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER nfIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    if (!pLdapSchema || !ppInstance)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pAttrIdMap = gVdirSchemaGlobals.pAttrIdMap;

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_INSTANCE),
            (PVOID*)&pInstance);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pInstance->attributeTypes.byName,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pInstance->attributeTypes.byId,
            LwRtlHashDigestPointer,
            LwRtlHashEqualPointer,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pInstance->objectClasses.byName,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pInstance->contentRules.byName,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pInstance->structureRules.byId,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pInstance->nameForms.byName,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pLdapSchema->attributeTypes, &atIter, &pair))
    {
        PVDIR_LDAP_ATTRIBUTE_TYPE pAt = (PVDIR_LDAP_ATTRIBUTE_TYPE)pair.pValue;
        PVDIR_SCHEMA_AT_DESC pATDesc = NULL;

        dwError = VmDirSchemaATDescCreate(pAt, &pATDesc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSchemaAttrIdMapGetAttrId(
                pAttrIdMap, pATDesc->pszName, &pATDesc->usAttrID);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pInstance->attributeTypes.byName,
                pATDesc->pszName, pATDesc, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pInstance->attributeTypes.byId,
                (PVOID)(uintptr_t)pATDesc->usAttrID, pATDesc, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pLdapSchema->objectClasses, &ocIter, &pair))
    {
        PVDIR_LDAP_OBJECT_CLASS pOc = (PVDIR_LDAP_OBJECT_CLASS)pair.pValue;
        PVDIR_SCHEMA_OC_DESC pOCDesc = NULL;

        dwError = VmDirSchemaOCDescCreate(pOc, &pOCDesc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pInstance->objectClasses.byName,
                pOCDesc->pszName, pOCDesc, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pLdapSchema->contentRules, &crIter, &pair))
    {
        PVDIR_LDAP_CONTENT_RULE pCr = (PVDIR_LDAP_CONTENT_RULE)pair.pValue;
        PVDIR_SCHEMA_CR_DESC pCRDesc = NULL;

        dwError = VmDirSchemaCRDescCreate(pCr, &pCRDesc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pInstance->contentRules.byName,
                pCRDesc->pszName, pCRDesc, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pLdapSchema->structureRules, &srIter, &pair))
    {
        PVDIR_LDAP_STRUCTURE_RULE pSr = (PVDIR_LDAP_STRUCTURE_RULE)pair.pValue;
        PVDIR_SCHEMA_SR_DESC pSRDesc = NULL;

        dwError = VmDirSchemaSRDescCreate(pSr, &pSRDesc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pInstance->structureRules.byId,
                pSRDesc->pszRuleID, pSRDesc, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pLdapSchema->nameForms, &nfIter, &pair))
    {
        PVDIR_LDAP_NAME_FORM pNf = (PVDIR_LDAP_NAME_FORM)pair.pValue;
        PVDIR_SCHEMA_NF_DESC pNFDesc = NULL;

        dwError = VmDirSchemaNFDescCreate(pNf, &pNFDesc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pInstance->nameForms.byName,
                pNFDesc->pszName, pNFDesc, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMutex(&(pInstance->mutex));
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppInstance = pInstance;

cleanup:
    return dwError;

error:
    VmDirFreeSchemaInstance(pInstance);
    goto cleanup;
}

DWORD
VmDirSchemaInstanceGetATDescByName(
    PVDIR_SCHEMA_INSTANCE   pInstance,
    PCSTR                   pszName,
    PVDIR_SCHEMA_AT_DESC*   ppATDesc
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_AT_DESC pATDesc = NULL;

    if (!pInstance || !pszName || !ppATDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    LwRtlHashMapFindKey(pInstance->attributeTypes.byName,
            (PVOID*)&pATDesc, pszName);

    if (!pATDesc)
    {
        dwError = ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppATDesc = pATDesc;

error:
    return dwError;
}

DWORD
VmDirSchemaInstanceGetATDescById(
    PVDIR_SCHEMA_INSTANCE   pInstance,
    USHORT                  usId,
    PVDIR_SCHEMA_AT_DESC*   ppATDesc
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_AT_DESC pATDesc = NULL;

    if (!pInstance || !ppATDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    LwRtlHashMapFindKey(pInstance->attributeTypes.byId,
            (PVOID*)&pATDesc, (PVOID)(intptr_t)usId);

    if (!pATDesc)
    {
        dwError = ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppATDesc = pATDesc;

error:
    return dwError;
}

DWORD
VmDirSchemaInstanceGetOCDescByName(
    PVDIR_SCHEMA_INSTANCE   pInstance,
    PCSTR                   pszName,
    PVDIR_SCHEMA_OC_DESC*   ppOCDesc
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_OC_DESC pOCDesc = NULL;

    if (!pInstance || !pszName || !ppOCDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    LwRtlHashMapFindKey(pInstance->objectClasses.byName,
            (PVOID*)&pOCDesc, pszName);

    if (!pOCDesc)
    {
        dwError = ERROR_NO_SUCH_OBJECTCLASS;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppOCDesc = pOCDesc;

error:
    return dwError;
}

DWORD
VmDirSchemaInstanceGetCRDescByName(
    PVDIR_SCHEMA_INSTANCE   pInstance,
    PCSTR                   pszName,
    PVDIR_SCHEMA_CR_DESC*   ppCRDesc
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_CR_DESC pCRDesc = NULL;

    if (!pInstance || !pszName || !ppCRDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    LwRtlHashMapFindKey(pInstance->contentRules.byName,
            (PVOID*)&pCRDesc, pszName);

    if (!pCRDesc)
    {
        dwError = ERROR_NO_SUCH_DITCONTENTRULES;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppCRDesc = pCRDesc;

error:
    return dwError;
}

DWORD
VmDirSchemaInstanceGetSRDescById(
    PVDIR_SCHEMA_INSTANCE   pInstance,
    PCSTR                   pszId,
    PVDIR_SCHEMA_SR_DESC*   ppSRDesc
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_SR_DESC pSRDesc = NULL;

    if (!pInstance || !pszId || !ppSRDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    LwRtlHashMapFindKey(pInstance->structureRules.byId,
            (PVOID*)&pSRDesc, pszId);

    if (!pSRDesc)
    {
        dwError = ERROR_NO_SUCH_DITSTRUCTURERULES;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppSRDesc = pSRDesc;

error:
    return dwError;
}

DWORD
VmDirSchemaInstanceGetNFDescByName(
    PVDIR_SCHEMA_INSTANCE   pInstance,
    PCSTR                   pszName,
    PVDIR_SCHEMA_NF_DESC*   ppNFDesc
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_NF_DESC pNFDesc = NULL;

    if (!pInstance || !pszName || !ppNFDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    LwRtlHashMapFindKey(pInstance->nameForms.byName,
            (PVOID*)&pNFDesc, pszName);

    if (!pNFDesc)
    {
        dwError = ERROR_NO_SUCH_NAMEFORMS;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppNFDesc = pNFDesc;

error:
    return dwError;
}

static
VOID
_FreeDescMapPair(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VMDIR_SAFE_FREE_MEMORY(pPair->pValue);
}

VOID
VmDirFreeSchemaInstance(
    PVDIR_SCHEMA_INSTANCE pInstance
    )
{
    if (pInstance)
    {
        if (pInstance->attributeTypes.byName)
        {
            LwRtlHashMapClear(pInstance->attributeTypes.byName,
                    _FreeDescMapPair, NULL);
            LwRtlFreeHashMap(&pInstance->attributeTypes.byName);
        }

        if (pInstance->attributeTypes.byId)
        {
            LwRtlHashMapClear(pInstance->attributeTypes.byId,
                    VmDirNoopHashMapPairFree, NULL);
            LwRtlFreeHashMap(&pInstance->attributeTypes.byId);
        }

        if (pInstance->objectClasses.byName)
        {
            LwRtlHashMapClear(pInstance->objectClasses.byName,
                    _FreeDescMapPair, NULL);
            LwRtlFreeHashMap(&pInstance->objectClasses.byName);
        }

        if (pInstance->contentRules.byName)
        {
            LwRtlHashMapClear(pInstance->contentRules.byName,
                    _FreeDescMapPair, NULL);
            LwRtlFreeHashMap(&pInstance->contentRules.byName);
        }

        if (pInstance->structureRules.byId)
        {
            LwRtlHashMapClear(pInstance->structureRules.byId,
                    _FreeDescMapPair, NULL);
            LwRtlFreeHashMap(&pInstance->structureRules.byId);
        }

        if (pInstance->nameForms.byName)
        {
            LwRtlHashMapClear(pInstance->nameForms.byName,
                    _FreeDescMapPair, NULL);
            LwRtlFreeHashMap(&pInstance->nameForms.byName);
        }

        VMDIR_SAFE_FREE_MUTEX(pInstance->mutex);
        VMDIR_SAFE_FREE_MEMORY(pInstance);
    }
}

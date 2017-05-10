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
VmDirLdapSchemaCopy(
    PVDIR_LDAP_SCHEMA   pOrgSchema,
    PVDIR_LDAP_SCHEMA*  ppCopySchema
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER atIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER ocIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER crIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER srIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER nfIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_LDAP_SCHEMA   pCopySchema = NULL;

    if (!pOrgSchema || !ppCopySchema)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSchemaInit(&pCopySchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pOrgSchema->attributeTypes, &atIter, &pair))
    {
        dwError = VmDirLdapSchemaAddAt(pCopySchema,
                (PVDIR_LDAP_ATTRIBUTE_TYPE)pair.pValue);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pOrgSchema->objectClasses, &ocIter, &pair))
    {
        dwError = VmDirLdapSchemaAddOc(pCopySchema,
                (PVDIR_LDAP_OBJECT_CLASS)pair.pValue);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pOrgSchema->contentRules, &crIter, &pair))
    {
        dwError = VmDirLdapSchemaAddCr(pCopySchema,
                (PVDIR_LDAP_CONTENT_RULE)pair.pValue);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pOrgSchema->structureRules, &srIter, &pair))
    {
        dwError = VmDirLdapSchemaAddSr(pCopySchema,
                (PVDIR_LDAP_STRUCTURE_RULE)pair.pValue);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pOrgSchema->nameForms, &nfIter, &pair))
    {
        dwError = VmDirLdapSchemaAddNf(pCopySchema,
                (PVDIR_LDAP_NAME_FORM)pair.pValue);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppCopySchema = pCopySchema;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeLdapSchema(pCopySchema);
    goto cleanup;
}

DWORD
VmDirLdapSchemaDeepCopy(
    PVDIR_LDAP_SCHEMA   pOrgSchema,
    PVDIR_LDAP_SCHEMA*  ppCopySchema
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER atIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER ocIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER crIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER srIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER nfIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_LDAP_SCHEMA   pCopySchema = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pCopyAt = NULL;
    PVDIR_LDAP_OBJECT_CLASS     pCopyOc = NULL;
    PVDIR_LDAP_CONTENT_RULE     pCopyCr = NULL;
    PVDIR_LDAP_STRUCTURE_RULE   pCopySr = NULL;
    PVDIR_LDAP_NAME_FORM        pCopyNf = NULL;

    if (!pOrgSchema || !ppCopySchema)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSchemaInit(&pCopySchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pOrgSchema->attributeTypes, &atIter, &pair))
    {
        PVDIR_LDAP_ATTRIBUTE_TYPE pAt = (PVDIR_LDAP_ATTRIBUTE_TYPE)pair.pValue;

        dwError = VmDirLdapAtDeepCopy(pAt, &pCopyAt);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddAt(pCopySchema, pCopyAt);
        BAIL_ON_VMDIR_ERROR(dwError);
        pCopyAt = NULL;
    }

    while (LwRtlHashMapIterate(pOrgSchema->objectClasses, &ocIter, &pair))
    {
        PVDIR_LDAP_OBJECT_CLASS pCr = (PVDIR_LDAP_OBJECT_CLASS)pair.pValue;

        dwError = VmDirLdapOcDeepCopy(pCr, &pCopyOc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddOc(pCopySchema, pCopyOc);
        BAIL_ON_VMDIR_ERROR(dwError);
        pCopyOc = NULL;
    }

    while (LwRtlHashMapIterate(pOrgSchema->contentRules, &crIter, &pair))
    {
        PVDIR_LDAP_CONTENT_RULE pCr = (PVDIR_LDAP_CONTENT_RULE)pair.pValue;

        dwError = VmDirLdapCrDeepCopy(pCr, &pCopyCr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddCr(pCopySchema, pCopyCr);
        BAIL_ON_VMDIR_ERROR(dwError);
        pCopyCr = NULL;
    }

    while (LwRtlHashMapIterate(pOrgSchema->structureRules, &srIter, &pair))
    {
        PVDIR_LDAP_STRUCTURE_RULE pSr = (PVDIR_LDAP_STRUCTURE_RULE)pair.pValue;

        dwError = VmDirLdapSrDeepCopy(pSr, &pCopySr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddSr(pCopySchema, pCopySr);
        BAIL_ON_VMDIR_ERROR(dwError);
        pCopySr = NULL;
    }

    while (LwRtlHashMapIterate(pOrgSchema->nameForms, &nfIter, &pair))
    {
        PVDIR_LDAP_NAME_FORM pNf = (PVDIR_LDAP_NAME_FORM)pair.pValue;

        dwError = VmDirLdapNfDeepCopy(pNf, &pCopyNf);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddNf(pCopySchema, pCopyNf);
        BAIL_ON_VMDIR_ERROR(dwError);
        pCopyNf = NULL;
    }

    *ppCopySchema = pCopySchema;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeLdapSchema(pCopySchema);
    VmDirFreeLdapAt(pCopyAt);
    VmDirFreeLdapOc(pCopyOc);
    VmDirFreeLdapCr(pCopyCr);
    VmDirFreeLdapSr(pCopySr);
    VmDirFreeLdapNf(pCopyNf);
    goto cleanup;
}

DWORD
VmDirLdapAtDeepCopy(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pOrgAt,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppCopyAt
    )
{
    DWORD   dwError = 0;
    PSTR    pszAt = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pCopyAt = NULL;

    if (!pOrgAt || !ppCopyAt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapAtToStr(pOrgAt, &pszAt);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapAtParseStr(pszAt, &pCopyAt);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyStrArray(
            pOrgAt->ppszUniqueScopes, &pCopyAt->ppszUniqueScopes);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCopyAt->dwSearchFlags = pOrgAt->dwSearchFlags;

    *ppCopyAt = pCopyAt;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAt);
    return dwError;

error:
    VmDirFreeLdapAt(pCopyAt);
    goto cleanup;
}

DWORD
VmDirLdapOcDeepCopy(
    PVDIR_LDAP_OBJECT_CLASS     pOrgOc,
    PVDIR_LDAP_OBJECT_CLASS*    ppCopyOc
    )
{
    DWORD   dwError = 0;
    PSTR    pszOc = NULL;
    PVDIR_LDAP_OBJECT_CLASS pCopyOc = NULL;

    if (!pOrgOc || !ppCopyOc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapOcToStr(pOrgOc, &pszOc);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapOcParseStr(pszOc, &pCopyOc);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppCopyOc = pCopyOc;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszOc);
    return dwError;

error:
    VmDirFreeLdapOc(pCopyOc);
    goto cleanup;
}

DWORD
VmDirLdapCrDeepCopy(
    PVDIR_LDAP_CONTENT_RULE     pOrgCr,
    PVDIR_LDAP_CONTENT_RULE*    ppCopyCr
    )
{
    DWORD   dwError = 0;
    PSTR    pszCr = NULL;
    PVDIR_LDAP_CONTENT_RULE pCopyCr = NULL;

    if (!pOrgCr || !ppCopyCr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapCrToStr(pOrgCr, &pszCr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapCrParseStr(pszCr, &pCopyCr);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppCopyCr = pCopyCr;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszCr);
    return dwError;

error:
    VmDirFreeLdapCr(pCopyCr);
    goto cleanup;
}

DWORD
VmDirLdapSrDeepCopy(
    PVDIR_LDAP_STRUCTURE_RULE   pOrgSr,
    PVDIR_LDAP_STRUCTURE_RULE*  ppCopySr
    )
{
    DWORD   dwError = 0;
    PSTR    pszSr = NULL;
    PVDIR_LDAP_STRUCTURE_RULE   pCopySr = NULL;

    if (!pOrgSr || !ppCopySr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSrToStr(pOrgSr, &pszSr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSrParseStr(pszSr, &pCopySr);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppCopySr = pCopySr;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszSr);
    return dwError;

error:
    VmDirFreeLdapSr(pCopySr);
    goto cleanup;
}

DWORD
VmDirLdapNfDeepCopy(
    PVDIR_LDAP_NAME_FORM    pOrgNf,
    PVDIR_LDAP_NAME_FORM*   ppCopyNf
    )
{
    DWORD   dwError = 0;
    PSTR    pszNf = NULL;
    PVDIR_LDAP_NAME_FORM    pCopyNf = NULL;

    if (!pOrgNf || !ppCopyNf)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapNfToStr(pOrgNf, &pszNf);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapNfParseStr(pszNf, &pCopyNf);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppCopyNf = pCopyNf;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszNf);
    return dwError;

error:
    VmDirFreeLdapNf(pCopyNf);
    goto cleanup;
}

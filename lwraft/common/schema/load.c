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
VmDirLdapSchemaLoadStrLists(
    PVDIR_LDAP_SCHEMA   pSchema,
    PVMDIR_STRING_LIST  pAtStrList,
    PVMDIR_STRING_LIST  pOcStrList,
    PVMDIR_STRING_LIST  pCrStrList,
    PVMDIR_STRING_LIST  pIdxStrList
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0, j = 0;
    PSTR    pszAtName = NULL;
    BOOLEAN bEmpty = FALSE;
    BOOLEAN bGlobalUniq = FALSE;

    PVDIR_LDAP_ATTRIBUTE_TYPE   pOldAt = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pNewAt = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE*  pNewAtList = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pMergedAt = NULL;

    PVDIR_LDAP_OBJECT_CLASS pOldOc = NULL;
    PVDIR_LDAP_OBJECT_CLASS pNewOc = NULL;
    PVDIR_LDAP_OBJECT_CLASS pMergedOc = NULL;

    PVDIR_LDAP_CONTENT_RULE pOldCr = NULL;
    PVDIR_LDAP_CONTENT_RULE pNewCr = NULL;
    PVDIR_LDAP_CONTENT_RULE pMergedCr = NULL;

    if (!pSchema || !pAtStrList || !pOcStrList || !pCrStrList || !pIdxStrList)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    bEmpty = VmDirLdapSchemaIsEmpty(pSchema);

    for (i = 0; i < pAtStrList->dwCount; i++)
    {
        dwError = VmDirLdapAtParseStr(pAtStrList->pStringList[i], &pNewAt);
        BAIL_ON_VMDIR_ERROR(dwError);

        // inherit sup syntax if available (PR 1868307)
        if (!bEmpty)
        {
            (VOID)VmDirLdapAtResolveSupWithLogOpt(pSchema, pNewAt, FALSE);
            // cast VOID because this might not succeed
            // if sup isn't already added in pSchema
        }

        dwError = VmDirLdapAtResolveAliases(pNewAt, &pNewAtList);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirFreeLdapAt(pNewAt);
        pNewAt = NULL;

        for (j = 0; pNewAtList && pNewAtList[j]; j++)
        {
            pOldAt = NULL;
            LwRtlHashMapFindKey(
                    pSchema->attributeTypes,
                    (PVOID*)&pOldAt,
                    pNewAtList[j]->pszName);

            dwError = VmDirLdapAtMerge(pOldAt, pNewAtList[j], &pMergedAt);
            BAIL_ON_VMDIR_ERROR(dwError);

            VmDirFreeLdapAt(pNewAtList[j]);
            pNewAtList[j] = NULL;

            dwError = VmDirLdapSchemaAddAt(pSchema, pMergedAt);
            BAIL_ON_VMDIR_ERROR(dwError);
            pMergedAt = NULL;
        }
        VMDIR_SAFE_FREE_MEMORY(pNewAtList);
    }

    for (i = 0; i < pOcStrList->dwCount; i++)
    {
        dwError = VmDirLdapOcParseStr(pOcStrList->pStringList[i], &pNewOc);
        BAIL_ON_VMDIR_ERROR(dwError);

        // class sup defaults to 'top' (PR 1853569)
        dwError = VmDirLdapOcResolveSup(pSchema, pNewOc);
        BAIL_ON_VMDIR_ERROR(dwError);

        pOldOc = NULL;
        LwRtlHashMapFindKey(
                pSchema->objectClasses,
                (PVOID*)&pOldOc,
                pNewOc->pszName);

        dwError = VmDirLdapOcMerge(pOldOc, pNewOc, &pMergedOc);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirFreeLdapOc(pNewOc);
        pNewOc = NULL;

        dwError = VmDirLdapSchemaAddOc(pSchema, pMergedOc);
        BAIL_ON_VMDIR_ERROR(dwError);
        pMergedOc = NULL;
    }

    for (i = 0; i < pCrStrList->dwCount; i++)
    {
        dwError = VmDirLdapCrParseStr(pCrStrList->pStringList[i], &pNewCr);
        BAIL_ON_VMDIR_ERROR(dwError);

        pOldCr = NULL;
        LwRtlHashMapFindKey(
                pSchema->contentRules,
                (PVOID*)&pOldCr,
                pNewCr->pszName);

        dwError = VmDirLdapCrMerge(pOldCr, pNewCr, &pMergedCr);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirFreeLdapCr(pNewCr);
        pNewCr = NULL;

        dwError = VmDirLdapSchemaAddCr(pSchema, pMergedCr);
        BAIL_ON_VMDIR_ERROR(dwError);
        pMergedCr = NULL;
    }

    for (i = 0; i < pIdxStrList->dwCount; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(pszAtName);

        dwError = VmDirLdapIdxParseStr(
                pIdxStrList->pStringList[i], &pszAtName, &bGlobalUniq);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddIdx(pSchema, pszAtName, bGlobalUniq);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSchemaResolveAndVerifyAll(pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaRemoveNoopData(pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAtName);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    for (; pNewAtList && pNewAtList[j]; j++)
    {
        VmDirFreeLdapAt(pNewAtList[j]);
    }
    VMDIR_SAFE_FREE_MEMORY(pNewAtList);
    VmDirFreeLdapAt(pMergedAt);
    VmDirFreeLdapOc(pMergedOc);
    VmDirFreeLdapCr(pMergedCr);
    VmDirFreeLdapAt(pNewAt);
    VmDirFreeLdapOc(pNewOc);
    VmDirFreeLdapCr(pNewCr);
    goto cleanup;
}

DWORD
VmDirLdapSchemaLoadFile(
    PVDIR_LDAP_SCHEMA   pSchema,
    PCSTR               pszSchemaFilePath
    )
{
    DWORD   dwError = 0;
    PVMDIR_STRING_LIST  pAtStrList = NULL;
    PVMDIR_STRING_LIST  pOcStrList = NULL;
    PVMDIR_STRING_LIST  pCrStrList = NULL;
    PVMDIR_STRING_LIST  pIdxStrList = NULL;

    if (!pSchema || IsNullOrEmptyString(pszSchemaFilePath))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirReadSchemaFile(pszSchemaFilePath,
            &pAtStrList, &pOcStrList, &pCrStrList, &pIdxStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaLoadStrLists(
            pSchema, pAtStrList, pOcStrList, pCrStrList, pIdxStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirStringListFree(pAtStrList);
    VmDirStringListFree(pOcStrList);
    VmDirStringListFree(pCrStrList);
    VmDirStringListFree(pIdxStrList);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VmDirLdapSchemaLoadRemoteSchema(
    PVDIR_LDAP_SCHEMA   pSchema,
    LDAP*               pLd
    )
{
    DWORD   dwError = 0;
    PCSTR   pcszAtFilter = "(objectClass=attributeSchema)";
    PCSTR   pcszOcFilter = "(objectClass=classSchema)";
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt = NULL;
    PVDIR_LDAP_OBJECT_CLASS     pOc = NULL;
    PVDIR_LDAP_CONTENT_RULE     pCr = NULL;

    if (!pSchema || !pLd)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = ldap_search_ext_s(pLd,
            SCHEMA_NAMING_CONTEXT_DN, LDAP_SCOPE_SUBTREE, pcszAtFilter,
            NULL, FALSE, NULL, NULL, NULL, 0, &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pEntry = ldap_first_entry(pLd, pResult); pEntry;
            pEntry = ldap_next_entry(pLd, pEntry))
    {
        dwError = VmDirLdapAtParseLDAPEntry(pLd, pEntry, &pAt);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddAt(pSchema, pAt);
        BAIL_ON_VMDIR_ERROR(dwError);
        pAt = NULL;
    }

    ldap_msgfree(pResult);

    dwError = ldap_search_ext_s(pLd,
            SCHEMA_NAMING_CONTEXT_DN, LDAP_SCOPE_SUBTREE, pcszOcFilter,
            NULL, FALSE, NULL, NULL, NULL, 0, &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pEntry = ldap_first_entry(pLd, pResult); pEntry;
            pEntry = ldap_next_entry(pLd, pEntry))
    {
        dwError = VmDirLdapOcParseLDAPEntry(pLd, pEntry, &pOc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapCrParseLDAPEntry(pLd, pEntry, &pCr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddOc(pSchema, pOc);
        BAIL_ON_VMDIR_ERROR(dwError);
        pOc = NULL;

        if (pCr)
        {
            dwError = VmDirLdapSchemaAddCr(pSchema, pCr);
            BAIL_ON_VMDIR_ERROR(dwError);
            pCr = NULL;
        }
    }

    dwError = VmDirLdapSchemaResolveAndVerifyAll(pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VDIR_SAFE_LDAP_MSGFREE(pResult);
    return dwError;

error:
    VmDirFreeLdapAt(pAt);
    VmDirFreeLdapOc(pOc);
    VmDirFreeLdapCr(pCr);
    goto cleanup;
}

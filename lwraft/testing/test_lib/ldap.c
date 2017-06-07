/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
VmDirTestReplaceBinaryAttributeValues(
    LDAP *pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    BYTE *pbAttributeValue,
    DWORD dwDataLength
    )
{
    DWORD dwError = 0;
    BerValue *ppBerValues[2] = {NULL, NULL};
    BerValue bvSecurityDescriptor = {0};
    LDAPMod addReplace;
    LDAPMod *mods[2];

    /* Initialize the attribute, specifying 'modify' as the operation */
    bvSecurityDescriptor.bv_val = pbAttributeValue;
    bvSecurityDescriptor.bv_len = dwDataLength;
    ppBerValues[0] = &bvSecurityDescriptor;
    addReplace.mod_op     = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    addReplace.mod_type   = (PSTR)pszAttribute;
    addReplace.mod_bvalues = ppBerValues;

    /* Fill the attributes array (remember it must be NULL-terminated) */
    mods[0] = &addReplace;
    mods[1] = NULL;

    dwError = ldap_modify_ext_s(pLd, pszDN, mods, NULL, NULL);

    return dwError;
}

DWORD
VmDirTestGetAttributeValueString(
    LDAP *pLd,
    PCSTR pBase,
    int ldapScope,
    PCSTR pszFilter,
    PCSTR pszAttribute,
    PSTR *ppszAttributeValue
    )
{
    DWORD dwError = 0;
    PCSTR ppszAttrs[2] = {0};
    LDAPMessage *pResult = NULL;
    BerValue** ppBerValues = NULL;
    PSTR pszAttributeValue = NULL;
    LDAPMessage *pEntry = NULL;

    ppszAttrs[0] = pszAttribute;
    dwError = ldap_search_ext_s(
                pLd,
                pBase,
                ldapScope,
                pszFilter ? pszFilter : "",
                (PSTR*)ppszAttrs,
                0,
                NULL,
                NULL,
                NULL,
                -1,
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLd, pResult) != 1)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_STATE);
    }

    pEntry = ldap_first_entry(pLd, pResult);
    if (!pEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_STATE);
    }

    ppBerValues = ldap_get_values_len(pLd, pEntry, pszAttribute);
    if (!ppBerValues || (ldap_count_values_len(ppBerValues) != 1))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_STATE);
    }

    dwError = VmDirAllocateStringA(ppBerValues[0]->bv_val, &pszAttributeValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszAttributeValue = pszAttributeValue;
    pszAttributeValue = NULL;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszAttributeValue);

    if (ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
        ppBerValues = NULL;
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
        pResult = NULL;
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirTestReplaceAttributeValues(
    LDAP *pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    PCSTR *ppszAttributeValues
    )
{
    DWORD dwError = 0;

    LDAPMod addReplace;
    LDAPMod *mods[2];

    /* Initialize the attribute, specifying 'ADD' as the operation */
    addReplace.mod_op     = LDAP_MOD_REPLACE;
    addReplace.mod_type   = (PSTR) pszAttribute;
    addReplace.mod_values = (PSTR*) ppszAttributeValues;

    /* Fill the attributes array (remember it must be NULL-terminated) */
    mods[0] = &addReplace;
    mods[1] = NULL;

    /* ....initialize connection, etc. */

    dwError = ldap_modify_ext_s(pLd, pszDN, mods, NULL, NULL);

    return dwError;
}

DWORD
VmDirTestGetAttributeValue(
    LDAP *pLd,
    PCSTR pBase,
    int ldapScope,
    PCSTR pszFilter,
    PCSTR pszAttribute,
    BYTE **ppbAttributeValue,
    PDWORD pdwAttributeLength
    )
{
    DWORD dwError = 0;
    PCSTR ppszAttrs[2] = {0};
    LDAPMessage *pResult = NULL;
    BerValue** ppBerValues = NULL;
    BYTE *pbAttributeValue = NULL;
    DWORD dwAttributeLength = 0;

    ppszAttrs[0] = pszAttribute;
    dwError = ldap_search_ext_s(
                pLd,
                pBase,
                ldapScope,
                pszFilter ? pszFilter : "",
                (PSTR*)ppszAttrs,
                0,
                NULL,
                NULL,
                NULL,
                -1,
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLd, pResult) > 0)
    {
        LDAPMessage* pEntry = ldap_first_entry(pLd, pResult);

        for (; pEntry != NULL; pEntry = ldap_next_entry(pLd, pEntry))
        {
            BerValue** ppBerValues = NULL;
            ppBerValues = ldap_get_values_len(pLd, pEntry, pszAttribute);
            if (ppBerValues != NULL && ldap_count_values_len(ppBerValues) > 0)
            {
                dwError = VmDirAllocateAndCopyMemory(
                            ppBerValues[0][0].bv_val,
                            ppBerValues[0][0].bv_len,
                            (PVOID*)&pbAttributeValue);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwAttributeLength = ppBerValues[0][0].bv_len;
                break;
            }
        }
    }

    *ppbAttributeValue = pbAttributeValue;
    *pdwAttributeLength = dwAttributeLength;
    pbAttributeValue = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pbAttributeValue);

    if (ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
        ppBerValues = NULL;
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
        pResult = NULL;
    }

    return dwError;

error:
    goto cleanup;
}

VOID
VmDirTestLdapUnbind(
    LDAP *pLd
    )
{
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }
}

//
// Enumerates the objects at a certain DN. If you just want to verify that the
// user can enumerate but don't care about the actual objects, pass NULL
// for ppObjectList.
//
// NB -- The VMDIR_STRING_LIST returned contains full DNs for the individual
// objects.
//
DWORD
VmDirTestGetObjectList(
    LDAP *pLd,
    PCSTR pszDn,
    PVMDIR_STRING_LIST *ppObjectList /* OPTIONAL */
    )
{
    DWORD dwError = 0;
    DWORD dwObjectCount = 0;
    PCSTR ppszAttrs[] = {NULL};
    LDAPMessage *pResult = NULL;
    PVMDIR_STRING_LIST pObjectList = NULL;

    dwError = ldap_search_ext_s(
                pLd,
                pszDn,
                LDAP_SCOPE_SUBTREE,
                "(objectClass=*)",
                (PSTR*)ppszAttrs,
                0,
                NULL,
                NULL,
                NULL,
                -1,
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ppObjectList != NULL)
    {
        dwObjectCount = ldap_count_entries(pLd, pResult);
        dwError = VmDirStringListInitialize(&pObjectList, dwObjectCount);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (dwObjectCount > 0)
        {
            LDAPMessage* pEntry = ldap_first_entry(pLd, pResult);

            //
            // Grab the next entry. The first one will be the base DN itself.
            //
            pEntry = ldap_next_entry(pLd, pEntry);
            for (; pEntry != NULL; pEntry = ldap_next_entry(pLd, pEntry))
            {
                dwError = VmDirStringListAddStrClone(ldap_get_dn(pLd, pEntry), pObjectList);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        *ppObjectList = pObjectList;
    }

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error:
    VmDirStringListFree(pObjectList);
    goto cleanup;
}

DWORD
VmDirTestConnectionFromUser(
    PVMDIR_TEST_STATE pState,
    PCSTR pszUserName,
    LDAP **ppLd
    )
{
    DWORD dwError = 0;
    PSTR pszUserUPN = NULL;
    LDAP *pLd;

    dwError = VmDirAllocateStringPrintf(
                &pszUserUPN,
                "%s@%s",
                pszUserName,
                pState->pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind(
                &pLd,
                pState->pszServerName,
                pszUserUPN,
                pState->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLd = pLd;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserUPN);
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDirTestDeleteContainer(
    LDAP *pLd,
    PCSTR pszContainerDn
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVMDIR_STRING_LIST pObjectList;

    dwError = VmDirTestGetObjectList(
                pLd,
                pszContainerDn,
                &pObjectList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIndex = 0; dwIndex < pObjectList->dwCount; ++dwIndex)
    {
        dwError = ldap_delete_ext_s(pLd, pObjectList->pStringList[dwIndex], NULL, NULL);
        if (dwError == LDAP_NOT_ALLOWED_ON_NONLEAF)
        {
            dwError = VmDirTestDeleteContainer(
                        pLd,
                        pObjectList->pStringList[dwIndex]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = ldap_delete_ext_s(pLd, pszContainerDn, NULL, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

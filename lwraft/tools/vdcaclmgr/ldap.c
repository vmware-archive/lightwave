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
VdcLdapAddAttributeValues(
    LDAP *pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    PCSTR *ppszAttributeValues
    )
{
    DWORD dwError = 0;

    LDAPMod addition;
    LDAPMod *mods[2];

    /* Initialize the attribute, specifying 'ADD' as the operation */
    addition.mod_op     = LDAP_MOD_ADD;
    addition.mod_type   = (PSTR) pszAttribute;
    addition.mod_values = (PSTR*) ppszAttributeValues;

    /* Fill the attributes array (remember it must be NULL-terminated) */
    mods[0] = &addition;
    mods[1] = NULL;

    /* ....initialize connection, etc. */

    dwError = ldap_modify_ext_s(pLd, pszDN, mods, NULL, NULL);

    return dwError;
}

DWORD
VdcLdapReplaceAttributeValues(
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
_VdcGetAttributeFromEntry(
    LDAP *pLd,
    LDAPMessage* pEntry,
    PSTR pszAttributeName,
    PSTR *ppszAttributeValue
    )
{
    DWORD dwError = 0;
    PSTR pszAttributeValue = NULL;
    BerValue** ppBerValues = NULL;

    ppBerValues = ldap_get_values_len(pLd, pEntry, pszAttributeName);
    if (ppBerValues != NULL && ldap_count_values_len(ppBerValues) > 0)
    {
        dwError = VmDirAllocateStringA(ppBerValues[0][0].bv_val, &pszAttributeValue);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszAttributeValue = pszAttributeValue;

cleanup:
    if (ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
        ppBerValues = NULL;
    }

    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszAttributeValue);
    goto cleanup;
}

DWORD
VdcLdapEnumerateObjects(
    LDAP *pLd,
    PCSTR pszBase,
    int ldapScope,
    PVMDIR_STRING_LIST *ppObjectDNs
    )
{
    DWORD dwError = 0;
    PCSTR ppszAttrs[] = {NULL};
    LDAPMessage *pResult = NULL;
    LDAPMessage* pEntry = NULL;
    PSTR pszObjectDN = NULL;
    DWORD iEntryCount = 0;
    PVMDIR_STRING_LIST pObjectDNs = NULL;

    dwError = ldap_search_ext_s(
                pLd,
                pszBase,
                ldapScope,
                "(objectClass=*)",
                (PSTR*)ppszAttrs,
                0,
                NULL,
                NULL,
                NULL,
                -1,
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    iEntryCount = ldap_count_entries(pLd, pResult);
    if (iEntryCount > 0)
    {
        dwError = VmDirStringListInitialize(&pObjectDNs, iEntryCount);
        BAIL_ON_VMDIR_ERROR(dwError);

        pEntry = ldap_first_entry(pLd, pResult);
        for (; pEntry != NULL; pEntry = ldap_next_entry(pLd, pEntry))
        {
            assert(pObjectDNs->dwCount < iEntryCount);

            dwError = VmDirAllocateStringA(ldap_get_dn(pLd, pEntry), &pszObjectDN);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringListAdd(pObjectDNs, pszObjectDN);
            BAIL_ON_VMDIR_ERROR(dwError);

            pszObjectDN = NULL;
        }
    }

    *ppObjectDNs = pObjectDNs;
    pObjectDNs = NULL;

cleanup:
    VmDirStringListFree(pObjectDNs);
    VMDIR_SAFE_FREE_STRINGA(pszObjectDN);
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VdcLdapGetObjectSidMappings(
    LDAP *pLd,
    PCSTR pszBase,
    PCSTR pszFilter,
    PLW_HASHMAP pObjectToSidMapping,
    PLW_HASHMAP pSidToObjectMapping
    )
{
    DWORD dwError = 0;
    PCSTR ppszAttrs[3] = {ATTR_CN, ATTR_OBJECT_SID, NULL};
    LDAPMessage *pResult = NULL;
    PSTR pszObjectCN = NULL;
    PSTR pszObjectSid = NULL;

    dwError = ldap_search_ext_s(
                pLd,
                pszBase,
                LDAP_SCOPE_SUBTREE,
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
            dwError = _VdcGetAttributeFromEntry(pLd, pEntry, ATTR_CN, &pszObjectCN);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = _VdcGetAttributeFromEntry(pLd, pEntry, ATTR_OBJECT_SID, &pszObjectSid);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = _VdcAddCopiesToHashTable(pObjectToSidMapping, pszObjectCN, pszObjectSid);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = _VdcAddCopiesToHashTable(pSidToObjectMapping, pszObjectSid, pszObjectCN);
            BAIL_ON_VMDIR_ERROR(dwError);

            VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
            VMDIR_SAFE_FREE_STRINGA(pszObjectCN);
        }
    }

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
    VMDIR_SAFE_FREE_STRINGA(pszObjectCN);

    return dwError;

error:
    goto cleanup;
}

DWORD
VdcLdapGetAttributeValue(
    LDAP *pLd,
    PCSTR pszObjectDN,
    PCSTR pszAttribute,
    PSTR *ppszAttributeValue
    )
{
    DWORD dwError = 0;
    PCSTR ppszAttrs[2] = {0};
    LDAPMessage *pResult = NULL;
    BerValue** ppBerValues = NULL;
    PSTR pszAttributeValue = NULL;

    ppszAttrs[0] = pszAttribute;
    dwError = ldap_search_ext_s(
                pLd,
                pszObjectDN,
                LDAP_SCOPE_BASE,
                "(objectClass=*)",
                (PSTR*)ppszAttrs,
                0,
                NULL,
                NULL,
                NULL,
                -1,
                &pResult);

    if (ldap_count_entries(pLd, pResult) > 0)
    {
        LDAPMessage* pEntry = ldap_first_entry(pLd, pResult);

        for (; pEntry != NULL; pEntry = ldap_next_entry(pLd, pEntry))
        {
            BerValue** ppBerValues = NULL;
            ppBerValues = ldap_get_values_len(pLd, pEntry, pszAttribute);
            if (ppBerValues != NULL && ldap_count_values_len(ppBerValues) > 0)
            {
                dwError = VmDirAllocateStringA(
                            ppBerValues[0][0].bv_val,
                            &pszAttributeValue);
                BAIL_ON_VMDIR_ERROR(dwError);
                break;
            }
        }
    }

    *ppszAttributeValue = pszAttributeValue;

cleanup:

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

    VMDIR_SAFE_FREE_STRINGA(pszAttributeValue);
    goto cleanup;
}

DWORD
VdcLdapAddContainer(
    LDAP*  pLd,
    PCSTR  pszDN,
    PCSTR  pszCN
    )
{
    DWORD       dwError = 0;
    PCSTR       valsCn[] = {pszCN, NULL};
    PCSTR       valsClass[] = {OC_TOP, OC_CONTAINER, NULL};
    LDAPMod     mod[2]={
                            {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
                            {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}}
                       };
    LDAPMod*    attrs[] = {&mod[0], &mod[1], NULL};

    dwError = ldap_add_ext_s(
                             pLd,
                             pszDN,
                             attrs,
                             NULL,
                             NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VdcLdapAddGroup(
    LDAP*  pLd,
    PCSTR  pszDN,
    PCSTR  pszCN
    )
{
    DWORD       dwError = 0;
    PCSTR       valsCn[] = {pszCN, NULL};
    PCSTR       valssAMActName[] = {pszCN, NULL};
    PCSTR       valsClass[] = {OC_TOP, OC_GROUP, NULL};
    LDAPMod     mod[3]={
                            {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
                            {LDAP_MOD_ADD, ATTR_SAM_ACCOUNT_NAME, {(PSTR*)valssAMActName}},
                            {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}}
                       };
    LDAPMod*    attrs[] = {&mod[0], &mod[1], &mod[2], NULL};

    dwError = ldap_add_ext_s(
                             pLd,
                             pszDN,
                             attrs,
                             NULL,
                             NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

BOOLEAN
VdcIfDNExist(
    LDAP* pLd,
    PCSTR pszDN)
{
    DWORD           dwError = 0;
    LDAPMessage*    pSearchRes = NULL;

    dwError = ldap_search_ext_s(
                            pLd,
                            pszDN,         /* base */
                            LDAP_SCOPE_BASE,
                            NULL,       /* filter */
                            NULL,       /* attrs */
                            FALSE,      /* attrsonly */
                            NULL,       /* serverctrls */
                            NULL,       /* clientctrls */
                            NULL,       /* timeout */
                            0,          /* sizelimit */
                            &pSearchRes);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }
    return dwError == 0;
error:
    goto cleanup;
}

/* Replace attribute on all matched entries */
DWORD
VdcLdapReplaceAttrOnEntries(
    LDAP *pLd,
    PCSTR pszBase,
    int ldapScope,
    PCSTR pszFilter,
    PCSTR pAttrName,
    PCSTR pAttrVal
    )
{
    DWORD dwError = 0;
    LDAPMessage *pResult = NULL;
    PSTR pszDn = NULL;
    PCSTR ppszAttrs[] = {ATTR_DN, NULL};
    LDAPMod mod = {0};
    LDAPMod* mods[2] = {&mod, NULL};
    PSTR vals[2] = {0};
    int totalCnt = 0;
    int failedCnt = 0;
    int alreadyUpdatedCnt = 0;
    PSTR oldAttrVal = NULL;

    mod.mod_op = LDAP_MOD_REPLACE;
    mod.mod_type = (PSTR)pAttrName;
    vals[0] = (PSTR)pAttrVal;
    vals[1] = NULL;
    mod.mod_vals.modv_strvals = vals;

    dwError = ldap_search_ext_s(
                pLd,
                pszBase,
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
            if (pszDn)
            {
                ldap_memfree(pszDn);
                pszDn = NULL;
            }
            pszDn = ldap_get_dn(pLd, pEntry);

            VMDIR_SAFE_FREE_STRINGA(oldAttrVal);
            dwError = VdcLdapGetAttributeValue( pLd,
                                        pszDn,
                                        pAttrName,
                                        &oldAttrVal);
            if (dwError == LDAP_SUCCESS && VmDirStringCompareA(oldAttrVal, pAttrVal, FALSE)==0)
            {
                totalCnt++;
                alreadyUpdatedCnt++;
                continue;
            }
            dwError = ldap_modify_ext_s( pLd, pszDn, mods, NULL, NULL);
            if (dwError != LDAP_SUCCESS)
            {
                failedCnt++;
            }
            totalCnt++;
        }
    }

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
        pResult = NULL;
    }

    if (pszDn)
    {
        ldap_memfree(pszDn);
        pszDn = NULL;
    }
    VMDIR_SAFE_FREE_STRINGA(oldAttrVal);

    return dwError;

error:
    goto cleanup;
}

VOID
VdcLdapUnbind(
    LDAP *pLd
    )
{
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }
}

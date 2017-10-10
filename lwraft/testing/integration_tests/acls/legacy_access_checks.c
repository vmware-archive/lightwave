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
_VmDirApplyAttributeModification(
    PVMDIR_TEST_STATE pState,
    PVMDIR_STRING_LIST pObjectList,
    PCSTR pszAttribute,
    PCSTR pszValue
    )
{
    DWORD i = 0;
    PCSTR ppszVals[] = {pszValue, NULL};
    DWORD dwError = 0;

    for (i = 0; i < pObjectList->dwCount; ++i)
    {
        printf("Setting attribute %s for %s to %s\n", pszAttribute, pObjectList->pStringList[i], ppszVals[0]);
        dwError = VmDirTestReplaceAttributeValues(
                    pState->pLd,
                    pObjectList->pStringList[i],
                    pszAttribute,
                    ppszVals);
        printf("dwError ==> %d\n", dwError);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDirTestGetParentDn(
    PCSTR pszBaseDn,
    PSTR *ppszParentDn
    )
{
    DWORD dwError = 0;
    PSTR pszParentDn = NULL;

    pszParentDn = strchr(pszBaseDn, ',');
    if (pszParentDn == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringA(pszParentDn + 1, &pszParentDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszParentDn = pszParentDn;

cleanup:
    return dwError = 0;
error:
    goto cleanup;
}


DWORD
_VmDirSetSecurityDescriptors(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszAcl = NULL;
    PSTR pszDomainSid = NULL;
    PSTR pszParentDn = NULL;
    PVMDIR_STRING_LIST pObjectList = NULL;

    dwError = VmDirTestGetDomainSid(pState, pState->pszBaseDN, &pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszAcl,
                "O:%s-500G:%s-544D:(A;;RPWP;;;S-1-7-32-666)(A;;GXNRNWGXCCDCRPWP;;;%s-544)(A;;GXNRNWGXCCDCRPWP;;;%s-500)",
                pszDomainSid,
                pszDomainSid,
                pszDomainSid,
                pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    printf("acl ==> %s\n", pszAcl);

    dwError = VmDirTestGetParentDn(pState->pszBaseDN, &pszParentDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetObjectList(pState->pLd, pszParentDn, &pObjectList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirApplyAttributeModification(
                pState,
                pObjectList,
                ATTR_ACL_STRING,
                pszAcl);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDomainSid);
    VMDIR_SAFE_FREE_STRINGA(pszParentDn);
    VMDIR_SAFE_FREE_STRINGA(pszAcl);
    VmDirStringListFree(pObjectList);
    return dwError;
error:
    goto cleanup;
}

static
LW_PCVOID
DescriptorCacheRecordGetKey(
    PLW_HASHTABLE_NODE      pNode,
    PVOID                   pUnused
    )
{
    PVMDIR_SD_CACHE_ENTRY pCacheEntry = NULL;

    pCacheEntry = LW_STRUCT_FROM_FIELD(pNode, VMDIR_SD_CACHE_ENTRY, Node);

    return pCacheEntry->pszDn;
}


VOID
_FreeCacheEntry( // TODO -- Naming
    PVMDIR_SD_CACHE_ENTRY pCacheEntry // TODO -- Naming. Struct should have "TEST" in it?
    )
{
    VMDIR_SAFE_FREE_STRINGA(pCacheEntry->pszDn);
    VMDIR_SAFE_FREE_STRINGA(pCacheEntry->pszAcl);
    VmDirFreeMemory(pCacheEntry);
}

VOID
_FreeSecurityDescriptorCache(
    PLW_HASHTABLE pHashTbl
    )
{
    PLW_HASHTABLE_NODE pNode = NULL;
    LW_HASHTABLE_ITER iter = LW_HASHTABLE_ITER_INIT;
    PVMDIR_SD_CACHE_ENTRY pCacheEntry = NULL;

    if (pHashTbl != NULL)
    {
        while ((pNode = LwRtlHashTableIterate(pHashTbl, &iter)))
        {
            pCacheEntry = LW_STRUCT_FROM_FIELD(pNode, VMDIR_SD_CACHE_ENTRY, Node);
            LwRtlHashTableRemove(pHashTbl, pNode);
            _FreeCacheEntry(pCacheEntry);
        }

        LwRtlFreeHashTable(&pHashTbl);
    }
}

VOID
_RestoreSecurityDescriptors( // TODO -- This could be combined with _FreeSecurityDescriptorCache
    PVMDIR_TEST_STATE pState,
    PLW_HASHTABLE pHashTbl
    )
{
    PLW_HASHTABLE_NODE pNode = NULL;
    LW_HASHTABLE_ITER iter = LW_HASHTABLE_ITER_INIT;
    PVMDIR_SD_CACHE_ENTRY pCacheEntry = NULL;
    PCSTR ppszVals[2] = {NULL, NULL};
    DWORD dwError = 0;

    if (pHashTbl != NULL)
    {
        while ((pNode = LwRtlHashTableIterate(pHashTbl, &iter)))
        {
            pCacheEntry = LW_STRUCT_FROM_FIELD(pNode, VMDIR_SD_CACHE_ENTRY, Node);
            ppszVals[0] = pCacheEntry->pszAcl;
            printf("Resetting acl for %s to %s\n", pCacheEntry->pszDn, pCacheEntry->pszAcl); // TODO
            dwError = VmDirTestReplaceAttributeValues(
                    pState->pLd,
                    pCacheEntry->pszDn,
                    ATTR_ACL_STRING,
                    ppszVals);
            if (dwError != 0)
            {
                printf("Resetting the SD on entry %s failed with dwError %d\n", pCacheEntry->pszDn, dwError);
            }
        }
    }
}

DWORD
_VmDirTestAllocateCacheEntry(
    LDAP *pLd,
    LDAPMessage *pEntry,
    PVMDIR_SD_CACHE_ENTRY *ppCacheEntry
    )
{
    DWORD dwError = 0;
    PVMDIR_SD_CACHE_ENTRY pCacheEntry = NULL;
    BerValue** ppBerValues = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_SD_CACHE_ENTRY), (PVOID)&pCacheEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ldap_get_dn(pLd, pEntry), &pCacheEntry->pszDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppBerValues = ldap_get_values_len(pLd, pEntry, ATTR_ACL_STRING);
    if (!ppBerValues || (ldap_count_values_len(ppBerValues) != 1))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_STATE); // TODO
    }

    dwError = VmDirAllocateStringA(ppBerValues[0]->bv_val, &pCacheEntry->pszAcl);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppCacheEntry = pCacheEntry;

cleanup:
    return dwError;
error:
    _FreeCacheEntry(pCacheEntry);
    goto cleanup;
}

DWORD
_VmDirCacheExistingSecurityDescriptors(
    PVMDIR_TEST_STATE pState,
    PLW_HASHTABLE *ppHashTbl
    )
{
    DWORD dwError = 0;
    DWORD dwObjectCount = 0;
    PCSTR ppszAttrs[] = {ATTR_ACL_STRING, NULL};
    LDAPMessage *pResult = NULL;
    LDAPMessage* pEntry = NULL;
    PLW_HASHTABLE pHashTbl = NULL;
    LDAP *pLd = pState->pLd; // TODO
    PVMDIR_SD_CACHE_ENTRY pCacheEntry = NULL;

    dwError = ldap_search_ext_s(
                pLd,
                pState->pszBaseDN,
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

    dwObjectCount = ldap_count_entries(pLd, pResult);
    dwError = LwRtlCreateHashTable(
                    &pHashTbl,
                    DescriptorCacheRecordGetKey,
                    LwRtlHashDigestPstr,
                    LwRtlHashEqualPstr,
                    NULL,
                    dwObjectCount);
    dwError = LwNtStatusToWin32Error(dwError);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry = ldap_first_entry(pLd, pResult);
    for (; pEntry != NULL; pEntry = ldap_next_entry(pLd, pEntry))
    {
        dwError = _VmDirTestAllocateCacheEntry(pLd, pEntry, &pCacheEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("Adding entry %s => %s\n", pCacheEntry->pszDn, pCacheEntry->pszAcl); // TODO
        LwRtlHashTableResizeAndInsert(
                pHashTbl,
                &pCacheEntry->Node,
                NULL);
    }

    *ppHashTbl = pHashTbl;

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;
error:
    _FreeSecurityDescriptorCache(pHashTbl);
    goto cleanup;
}

DWORD
TestLegacyAccessChecks(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PLW_HASHTABLE pHashTbl = NULL;

    dwError = _VmDirCacheExistingSecurityDescriptors(pState, &pHashTbl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSetSecurityDescriptors(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    // TODO
    printf("Sleeping ...\n");
    VmDirSleep(30 * 1000);
    printf("Awake ...\n");
    // TODO
    //
#if 0 // TODO
    dwError = TestStandardRightsForAdminUser(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestStandardRightsForAdminGroup(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestStandardRightsForDomainAdmin(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestStandardRightsForDomainClients(pState);
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

    _RestoreSecurityDescriptors(pState, pHashTbl);

cleanup:
    _FreeSecurityDescriptorCache(pHashTbl);
    return dwError;
error:
    goto cleanup;
}

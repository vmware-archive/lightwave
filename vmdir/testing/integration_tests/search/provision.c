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
_TestProvisionSearchContainer(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    dwError = VmDirAllocateStringPrintf(&pContext->pszSearchDN,
            "%s,%s", VMDIR_TEST_SEARCH_BASE_RDN, pContext->pTestState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateContainer(pContext->pTestState, VMDIR_TEST_SEARCH_BASE, pContext->pszSearchDN, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pContext->pszSearchC1DN,
            "%s,%s", VMDIR_TEST_SEARCH_CONTAINER_1_BIG_RDN, pContext->pszSearchDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateContainer(pContext->pTestState,
            VMDIR_TEST_SEARCH_CONTAINER_1_BIG, pContext->pszSearchC1DN, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pContext->pszSearchC2DN,
            "%s,%s", VMDIR_TEST_SEARCH_CONTAINER_2_BIG_RDN, pContext->pszSearchDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateContainer(pContext->pTestState,
            VMDIR_TEST_SEARCH_CONTAINER_2_BIG, pContext->pszSearchC2DN, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pContext->pszSearchC3DN,
            "%s,%s", VMDIR_TEST_SEARCH_CONTAINER_3_SMALL_RDN, pContext->pszSearchDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateContainer(pContext->pTestState,
            VMDIR_TEST_SEARCH_CONTAINER_3_SMALL, pContext->pszSearchC3DN, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_TestAddObject(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_RECORD   pRec
    )
{
    DWORD   dwError = 0;
    PSTR    pszTmp = NULL;
    PCSTR   pszValCN[] = {pRec->pszCN, NULL};
    PCSTR   pszValDesc[] = {pRec->pszCN, NULL};
    PCSTR   pszValStrIgnoreNonunique[] = {pRec->pszStrIgnoreNonunique, NULL, NULL};
    PCSTR   pszValStrIgnoreUnique[] = {pRec->pszStrIgnoreUnique, NULL};
    PCSTR   pszValStrExactNonunique[] = {pRec->pszStrExactNonunique, NULL};
    PCSTR   pszValStrExactUnique[] = {pRec->pszStrExactUnique, NULL};
    PCSTR   pszValIntegerNonunique[] = {pRec->pszIntegerNonunique, NULL};
    PCSTR   pszValIntegerUnique[] = {pRec->pszIntegerUnique, NULL};
    PCSTR   pszValOC[] = {OC_TEST_SEARCH, NULL};

    LDAPMod mod[] =
    {
        {LDAP_MOD_ADD, ATTR_CN,                 {(PSTR*)pszValCN}},
        {LDAP_MOD_ADD, ATTR_OBJECT_CLASS,       {(PSTR*)pszValOC}},
        {LDAP_MOD_ADD, ATTR_DESCRIPTION,        {(PSTR*)pszValDesc}},
        {LDAP_MOD_ADD, ATTR_STR_IGNORE_NONUNIQUE, {(PSTR*)pszValStrIgnoreNonunique}},
        {LDAP_MOD_ADD, ATTR_STR_IGNORE_UNIQUE,  {(PSTR*)pszValStrIgnoreUnique}},
        {LDAP_MOD_ADD, ATTR_STR_EXACT_NONUNIQUE,{(PSTR*)pszValStrExactNonunique}},
        {LDAP_MOD_ADD, ATTR_STR_EXACT_UNIQUE,   {(PSTR*)pszValStrExactUnique}},
        {LDAP_MOD_ADD, ATTR_INTEGER_NONUNIQUE,  {(PSTR*)pszValIntegerNonunique}},
        {LDAP_MOD_ADD, ATTR_INTEGER_UNIQUE,     {(PSTR*)pszValIntegerUnique}},
    };

    LDAPMod* addAttrs[] = {&mod[0], &mod[1], &mod[2], &mod[3], &mod[4], &mod[5],&mod[6], &mod[7], &mod[8], NULL};

    // add multi-value attribute
    dwError = VmDirAllocateStringPrintf(&pszTmp,
            "%s-1", pRec->pszStrIgnoreNonunique);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszValStrIgnoreNonunique[1] = pszTmp;

    dwError = ldap_add_ext_s(
            pContext->pTestState->pLd,
            pRec->pszDN,
            addAttrs,
            NULL,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    VMDIR_SAFE_FREE_MEMORY(pszTmp);
    return dwError;
}

static
DWORD
_TestProvisionSetAttribute(
    PVMDIR_SEARCH_TEST_RECORD   pRec
    )
{
    DWORD   dwError = 0;

    dwError = VmDirAllocateStringPrintf(
            &pRec->pszIntegerNonunique, "%d", pRec->dwIndex % INTEGER_NONUNIQUE_MOD);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pRec->pszIntegerUnique, "%d", pRec->dwIndex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pRec->pszStrExactNonunique,
            "%s%s", VMDIR_STR_EXACT, pRec->pszIntegerNonunique);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pRec->pszStrExactUnique,
            "%s%s", VMDIR_STR_EXACT, pRec->pszIntegerUnique);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pRec->pszStrIgnoreNonunique,
            "%s%s", VMDIR_STR_IGNORE, pRec->pszIntegerNonunique);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pRec->pszStrIgnoreUnique,
            "%s%s", VMDIR_STR_IGNORE, pRec->pszIntegerUnique);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

static
VOID
_TestFreeSearchTestRecordContent(
    PVMDIR_SEARCH_TEST_RECORD   pRec
    )
{
    if (pRec)
    {
        VMDIR_SAFE_FREE_MEMORY(pRec->pszCN);
        VMDIR_SAFE_FREE_MEMORY(pRec->pszDN);
        VMDIR_SAFE_FREE_MEMORY(pRec->pszIntegerNonunique);
        VMDIR_SAFE_FREE_MEMORY(pRec->pszIntegerUnique);
        VMDIR_SAFE_FREE_MEMORY(pRec->pszStrExactNonunique);
        VMDIR_SAFE_FREE_MEMORY(pRec->pszStrExactUnique);
        VMDIR_SAFE_FREE_MEMORY(pRec->pszStrIgnoreNonunique);
        VMDIR_SAFE_FREE_MEMORY(pRec->pszStrIgnoreUnique);
    }
}

/*(
 *                   cn=testsearch  (3646 objects -s sub)
 *                  /    /    \   \
 *              cn=C1  cn=C2 cn=C3 \
 *               /       /      \  (1204 obj)
 *     (1205 obj)  (1204 obj)   \
 *                             (29 obj)
 *
 *  A sample search entry looks like:
 *
    dn: cn=TestSearchCN-1000,cn=testSearchC1Big,cn=testSearch,dc=lw,dc=local
    vmwTestSearchIntegerUnique: 1000
    vmwTestSearchIntegerNonunique: 0
    vmwTestSearchCaseExactStringUnique: StringExact1000
    vmwTestSearchCaseExactStringNonunique: StringExact0
    vmwTestSearchCaseIgnoreStringUnique: StringIgnore1000
    vmwTestSearchCaseIgnoreStringNonunique: StringIgnore0    << multi-value
    vmwTestSearchCaseIgnoreStringNonunique: StringIgnore0-1
    description: TestSearchCN-1000
    objectClass: top
    objectClass: vmwSearchTest
    cn: TestSearchCN-1000
 */
static
DWORD
_TestAddSearchObject(
    PVMDIR_SEARCH_TEST_CONTEXT pContext,
    DWORD   dwCnt
    )
{
    DWORD   dwError = 0;
    VMDIR_SEARCH_TEST_RECORD    record = {0};
    PSTR    pszBase = NULL;

    record.dwIndex = dwCnt;

    switch (dwCnt%3)
    {
        case 0:
            pszBase = pContext->pszSearchDN;
            break;
        case 1:
            pszBase = pContext->pszSearchC1DN;
            break;
        case 2:
            pszBase = pContext->pszSearchC2DN;
            break;
        default: assert(0);
            break;
    }

    if (! (dwCnt % VMDIR_SIZE_128))
    {
        pszBase = pContext->pszSearchC3DN;
    }

    dwError = VmDirAllocateStringPrintf(&record.pszCN, "%s%d", VMDIR_TEST_SEARCH_OBJECT_CN, dwCnt);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&record.pszDN, "cn=%s,%s", record.pszCN, pszBase);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestProvisionSetAttribute(&record);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestAddObject(pContext, &record);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    _TestFreeSearchTestRecordContent(&record);
    return dwError;
}

static
DWORD
_TestProvisionSearchObject(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;

    for (dwCnt=0; dwCnt<MAX_SEARCH_OBJECT; dwCnt++)
    {
        dwError = _TestAddSearchObject(pContext, dwCnt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}

DWORD
TestProvisionSearchSetup(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    dwError = _TestProvisionSearchContainer(pContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestProvisionSearchObject(pContext);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
TestProvisionSearchCleanup(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    if (!pContext->pTestState->bSkipCleanup)
    {
        dwError = VmDirTestDeleteContainerByDn(pContext->pTestState->pLd, pContext->pszSearchDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pContext->pszSearchC1DN);
    VMDIR_SAFE_FREE_MEMORY(pContext->pszSearchC2DN);
    VMDIR_SAFE_FREE_MEMORY(pContext->pszSearchDN);

    return dwError;

error:
    goto cleanup;
}

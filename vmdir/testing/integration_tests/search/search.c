/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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
_TestDefineScenario(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext
    );

static
DWORD
_TestExecuteSearchCase(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    );

DWORD
TestSearch(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    dwError = TestExecuteRestSearch(pContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestDefineScenario(pContext);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_TestDefineScenario(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;

    {
        VMDIR_SEARCH_TEST_CASE testTbl[] = TEST_SEARCH_CASE_1;

        for (dwCnt=0; dwCnt < sizeof(testTbl)/sizeof(testTbl[0]); dwCnt++)
        {
            dwError = _TestExecuteSearchCase(pContext, &testTbl[dwCnt]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        printf("TEST_SEARCH_CASE_1 succeeded\n");
        fflush(stdout);
    }

    {
        VMDIR_SEARCH_TEST_CASE testTbl[] = TEST_SEARCH_CASE_2;

        for (dwCnt=0; dwCnt < sizeof(testTbl)/sizeof(testTbl[0]); dwCnt++)
        {
            dwError = _TestExecuteSearchCase(pContext, &testTbl[dwCnt]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        printf("TEST_SEARCH_CASE_2 succeeded\n");
        fflush(stdout);
    }

    {
        VMDIR_SEARCH_TEST_CASE testTbl[] = TEST_SEARCH_CASE_NORMAL_USER_ITERATION_LIMIT;

        pContext->testLdapOwner = LDAP_OWNER_NORMAL_COMPUTER;
        for (dwCnt=0; dwCnt < sizeof(testTbl)/sizeof(testTbl[0]); dwCnt++)
        {
            dwError = _TestExecuteSearchCase(pContext, &testTbl[dwCnt]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        pContext->testLdapOwner = LDAP_OWNER_ADMIN;

        printf("TEST_SEARCH_CASE_NORMAL_USER_ITERATION_LIMIT succeeded\n");
        fflush(stdout);
    }

    {
        VMDIR_SEARCH_TEST_CASE testTbl[] = TEST_SEARCH_CASE_CUSTOMIZE_MAP;
        PCSTR   ppszAttrVals[] = {"vmwTestSearchCaseIgnoreStringNonunique:20", NULL};

        // customize iterator map
        dwError = VmDirTestAddAttributeValues(
                pContext->pTestState->pLd,
                CFG_ITERATION_MAP_DN,
                ATTR_ATTR_TYPE_PRI,
                ppszAttrVals);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (dwCnt=0; dwCnt < sizeof(testTbl)/sizeof(testTbl[0]); dwCnt++)
        {
            dwError = _TestExecuteSearchCase(pContext, &testTbl[dwCnt]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        printf("TEST_SEARCH_CASE_CUSTOMIZE_MAP succeeded\n");
        fflush(stdout);

        // restore customization
        dwError = VmDirTestDeleteAttributeValues(
                pContext->pTestState->pLd,
                CFG_ITERATION_MAP_DN,
                ATTR_ATTR_TYPE_PRI,
                ppszAttrVals);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}

static
DWORD
_TestExecuteSearchCase(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    )
{
    DWORD   dwError = 0;

    if (pCase->definition.dwPageSize > 0)
    {
        dwError = TestExecutePagedSearch(pContext, pCase);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = TestExecuteNormalSearch(pContext, pCase);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}

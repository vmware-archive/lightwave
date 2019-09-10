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

#define TEST_MULTI_VALUE_ATTR_USER  "mulivalueattrUser"
#define TEST_MULTI_VALUE_ATTR_VALUE_1  "value-1"
#define TEST_MULTI_VALUE_ATTR_VALUE_2  "value-2"
#define TEST_MULTI_VALUE_ATTR_VALUE_3  "value-3"
#define TEST_MULTI_VALUE_ATTR_VALUE_4  "value-4"
#define TEST_MULTI_VALUE_ATTR_VALUE_5  "value-5"
#define TEST_MULTI_VALUE_ATTR_VALUE_6  "value-6"


PCSTR   _pszNode1Set1[] = {TEST_MULTI_VALUE_ATTR_VALUE_1, NULL};
PCSTR   _pszNode1Set2[] = {TEST_MULTI_VALUE_ATTR_VALUE_3, TEST_MULTI_VALUE_ATTR_VALUE_5, NULL};
PCSTR   _pszNode2Set1[] = {TEST_MULTI_VALUE_ATTR_VALUE_2, NULL};
PCSTR   _pszNode2Set2[] = {TEST_MULTI_VALUE_ATTR_VALUE_4, TEST_MULTI_VALUE_ATTR_VALUE_6, NULL};

VOID
TestMultiValueAttrCleanup(
    PVMDIR_TEST_STATE pState
    )
{
    if (!pState->bSkipCleanup)
    {
        VmDirTestDeleteUser(pState, NULL, TEST_MULTI_VALUE_ATTR_USER);
    }

    return;
}

DWORD
TestMultiValueAttrSetup(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD   dwError = 0;

    dwError = VmDirTestCreateUser(pState, NULL, TEST_MULTI_VALUE_ATTR_USER, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_TestAddAttrValue(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszTestUserDN
    )
{
    DWORD   dwError = 0;

    // add value via node 1
    dwError = VmDirTestAddAttributeValues(
        pState->pLd,
        pszTestUserDN,
        ATTR_DESCRIPTION,
        _pszNode1Set1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestAddAttributeValues(
        pState->pLd,
        pszTestUserDN,
        ATTR_DESCRIPTION,
        _pszNode1Set2);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pState->pSecondLd)
    {
        VmDirSleep(2000);  // for test user to converge

        // add value via node 2
        dwError = VmDirTestAddAttributeValues(
            pState->pSecondLd,
            pszTestUserDN,
            ATTR_DESCRIPTION,
            _pszNode2Set1);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirTestAddAttributeValues(
            pState->pSecondLd,
            pszTestUserDN,
            ATTR_DESCRIPTION,
            _pszNode2Set2);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirSleep(2000);  // for test user to converge
    }

error:
    return dwError;
}

static
DWORD
_TestDeleteAttrValue(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszTestUserDN
    )
{
    DWORD   dwError = 0;

    // delete value via node 1
    dwError = VmDirTestDeleteAttributeValues(
        pState->pLd,
        pszTestUserDN,
        ATTR_DESCRIPTION,
        _pszNode1Set1);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pState->pSecondLd)
    {
        VmDirSleep(2000);  // for test user to converge

        // delete value via node 2
        dwError = VmDirTestDeleteAttributeValues(
            pState->pSecondLd,
            pszTestUserDN,
            ATTR_DESCRIPTION,
            _pszNode2Set1);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirSleep(2000);  // for test user to converge
    }

error:
    return dwError;
}

static
DWORD
_TestVerifyAddAndDeleteAttrValue(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszTestUserDN
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    BOOLEAN bIsListEqual = FALSE;
    PVMDIR_STRING_LIST  pResultList = NULL;

    PVMDIR_STRING_LIST  pNode1List = NULL;
    PVMDIR_STRING_LIST  pNode2List = NULL;

    dwError = VmDirStringListInitialize(&pResultList, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; _pszNode1Set2[dwCnt] != NULL; dwCnt++)
    {
        dwError = VmDirStringListAddStrClone(_pszNode1Set2[dwCnt],pResultList);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pState->pSecondLd)
    {
        for (dwCnt=0; _pszNode1Set2[dwCnt] != NULL; dwCnt++)
        {
            dwError = VmDirStringListAddStrClone(_pszNode2Set2[dwCnt], pResultList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    // get result
    dwError = VmDirTestGetEntryAttributeValuesInStr(
        pState->pLd,
        pszTestUserDN,
        LDAP_SCOPE_BASE,
        NULL,
        ATTR_DESCRIPTION,
        &pNode1List);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pState->pSecondLd)
    {
        dwError = VmDirTestGetEntryAttributeValuesInStr(
            pState->pSecondLd,
            pszTestUserDN,
            LDAP_SCOPE_BASE,
            NULL,
            ATTR_DESCRIPTION,
            &pNode2List);
        BAIL_ON_VMDIR_ERROR(dwError);

        bIsListEqual = VmDirStringListEqualsNoOrder(pNode1List, pNode2List, FALSE);
        TestAssert(bIsListEqual);
    }

    bIsListEqual = VmDirStringListEqualsNoOrder(pNode1List, pResultList, FALSE);
    TestAssert(bIsListEqual);

cleanup:
    VmDirStringListFree(pResultList);
    VmDirStringListFree(pNode1List);
    VmDirStringListFree(pNode2List);

    return dwError;

error:
    goto cleanup;
}

DWORD
TestMultiValueAttr(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD   dwError = 0;
    PSTR    pszTestUserDN = NULL;


    printf("Testing multi-value attribute ...\n");

    dwError = VmDirAllocateStringPrintf(
                &pszTestUserDN,
                "cn=%s,cn=users,%s",
                TEST_MULTI_VALUE_ATTR_USER,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestAddAttrValue(pState, pszTestUserDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestDeleteAttrValue(pState, pszTestUserDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestVerifyAddAndDeleteAttrValue(pState, pszTestUserDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("multi-value attribute tests succeded!\n");
    fflush(stdout);

cleanup:
    return dwError;

error:
    printf("multi-value attribute tests failed with error 0n%d\n", dwError);
    goto cleanup;
}

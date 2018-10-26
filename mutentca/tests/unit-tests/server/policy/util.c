/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

LWCA_POLICY_CFG_TYPE type1 = LWCA_POLICY_CFG_TYPE_IP;
LWCA_POLICY_CFG_MATCH match1 = LWCA_POLICY_CFG_MATCH_CONSTANT;
#define TEST_VALUE "TestValue"

LWCA_POLICY_CFG_TYPE type2 = LWCA_POLICY_CFG_TYPE_NAME;
LWCA_POLICY_CFG_MATCH match2 = LWCA_POLICY_CFG_MATCH_ANY;

BOOLEAN bMultiSANEnabled = TRUE;
#define KEY_USAGE 255
#define CERT_DURATION 365

VOID
Test_LwCAPolicyCfgObjInit(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_POLICY_CFG_OBJ pObj = NULL;

    dwError = LwCAPolicyCfgObjInit(type1, match1, TEST_VALUE, &pObj);
    assert_int_equal(dwError, 0);
    assert_non_null(pObj);

    LwCAPolicyCfgObjFree(pObj);
}

VOID
Test_LwCAPolicyCfgObjArrayInit(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_POLICY_CFG_OBJ pObj = NULL;
    PLWCA_POLICY_CFG_OBJ_ARRAY pObjArray = NULL;

    dwError = LwCAPolicyCfgObjInit(type1, match1, TEST_VALUE, &pObj);
    assert_int_equal(dwError, 0);
    assert_non_null(pObj);

    dwError = LwCAPolicyCfgObjArrayInit(&pObj, 1, &pObjArray);
    assert_int_equal(dwError, 0);
    assert_non_null(pObjArray);

    LwCAPolicyCfgObjArrayFree(pObjArray);
}

VOID
Test_LwCAPoliciesInit(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_POLICY_CFG_OBJ pObj1 = NULL;
    PLWCA_POLICY_CFG_OBJ pObj2 = NULL;
    PLWCA_POLICY_CFG_OBJ_ARRAY pSNs = NULL;
    PLWCA_POLICY_CFG_OBJ_ARRAY pSANs = NULL;
    PLWCA_POLICIES pPolicies = NULL;

    dwError = LwCAPolicyCfgObjInit(type1, match1, TEST_VALUE, &pObj1);
    assert_int_equal(dwError, 0);
    assert_non_null(pObj1);

    dwError = LwCAPolicyCfgObjArrayInit(&pObj1, 1, &pSNs);
    assert_int_equal(dwError, 0);
    assert_non_null(pSNs);

    dwError = LwCAPolicyCfgObjInit(type2, match2, NULL, &pObj2);
    assert_int_equal(dwError, 0);
    assert_non_null(pObj2);

    dwError = LwCAPolicyCfgObjArrayInit(&pObj2, 1, &pSANs);
    assert_int_equal(dwError, 0);
    assert_non_null(pSANs);

    dwError = LwCAPoliciesInit(bMultiSANEnabled, pSNs, pSANs, KEY_USAGE, CERT_DURATION, &pPolicies);
    assert_int_equal(dwError, 0);
    assert_non_null(pPolicies);

    LwCAPoliciesFree(pPolicies);
}

VOID
Test_LwCAPoliciesInit_Empty(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_POLICIES pPolicies = NULL;

    dwError = LwCAPoliciesInit(FALSE, NULL, NULL, 0, 0, &pPolicies);
    assert_int_equal(dwError, 0);
    assert_non_null(pPolicies);

    LwCAPoliciesFree(pPolicies);
}

VOID
Test_LwCAPolicyCfgObjCopy(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_POLICY_CFG_OBJ pObj = NULL;
    PLWCA_POLICY_CFG_OBJ pObjCopy = NULL;

    dwError = LwCAPolicyCfgObjInit(type1, match1, TEST_VALUE, &pObj);
    assert_int_equal(dwError, 0);
    assert_non_null(pObj);

    dwError = LwCAPolicyCfgObjCopy(pObj, &pObjCopy);
    assert_int_equal(dwError, 0);
    assert_non_null(pObjCopy);
    assert_int_equal(pObjCopy->type, type1);
    assert_int_equal(pObjCopy->match, match1);
    assert_string_equal(pObjCopy->pszValue, TEST_VALUE);

    LwCAPolicyCfgObjFree(pObj);
    LwCAPolicyCfgObjFree(pObjCopy);
}

VOID
Test_LwCAPolicyCfgObjArrayCopy(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_POLICY_CFG_OBJ pObj = NULL;
    PLWCA_POLICY_CFG_OBJ_ARRAY pObjArray = NULL;
    PLWCA_POLICY_CFG_OBJ_ARRAY pObjArrayCopy = NULL;

    dwError = LwCAPolicyCfgObjInit(type1, match1, TEST_VALUE, &pObj);
    assert_int_equal(dwError, 0);
    assert_non_null(pObj);

    dwError = LwCAPolicyCfgObjArrayInit(&pObj, 1, &pObjArray);
    assert_int_equal(dwError, 0);
    assert_non_null(pObjArray);

    dwError = LwCAPolicyCfgObjArrayCopy(pObjArray, &pObjArrayCopy);
    assert_int_equal(dwError, 0);
    assert_non_null(pObjArrayCopy);
    assert_int_equal(pObjArrayCopy->dwCount, 1);
    assert_int_equal(pObjArrayCopy->ppObj[0]->type, type1);
    assert_int_equal(pObjArrayCopy->ppObj[0]->match, match1);
    assert_string_equal(pObjArrayCopy->ppObj[0]->pszValue, TEST_VALUE);

    LwCAPolicyCfgObjArrayFree(pObjArray);
    LwCAPolicyCfgObjArrayFree(pObjArrayCopy);
}

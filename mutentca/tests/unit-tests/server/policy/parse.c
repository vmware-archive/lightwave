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

static
VOID
_Test_LwCAPolicyInitCtx_Invalid(
    PCSTR pcszJson
    );

VOID
Test_LwCAPolicyInitCtx_Valid_AllCombinations(
    VOID **state
    )
{
    DWORD dwError = 0;
    json_error_t jsonError = {0};
    PLWCA_JSON_OBJECT pJson = NULL;
    PLWCA_POLICY_CONTEXT pPolicyCtx = NULL;

    pJson = json_loads(TEST_POLICY_JSON_VALID_ALL_COMBINATIONS, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    dwError = LwCAPolicyInitCtx(pJson, &pPolicyCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pPolicyCtx);
    assert_non_null(pPolicyCtx->pCAPoliciesAllowed);
    assert_non_null(pPolicyCtx->pCertPoliciesAllowed);

    LwCAPolicyFreeCtx(pPolicyCtx);
    LwCAJsonCleanupObject(pJson);
}

VOID
Test_LwCAPolicyInitCtx_Valid_OnlyCertPolicy(
    VOID **state
    )
{
    DWORD dwError = 0;
    json_error_t jsonError = {0};
    PLWCA_JSON_OBJECT pJson = NULL;
    PLWCA_POLICY_CONTEXT pPolicyCtx = NULL;

    pJson = json_loads(TEST_POLICY_JSON_VALID_ONLY_CERTPOLICY, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    dwError = LwCAPolicyInitCtx(pJson, &pPolicyCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pPolicyCtx);
    assert_null(pPolicyCtx->pCAPoliciesAllowed);
    assert_non_null(pPolicyCtx->pCertPoliciesAllowed);

    LwCAPolicyFreeCtx(pPolicyCtx);
    LwCAJsonCleanupObject(pJson);
}

VOID
Test_LwCAPolicyInitCtx_Valid_OnlyCAPolicy(
    VOID **state
    )
{
    DWORD dwError = 0;
    json_error_t jsonError = {0};
    PLWCA_JSON_OBJECT pJson = NULL;
    PLWCA_POLICY_CONTEXT pPolicyCtx = NULL;

    pJson = json_loads(TEST_POLICY_JSON_VALID_ONLY_CAPOLICY, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    dwError = LwCAPolicyInitCtx(pJson, &pPolicyCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pPolicyCtx);
    assert_non_null(pPolicyCtx->pCAPoliciesAllowed);
    assert_null(pPolicyCtx->pCertPoliciesAllowed);

    LwCAPolicyFreeCtx(pPolicyCtx);
    LwCAJsonCleanupObject(pJson);
}

VOID
Test_LwCAPolicyInitCtx_Invalid_Type(
    VOID **state
    )
{
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_TYPE );
}

VOID
Test_LwCAPolicyInitCtx_Invalid_Match(
    VOID **state
    )
{
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_MATCH );
}

VOID
Test_LwCAPolicyInitCtx_Invalid_TypeMatchCombo(
    VOID **state
    )
{
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO1 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO2 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO3 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO4 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO5 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO6 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO7 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO8 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_TYPE_MATCH_COMBO9 );
}

VOID
Test_LwCAPolicyInitCtx_Invalid_MissingValueForTypeMatchCombo(
    VOID **state
    )
{
    // Missing key 'type'
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_MISSING_TYPE );

    // Missing key 'match'
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_MISSING_MATCH );

    // Missing key 'value' required for specific type-match combinations
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_MISSING_VALUE1 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_MISSING_VALUE2 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_MISSING_VALUE3 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_MISSING_VALUE4 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_MISSING_VALUE5 );
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_MISSING_VALUE6 );
}

VOID
Test_LwCAPolicyInitCtx_Invalid_KeyUsage(
    VOID **state
    )
{
    // KeyUsagePolicy value > 511
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_KEYUSAGE1 );

    // KeyUsagePolicyValue < 0
    _Test_LwCAPolicyInitCtx_Invalid( TEST_POLICY_JSON_INVALID_KEYUSAGE2 );
}

static
VOID
_Test_LwCAPolicyInitCtx_Invalid(
    PCSTR pcszJson
    )
{
    DWORD dwError = 0;
    json_error_t jsonError = {0};
    PLWCA_JSON_OBJECT pJson = NULL;
    PLWCA_POLICY_CONTEXT pPolicyCtx = NULL;

    pJson = json_loads(pcszJson, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    dwError = LwCAPolicyInitCtx(pJson, &pPolicyCtx);
    assert_int_equal(dwError, LWCA_POLICY_CONFIG_PARSE_ERROR);
    assert_null(pPolicyCtx);

    LwCAPolicyFreeCtx(pPolicyCtx);
    LwCAJsonCleanupObject(pJson);
}

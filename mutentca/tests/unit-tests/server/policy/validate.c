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

#define DUMMY_UPN "dummy@lightwave.local"
#define DUMMY_UPN2 "est.lw.local@lw.local"

PLWCA_POLICY_CONTEXT pPolicyCtx = NULL;
PLWCA_REQ_CONTEXT pReqCtx = NULL;
X509_REQ *pReq = NULL;

static
VOID
__wrap_LwCAPolicyValidateSetup(
    PCSTR pcszJson,
    PCSTR pcszBindUPN,
    PCSTR pcszCSR
    );

static
VOID
__wrap_LwCAPolicyValidateCleanup(
    );

VOID
Test_LwCAPolicyValidate_SN_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsValid = FALSE;

    __wrap_LwCAPolicyValidateSetup(TEST_SN_JSON_VALID1, DUMMY_UPN, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_SN,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_true(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();

    __wrap_LwCAPolicyValidateSetup(TEST_SN_JSON_VALID2, DUMMY_UPN2, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_SN,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_true(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();

    __wrap_LwCAPolicyValidateSetup(TEST_SN_JSON_VALID3, DUMMY_UPN, TEST_CSR2);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_SN,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_true(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();
}

VOID
Test_LwCAPolicyValidate_SN_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsValid = FALSE;

    __wrap_LwCAPolicyValidateSetup(TEST_SN_JSON_INVALID1, DUMMY_UPN, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_SN,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_false(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();

    __wrap_LwCAPolicyValidateSetup(TEST_SN_JSON_INVALID2, DUMMY_UPN2, TEST_CSR2);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_SN,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_false(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();
}

VOID
Test_LwCAPolicyValidate_SAN_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsValid = FALSE;

    __wrap_LwCAPolicyValidateSetup(TEST_SAN_JSON_VALID1, DUMMY_UPN, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_SAN,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_true(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();

    __wrap_LwCAPolicyValidateSetup(TEST_SAN_JSON_VALID2, DUMMY_UPN, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_SAN,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_true(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();

    __wrap_LwCAPolicyValidateSetup(TEST_SAN_JSON_VALID3, DUMMY_UPN2, TEST_CSR2);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_SAN,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_true(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();
}

VOID
Test_LwCAPolicyValidate_SAN_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsValid = FALSE;

    __wrap_LwCAPolicyValidateSetup(TEST_SAN_JSON_INVALID1, DUMMY_UPN, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_SAN,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_false(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();

    __wrap_LwCAPolicyValidateSetup(TEST_SAN_JSON_INVALID2, DUMMY_UPN, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_SAN,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_false(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();

    __wrap_LwCAPolicyValidateSetup(TEST_SAN_JSON_INVALID3, DUMMY_UPN, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_SAN,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_false(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();
}

VOID
Test_LwCAPolicyValidate_KeyUsage_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsValid = FALSE;

    __wrap_LwCAPolicyValidateSetup(TEST_KEY_USAGE_JSON_VALID1, DUMMY_UPN, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_KEY_USAGE,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_true(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();
}

VOID
Test_LwCAPolicyValidate_KeyUsage_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsValid = FALSE;

    __wrap_LwCAPolicyValidateSetup(TEST_KEY_USAGE_JSON_INVALID1, DUMMY_UPN, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_KEY_USAGE,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_false(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();
}

VOID
Test_LwCAPolicyValidate_CertDuration_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERT_VALIDITY pValidity = NULL;
    BOOLEAN bIsValid = FALSE;
    time_t tmNotBefore = 0;
    time_t tmNotAfter = 0;

    tmNotBefore = time(NULL);
    tmNotAfter = tmNotBefore + LWCA_TIME_SECS_PER_WEEK;

    dwError = LwCACreateCertValidity(tmNotBefore, tmNotAfter, &pValidity);
    assert_int_equal(dwError, 0);
    assert_non_null(pValidity);

    __wrap_LwCAPolicyValidateSetup(TEST_DURATION_JSON_VALID1, DUMMY_UPN, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, pValidity,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_DURATION,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_true(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();

    LwCAFreeCertValidity(pValidity);

    __wrap_LwCAPolicyValidateSetup(TEST_DURATION_JSON_VALID1, DUMMY_UPN, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, NULL,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_DURATION,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_true(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();
}

VOID
Test_LwCAPolicyValidate_CertDuration_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERT_VALIDITY pValidity = NULL;
    BOOLEAN bIsValid = FALSE;
    time_t tmNotBefore = 0;
    time_t tmNotAfter = 0;

    tmNotBefore = time(NULL);
    tmNotAfter = tmNotBefore + LWCA_TIME_SECS_PER_WEEK;

    dwError = LwCACreateCertValidity(tmNotBefore, tmNotAfter, &pValidity);
    assert_int_equal(dwError, 0);
    assert_non_null(pValidity);

    __wrap_LwCAPolicyValidateSetup(TEST_DURATION_JSON_INVALID1, DUMMY_UPN, TEST_CSR1);

    dwError = LwCAPolicyValidate(
                pPolicyCtx, pReqCtx, pReq, pValidity,
                LWCA_POLICY_TYPE_CERTIFICATE,
                LWCA_POLICY_CHECK_DURATION,
                &bIsValid);
    assert_int_equal(dwError, 0);
    assert_false(bIsValid);

    __wrap_LwCAPolicyValidateCleanup();

}

static
VOID
__wrap_LwCAPolicyValidateSetup(
    PCSTR pcszJson,
    PCSTR pcszBindUPN,
    PCSTR pcszCSR
    )
{
    DWORD dwError = 0;
    json_error_t jsonError = {0};
    PLWCA_JSON_OBJECT pJson = NULL;

    // Initializing policy context
    pJson = json_loads(pcszJson, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    dwError = LwCAPolicyInitCtx(pJson, &pPolicyCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pPolicyCtx);

    LwCAJsonCleanupObject(pJson);

    // Initializing request context
    dwError = LwCAAllocateMemory(sizeof(LWCA_REQ_CONTEXT), (PVOID *)&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAAllocateStringA(pcszBindUPN, &pReqCtx->pszBindUPN);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx->pszBindUPN);

    // Converting certificate signing request to X509_REQ
    dwError = LwCAPEMToX509Req(pcszCSR, &pReq);
    assert_int_equal(dwError, 0);
    assert_non_null(pReq);
}

static
VOID
__wrap_LwCAPolicyValidateCleanup(
    )
{
    LwCAPolicyFreeCtx(pPolicyCtx);
    LwCARequestContextFree(pReqCtx);
    LwCAX509ReqFree(pReq);
}

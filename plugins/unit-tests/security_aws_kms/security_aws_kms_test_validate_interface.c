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

VOID
Security_Aws_Kms_Tests_Validate_Interface(
    VOID **state
    )
{
    PSECURITY_AWS_KMS_TEST_STATE pState = *state;

    assert_non_null(pState && pState->pInterface);

    assert_non_null(pState->pInterface->pFnInitialize);
    assert_non_null(pState->pInterface->pFnGetCaps);
    assert_non_null(pState->pInterface->pFnCapOverride);
    assert_non_null(pState->pInterface->pFnAddKeyPair);
    assert_non_null(pState->pInterface->pFnCreateKeyPair);
    assert_non_null(pState->pInterface->pFnSignCertificate);
    assert_non_null(pState->pInterface->pFnVerifyCertificate);
    assert_non_null(pState->pInterface->pFnGetErrorString);
    assert_non_null(pState->pInterface->pFnCloseHandle);
    assert_non_null(pState->pInterface->pFnFreeMemory);
}

VOID
Security_Aws_Kms_Tests_Validate_Interface_Cleared(
    VOID **state
    )
{
    PSECURITY_AWS_KMS_TEST_STATE pState = *state;

    assert_non_null(pState && pState->pInterface);

    assert_null(pState->pInterface->pFnInitialize);
    assert_null(pState->pInterface->pFnGetCaps);
    assert_null(pState->pInterface->pFnCapOverride);
    assert_null(pState->pInterface->pFnAddKeyPair);
    assert_null(pState->pInterface->pFnCreateKeyPair);
    assert_null(pState->pInterface->pFnSignCertificate);
    assert_null(pState->pInterface->pFnVerifyCertificate);
    assert_null(pState->pInterface->pFnGetErrorString);
    assert_null(pState->pInterface->pFnCloseHandle);
    assert_null(pState->pInterface->pFnFreeMemory);
}

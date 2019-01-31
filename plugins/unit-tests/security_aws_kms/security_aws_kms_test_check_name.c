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

int
Security_Aws_Kms_Tests_Get_Name (
    void **state
    )
{
    int error = 0;
    PFN_LWCA_SECURITY_GET_NAME pFnGetName = NULL;
    PSECURITY_AWS_KMS_TEST_STATE pState = *state;

    pFnGetName = dlsym(pState->module, LWCA_FN_NAME_SECURITY_GET_NAME);
    if (!pFnGetName)
    {
        fail_msg("dlsym failed LwCASecurityGetName: %s\n", dlerror());
        goto error;
    }

    pState->pszName = pFnGetName();

error:
    return error;
}

VOID
Security_Aws_Kms_Tests_Check_Name (
    void **state
    )
{
    PSECURITY_AWS_KMS_TEST_STATE pState = *state;

    PCSTR pszExpectedName = "lwca_security_aws_kms";

    assert_string_equal(pState->pszName, pszExpectedName);
}

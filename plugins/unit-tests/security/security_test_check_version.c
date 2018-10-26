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
Security_Tests_Get_Version (
    void **state
    )
{
    int error = 0;
    PFN_LWCA_SECURITY_GET_VERSION pFnGetVersion = NULL;
    PSECURITY_TEST_STATE pState = *state;

    pFnGetVersion = dlsym(pState->module, LWCA_FN_NAME_SECURITY_GET_VERSION);
    if (!pFnGetVersion)
    {
        fail_msg("dlsym failed LwCASecurityGetVersion: %s\n", dlerror());
        goto error;
    }

    pState->pszVersion = pFnGetVersion();

error:
    return error;
}

VOID
Security_Tests_Check_Version (
    void **state
    )
{
    PSECURITY_TEST_STATE pState = *state;

    PCSTR pszExpectedVersion = LWCA_SECURITY_VERSION_MAJOR"."\
LWCA_SECURITY_VERSION_MINOR"."\
LWCA_SECURITY_VERSION_RELEASE;

    assert_string_equal(pState->pszVersion, pszExpectedVersion);
}

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
Security_Tests_Check_Caps(
    void **state
    )
{
    LWCA_SECURITY_CAP nCaps = LWCA_SECURITY_CAP_NONE;
    PSECURITY_TEST_STATE pState = *state;

    if (Security_Tests_Initialize(state))
    {
        goto error;
    }

    if (pState->pInterface->pFnGetCaps(pState->pHandle, &nCaps))
    {
        fail_msg("failed getting caps\n");
    }

    assert_int_equal(LWCA_SECURITY_CAP_HAS_ALL(nCaps), 1);
error:
    return;
}

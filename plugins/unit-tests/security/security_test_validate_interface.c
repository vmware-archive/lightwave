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
Security_Tests_Validate_Interface(
    void **state
    )
{
    PSECURITY_TEST_STATE pState = *state;

    if (!pState->pInterface)
    {
        fail_msg("Interface not loaded. pInterface = NULL\n");
        goto error;
    }

    if (!pState->pInterface->pFnInitialize ||
        !pState->pInterface->pFnGetCaps ||
        !pState->pInterface->pFnCapOverride ||
        !pState->pInterface->pFnAddKeyPair ||
        !pState->pInterface->pFnCreateKeyPair ||
        !pState->pInterface->pFnSign ||
        !pState->pInterface->pFnVerify ||
        !pState->pInterface->pFnCloseHandle ||
        !pState->pInterface->pFnFreeMemory)
    {
        fail_msg("Interface table is not valid. missing entries\n");
    }

error:
    return;
}

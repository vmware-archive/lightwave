/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

VMDIR_SEARCH_TEST_CONTEXT _gSearchContext = {0};

DWORD
TestSetup(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_SEARCH_TEST_CONTEXT pContext = NULL;

    pContext = pState->pContext = &_gSearchContext;
    _gSearchContext.pTestState = pState;

    dwError = TestProvisionSearchSetup(pContext);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
TestCleanup(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_SEARCH_TEST_CONTEXT pContext = NULL;

    pContext = pState->pContext = &_gSearchContext;
    _gSearchContext.pTestState = pState;

    dwError = TestProvisionSearchCleanup(pContext);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
TestRunner(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_SEARCH_TEST_CONTEXT pContext = NULL;

    pContext = pState->pContext = &_gSearchContext;
    _gSearchContext.pTestState = pState;

    printf("Starting search-related tests ...\n");

    dwError = TestSearch(pContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestAbormalPagedSearch(pContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Search tests completed successfully.\n");
    fflush(stdout);

cleanup:
    return dwError;

error:
    goto cleanup;
}

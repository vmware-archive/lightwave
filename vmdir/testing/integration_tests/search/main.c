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

DWORD
TestSetup(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = PagedSearchTestSetup(pState);
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

    dwError = PagedSearchTestCleanup(pState);
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

    printf("Starting search-related tests ...\n");
    dwError = TestAnonymousSearch(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestPagedSearch(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Search tests completed successfully.\n");

cleanup:
    return dwError;
error:
goto cleanup;
}

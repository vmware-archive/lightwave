/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

//
// (1) Get password and store it so we can restore it later.
// (2) Set DC password to a known string
// (3) Read the DC password and make sure it comports with our sentinel value.
// (4) Set the DC password to the original value.
// (5) Check that the old password is set to our sentinel value.
//
DWORD
TestDCAccountPasswordCode(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszOldPassword = NULL;
    PSTR pszOldPassword2 = NULL;
    PSTR pszOldPassword3 = NULL;
    PCSTR pszSentinelPassword = "My first password";
    PSTR pszPassword = NULL;

    dwError = VmDirReadDCAccountPassword(&pszOldPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirWriteDCAccountOldPassword(
                pszOldPassword,
                strlen(pszOldPassword));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirReadDCAccountOldPassword(&pszOldPassword3);
    BAIL_ON_VMDIR_ERROR(dwError);

    TestAssertStrEquals(pszOldPassword, pszOldPassword3);

    dwError = VmDirWriteDCAccountPassword(
                pszSentinelPassword,
                strlen(pszSentinelPassword));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirReadDCAccountPassword(&pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    TestAssertStrEquals(pszSentinelPassword, pszPassword);

    //
    // Restore password.
    //
    dwError = VmDirWriteDCAccountPassword(
                pszOldPassword,
                strlen(pszOldPassword));
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Registry integration tests passed\n");
cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszOldPassword);
    VMDIR_SAFE_FREE_STRINGA(pszOldPassword2);
    VMDIR_SAFE_FREE_STRINGA(pszPassword);

    return dwError;

error:
    printf("registry integration tests failed with error 0n%d\n", dwError);
    goto cleanup;
}

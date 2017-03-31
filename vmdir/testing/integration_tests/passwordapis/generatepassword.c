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

DWORD
TestGeneratePassword(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PBYTE pBytes = NULL;
    DWORD dwByteCount = 0;

   dwError = VmDirGeneratePassword(NULL, NULL, NULL, &pBytes, &dwByteCount);
   TestAssert(dwError == 0);
   TestAssert(pBytes != NULL);
   TestAssert(dwByteCount != 0);
   BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pBytes);
    return dwError;
error:
    goto cleanup;
}

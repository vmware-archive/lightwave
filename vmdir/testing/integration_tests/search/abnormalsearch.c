/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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

VMDIR_SEARCH_TEST_CASE _gAbnormalTestTbl[] = TEST_SEARCH_CASE_ABNORMAL;

DWORD
TestLongRunningSearchSucceeds(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    VMDIR_SEARCH_OP_CONTEXT    opCtx = {0};
    VDIR_SEARCH_EXEC_PATH      searchExecPath = {0};

    opCtx.bPaged = TRUE;
    opCtx.dwPageSize = _gAbnormalTestTbl[0].definition.dwPageSize;

    while (!opCtx.bDone)
    {
        dwError = TestSendSearch(pContext, &_gAbnormalTestTbl[0], &opCtx, &searchExecPath);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirSleep(5 * 1000); // pause 5 seconds
    }

    searchExecPath.iEntrySent = opCtx.dwTotalResultCount;

    dwError = TestValidateSearchResult(&_gAbnormalTestTbl[0], &opCtx, &searchExecPath);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    TestFreeSearchOpCtxContent(&opCtx);
    VmDirSearchExecPathFreeContent(&searchExecPath);

    return dwError;

error:
    goto cleanup;
}

DWORD
TestStaleCookie(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    PVMDIR_TEST_STATE          pState = pContext->pTestState;
    VMDIR_SEARCH_OP_CONTEXT    opCtx = {0};
    VMDIR_SEARCH_OP_CONTEXT    opCtxBad = {0};
    VDIR_SEARCH_EXEC_PATH      searchExecPath = {0};
    struct berval              priorCookie = {0};

    opCtx.bPaged = TRUE;
    opCtx.dwPageSize = _gAbnormalTestTbl[0].definition.dwPageSize;

    while (!opCtx.bDone)
    {
        dwError = TestSendSearch(pContext, &_gAbnormalTestTbl[0], &opCtx, &searchExecPath);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!priorCookie.bv_val)
        {
            dwError = VmDirAllocateStringA(opCtx.pbvCookie->bv_val, &priorCookie.bv_val);
            BAIL_ON_VMDIR_ERROR(dwError);

            priorCookie.bv_len = VmDirStringLenA(priorCookie.bv_val);
        }
    }

    searchExecPath.iEntrySent = opCtx.dwTotalResultCount;

    dwError = TestValidateSearchResult(&_gAbnormalTestTbl[0], &opCtx, &searchExecPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    // reset opCtx but keep prior cookies
    opCtxBad.bPaged = TRUE;
    opCtxBad.dwPageSize = _gAbnormalTestTbl[0].definition.dwPageSize;
    opCtxBad.pbvCookie = &priorCookie;

    // server return LDAP_PROTOCOL_ERROR for invalid cookie
    dwError = TestSendSearch(pContext, &_gAbnormalTestTbl[0], &opCtxBad, &searchExecPath);
    TestAssertEquals(dwError, LDAP_PROTOCOL_ERROR);
    dwError = 0;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(priorCookie.bv_val);
    TestFreeSearchOpCtxContent(&opCtx);
    VmDirSearchExecPathFreeContent(&searchExecPath);

    return dwError;

error:
    goto cleanup;
}

DWORD
TestAbormalPagedSearch(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    )
{
    DWORD dwError = 0;

    dwError = TestStaleCookie(pContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestLongRunningSearchSucceeds(pContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("TEST_SEARCH_CASE_ABNORMAL succeeded ...\n");
    fflush(stdout);

cleanup:
    return dwError;

error:
    goto cleanup;
}

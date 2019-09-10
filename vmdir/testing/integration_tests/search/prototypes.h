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

DWORD
TestAnonymousSearch(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    );

DWORD
TestAbormalPagedSearch(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    );

DWORD
TestProvisionSearchSetup(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    );

DWORD
TestProvisionSearchCleanup(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    );

DWORD
TestLdapParseSearchPlanControl(
    LDAPControl**          ppCtrls,
    PVDIR_SEARCH_EXEC_PATH pExecPath
    );

DWORD
TestSearch(
    PVMDIR_SEARCH_TEST_CONTEXT pContext
    );

DWORD
TestSendSearch(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase,
    PVMDIR_SEARCH_OP_CONTEXT    pOpCtx,
    PVDIR_SEARCH_EXEC_PATH      pSearchExecPath
    );

DWORD
TestValidateSearchResult(
    PVMDIR_SEARCH_TEST_CASE     pCase,
    PVMDIR_SEARCH_OP_CONTEXT    pOpCtx,
    PVDIR_SEARCH_EXEC_PATH      pSearchExecPath
    );

DWORD
TestExecuteNormalSearch(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    );

DWORD
TestExecutePagedSearch(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    );

VOID
TestFreeSearchOpCtxContent(
    PVMDIR_SEARCH_OP_CONTEXT pOpCtx
    );

DWORD
TestExecuteRestSearch(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext
    );

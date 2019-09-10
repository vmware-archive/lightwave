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

// not a public function?
int
ldap_parse_page_control(
    LDAP *          pLd,
    LDAPControl **  ctrls,
    ber_int_t *     countp,
    struct berval **cookiep
    );

DWORD
TestSendSearch(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase,
    PVMDIR_SEARCH_OP_CONTEXT    pOpCtx,
    PVDIR_SEARCH_EXEC_PATH      pSearchExecPath
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    LDAP*   pLocalLdap = NULL;
    int         errCode = 0;
    LDAPMessage *pResult = NULL;
    LDAPControl *pInSearchPlanCtrl = NULL;
    LDAPControl *pInPageCtrl = NULL;
    LDAPControl *pServerCtrl[] = { NULL, NULL, NULL };
    LDAPControl** ppResultctrls = NULL;
    LDAPControl* pOutSearchPlanCtrl = NULL;

    pLocalLdap = VmDirTestGetLdapOwner(
        pContext->pTestState,
        pContext->testLdapOwner);

    dwError = ldap_control_create(
            LDAP_SEARCH_PLAN_CONTROL,
            0,
            NULL,
            0,
            &pInSearchPlanCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    pServerCtrl[0] = pInSearchPlanCtrl;

    if (pOpCtx->bPaged)
    {
        dwError = ldap_create_page_control(
                pLocalLdap,
                pOpCtx->dwPageSize,
                pOpCtx->pbvCookie,
                0,
                &pInPageCtrl);
        BAIL_ON_VMDIR_ERROR(dwError);

        pServerCtrl[1] = pInPageCtrl;
    }

    dwError = VmDirAllocateStringPrintf(
            &pszDN, "%s,%s", pCase->definition.pszBaseDN, pContext->pTestState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
            pLocalLdap,
            pszDN,
            pCase->definition.iScope,
            pCase->definition.pszFilter,
            NULL,
            0,
            pServerCtrl,
            NULL,
            NULL,
            pCase->definition.iSizeLimit,
            &pResult);
    if (dwError == LDAP_UNWILLING_TO_PERFORM)
    {
        pOpCtx->bDone = TRUE;
        pOpCtx->dwResultCode = dwError;
        dwError = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_parse_result(
            pLocalLdap,
            pResult,
            &errCode,
            NULL,
            NULL,
            NULL,
            &ppResultctrls,
            0);
    BAIL_ON_VMDIR_ERROR(dwError);

    pOutSearchPlanCtrl = ldap_control_find(LDAP_SEARCH_PLAN_CONTROL, ppResultctrls, NULL);
    if (pOutSearchPlanCtrl)
    {
        dwError = VmDirParseSearchPlanControlContent(
                pOutSearchPlanCtrl,
                pSearchExecPath);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pOpCtx->dwResultCount = ldap_count_entries(pLocalLdap, pResult);

    if (pOpCtx->bPaged)
    {
        ber_int_t iPagedReturnCnt = 0;

        if (pOpCtx->pbvCookie)
        {
            ber_bvfree(pOpCtx->pbvCookie);
            pOpCtx->pbvCookie = NULL;
        }

        dwError = ldap_parse_page_control(
                pLocalLdap,
                ppResultctrls,
                &iPagedReturnCnt,
                &pOpCtx->pbvCookie);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pOpCtx->pbvCookie->bv_val == NULL ||
            (VmDirStringLenA(pOpCtx->pbvCookie->bv_val) == 0))
        {
            pOpCtx->bDone = TRUE;
        }

        pOpCtx->dwTotalResultCount += pOpCtx->dwResultCount;
    }
    else
    {
        pOpCtx->dwTotalResultCount = pOpCtx->dwResultCount;
    }

cleanup:
    if (ppResultctrls)
    {
        ldap_controls_free(ppResultctrls);
    }

    if (pInPageCtrl)
    {
        ldap_control_free(pInPageCtrl);
    }

    if (pInSearchPlanCtrl)
    {
        ldap_control_free(pInSearchPlanCtrl);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error:
    pOpCtx->dwResultCode = dwError;

    goto cleanup;
}

DWORD
TestValidateSearchResult(
    PVMDIR_SEARCH_TEST_CASE     pCase,
    PVMDIR_SEARCH_OP_CONTEXT    pOpCtx,
    PVDIR_SEARCH_EXEC_PATH      pSearchExecPath
    )
{
    DWORD   dwError = 0;
    BOOLEAN bExceedMaxIteration = pSearchExecPath->bExceedMaxIteration ? TRUE:FALSE;

    if (pCase->result.bExceedMaxIteation != bExceedMaxIteration)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_TEST_SEARCH_ERROR_SCAN_LIMIT);
    }

    if (pCase->result.algo != pSearchExecPath->searchAlgo)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_TEST_SEARCH_ERROR_ALGORITHM);
    }

    if (pCase->result.bExceedMaxIteation && pOpCtx->dwResultCode == LDAP_UNWILLING_TO_PERFORM)
    {   // failed ldap result (53) case, no further validate for now.
        goto cleanup;
    }

    if (pCase->result.iNumEntryReceived != pSearchExecPath->iEntrySent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_TEST_SEARCH_ERROR_TOTAL_ENTRY);
    }

    if (VmDirStringLenA(pCase->result.pszIndex) > 0 &&
        (!pSearchExecPath->pszIndex ||
         VmDirStringCompareA(pCase->result.pszIndex, pSearchExecPath->pszIndex, FALSE) != 0)
       )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_TEST_SEARCH_ERROR_INDEX_TABLE);
    }

cleanup:
    return dwError;

error:
    printf("failed test desc: %s ...\n", pCase->definition.pszDesc);
    printf("failed test result: algo %d received %d, exceed iter %d, paged %d, pagedone %d, CL size %d, table %s, iter %d\n",
            pSearchExecPath->searchAlgo,
            pSearchExecPath->iEntrySent,
            pSearchExecPath->bExceedMaxIteration,
            pSearchExecPath->bPagedSearch,
            pSearchExecPath->bPagedSearchDone,
            pSearchExecPath->candiatePlan.iCandateSize,
            VDIR_SAFE_STRING(pSearchExecPath->pszIndex),
            pSearchExecPath->IteratePlan.iNumIteration);

    goto cleanup;
}

VOID
TestFreeSearchOpCtxContent(
    PVMDIR_SEARCH_OP_CONTEXT pOpCtx
    )
{
    ber_bvfree(pOpCtx->pbvCookie);
    VMDIR_SAFE_FREE_MEMORY(pOpCtx->pszRestPagedCookie);
}

DWORD
TestExecuteNormalSearch(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    )
{
    DWORD   dwError = 0;
    VMDIR_SEARCH_OP_CONTEXT    opCtx = {0};
    VDIR_SEARCH_EXEC_PATH      searchExecPath = {0};

    dwError = TestSendSearch(pContext, pCase, &opCtx, &searchExecPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestValidateSearchResult(pCase, &opCtx, &searchExecPath);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    TestFreeSearchOpCtxContent(&opCtx);
    VmDirSearchExecPathFreeContent(&searchExecPath);

    return dwError;

error:
    goto cleanup;
}

DWORD
TestExecutePagedSearch(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    )
{
    DWORD   dwError = 0;
    VMDIR_SEARCH_OP_CONTEXT    opCtx = {0};
    VDIR_SEARCH_EXEC_PATH      searchExecPath = {0};

    opCtx.bPaged = TRUE;
    opCtx.dwPageSize = pCase->definition.dwPageSize;

    while (!opCtx.bDone)
    {
        dwError = TestSendSearch(pContext, pCase, &opCtx, &searchExecPath);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    searchExecPath.iEntrySent = opCtx.dwTotalResultCount;

    dwError = TestValidateSearchResult(pCase, &opCtx, &searchExecPath);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    TestFreeSearchOpCtxContent(&opCtx);
    VmDirSearchExecPathFreeContent(&searchExecPath);

    return dwError;

error:
    goto cleanup;
}

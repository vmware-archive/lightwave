/*
 * Copyright © 2097 VMware, Inc.  All Rights Reserved.
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
TestUserWarnExpire(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    )
{
    DWORD   dwError = 0;
    PSTR    ppszAttrValue[] = { NULL, NULL };
    PSTR    pszPwdLastSet = NULL;
    time_t  tNow = time(NULL) - 85*24*60*60;  // default expire 90 days
    PVMDIR_TEST_STATE pState = pPolicyContext->pTestState;
    VMDIR_PP_CTRL_BIND ctrlBind = {0};

    dwError = VmDirAllocateStringPrintf(&pszPwdLastSet, "%d", tNow);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppszAttrValue[0] = pszPwdLastSet;
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pszTestUserDN,
        ATTR_PWD_LAST_SET,
        (PCSTR*)ppszAttrValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    ctrlBind.pszMech = TEST_SASL_SRP;
    ctrlBind.pszHost = pState->pszServerName;
    ctrlBind.pszDomain = pState->pszDomain;
    ctrlBind.pszBindCN = pPolicyContext->pszTestUserCN;
    ctrlBind.pszBindUPN = pPolicyContext->pszTestUserUPN;
    ctrlBind.pszBindDN = pPolicyContext->pszTestUserDN;
    ctrlBind.pszPassword = pPolicyContext->pszTestUserPassword;

    dwError = TestPPCtrlBind(&ctrlBind);
    BAIL_ON_VMDIR_ERROR(dwError);

    TestAssertEquals(ctrlBind.bHasPPCtrlResponse, 1);
    TestAssertEquals(ctrlBind.dwBindResult, 0);
    // expect expiring around 5 days
    TestAssertBetween(5*24*60*60 - 10, ctrlBind.PPolicyState.iWarnPwdExpiring, 5*24*60*60 + 10);

    memset(&ctrlBind.PPolicyState, 0, sizeof(ctrlBind.PPolicyState));
    ctrlBind.pszMech = TEST_SASL_SIMPLE;
    ctrlBind.bHasPPCtrlResponse = 0;
    ctrlBind.dwBindResult = 0;

    dwError = TestPPCtrlBind(&ctrlBind);
    BAIL_ON_VMDIR_ERROR(dwError);

    TestAssertEquals(ctrlBind.bHasPPCtrlResponse, 1);
    TestAssertEquals(ctrlBind.dwBindResult, 0);
    TestAssertBetween(5*24*60*60 - 10, ctrlBind.PPolicyState.iWarnPwdExpiring, 5*24*60*60 + 10);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszPwdLastSet);
    return dwError;

error:
    goto cleanup;
}

DWORD
TestPolicy(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    )
{
    DWORD   dwError = 0;

    dwError = TestUserWarnExpire(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);;

cleanup:
    printf("%s %s (%d)\n", __FUNCTION__, dwError ? "failed" : "succeeded", dwError);
    return dwError;

error:
    goto cleanup;
}


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
TestPolicyControlWarnExpire(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PSTR    ppszAttrValue[] = { NULL, NULL };
    PSTR    pszPwdLastSet = NULL;
    time_t  tNow = time(NULL) - 85*24*60*60;  // default expire 90 days
    PVMDIR_TEST_STATE pState = pPolicyContext->pTestState;
    VMDIR_PP_CTRL_BIND ctrlBind = {0};

    struct _TestWarnExpireParam
    {
        PCSTR pszTargetDN;
        PCSTR pszTargetUPN;
        PCSTR pszTargetPasswd;
        DWORD dwResult;
        BOOLEAN bHasPPCtrlResp;
    }
    TestWarnExpireParam[] =
    {   // normal user password expiring warning
        {   pPolicyContext->pszTestUserDN,
            pPolicyContext->pszTestUserUPN,
            pPolicyContext->pszTestUserPassword,
            0,
            1
        },
        // admin user not subject to password expiring warning
        {   pPolicyContext->pTestState->pszUserDN,
            pPolicyContext->pTestState->pszUserUPN,
            pPolicyContext->pTestState->pszPassword,
            0,
            0
        },
    };

    dwError = VmDirAllocateStringPrintf(&pszPwdLastSet, "%d", tNow);
    BAIL_ON_VMDIR_ERROR(dwError);

    ctrlBind.pszHost = pState->pszServerName;
    ctrlBind.pszDomain = pState->pszDomain;

    for (dwCnt=0; dwCnt < sizeof(TestWarnExpireParam)/sizeof(TestWarnExpireParam[0]); dwCnt++)
    {
        ppszAttrValue[0] = pszPwdLastSet;
        dwError = VmDirTestReplaceAttributeValues(
            pPolicyContext->pTestState->pLd,
            TestWarnExpireParam[dwCnt].pszTargetDN,
            ATTR_PWD_LAST_SET,
            (PCSTR*)ppszAttrValue);
        BAIL_ON_VMDIR_ERROR(dwError);

        ctrlBind.pszBindUPN = TestWarnExpireParam[dwCnt].pszTargetUPN;
        ctrlBind.pszBindDN = TestWarnExpireParam[dwCnt].pszTargetDN;
        ctrlBind.pszPassword = TestWarnExpireParam[dwCnt].pszTargetPasswd;

        {
            ctrlBind.pszMech = TEST_SASL_SRP;
            memset(&ctrlBind.ctrlResult, 0, sizeof(ctrlBind.ctrlResult));

            dwError = TestPPCtrlBind(&ctrlBind);
            BAIL_ON_VMDIR_ERROR(dwError);

            TestAssertEquals(ctrlBind.ctrlResult.bHasPPCtrlResponse, TestWarnExpireParam[dwCnt].bHasPPCtrlResp);
            TestAssertEquals(ctrlBind.ctrlResult.dwOpResult, TestWarnExpireParam[dwCnt].dwResult);
            if (ctrlBind.ctrlResult.bHasPPCtrlResponse)
            {   // expect expiring around 5 days
                TestAssertBetween(5*24*60*60 - 10, ctrlBind.ctrlResult.PPolicyState.iWarnPwdExpiring, 5*24*60*60 + 10);
            }
        }

        {
            ctrlBind.pszMech = TEST_SASL_SIMPLE;
            memset(&ctrlBind.ctrlResult, 0, sizeof(ctrlBind.ctrlResult));

            dwError = TestPPCtrlBind(&ctrlBind);
            BAIL_ON_VMDIR_ERROR(dwError);

            TestAssertEquals(ctrlBind.ctrlResult.bHasPPCtrlResponse, TestWarnExpireParam[dwCnt].bHasPPCtrlResp);
            TestAssertEquals(ctrlBind.ctrlResult.dwOpResult, TestWarnExpireParam[dwCnt].dwResult);
            if (ctrlBind.ctrlResult.bHasPPCtrlResponse)
            {
                TestAssertBetween(5*24*60*60 - 10, ctrlBind.ctrlResult.PPolicyState.iWarnPwdExpiring, 5*24*60*60 + 10);
            }
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszPwdLastSet);
    return dwError;

error:
    goto cleanup;
}

DWORD
TestPolicyControlErrorExpire(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PSTR    ppszAttrValue[] = { NULL, NULL };
    PSTR    pszPwdLastSet = NULL;
    time_t  tNow = time(NULL) - 91*24*60*60;  // default expire 90 days
    PVMDIR_TEST_STATE pState = pPolicyContext->pTestState;
    VMDIR_PP_CTRL_BIND ctrlBind = {0};

    struct _TestErrorExpireParam
    {
        PCSTR pszTargetDN;
        PCSTR pszTargetUPN;
        PCSTR pszTargetPasswd;
        DWORD dwResult;
        BOOLEAN bHasPPCtrlResp;
        LDAPPasswordPolicyError PolicyError;
    }
    TestErrorExpireParam[] =
    {   // normal user password expired
        {   pPolicyContext->pszTestUserDN,
            pPolicyContext->pszTestUserUPN,
            pPolicyContext->pszTestUserPassword,
            49,
            1,
            PP_passwordExpired
        },
        // admin user not subject to password expiration
        {   pPolicyContext->pTestState->pszUserDN,
            pPolicyContext->pTestState->pszUserUPN,
            pPolicyContext->pTestState->pszPassword,
            0,
            0,
            PP_noError
        },
    };

    dwError = VmDirAllocateStringPrintf(&pszPwdLastSet, "%d", tNow);
    BAIL_ON_VMDIR_ERROR(dwError);

    ctrlBind.pszHost = pState->pszServerName;
    ctrlBind.pszDomain = pState->pszDomain;

    for (dwCnt=0; dwCnt < sizeof(TestErrorExpireParam)/sizeof(TestErrorExpireParam[0]); dwCnt++)
    {
        ppszAttrValue[0] = pszPwdLastSet;
        dwError = VmDirTestReplaceAttributeValues(
            pPolicyContext->pTestState->pLd,
            TestErrorExpireParam[dwCnt].pszTargetDN,
            ATTR_PWD_LAST_SET,
            (PCSTR*)ppszAttrValue);
        BAIL_ON_VMDIR_ERROR(dwError);

        ctrlBind.pszBindUPN = TestErrorExpireParam[dwCnt].pszTargetUPN;
        ctrlBind.pszBindDN = TestErrorExpireParam[dwCnt].pszTargetDN;
        ctrlBind.pszPassword = TestErrorExpireParam[dwCnt].pszTargetPasswd;

        {
            ctrlBind.pszMech = TEST_SASL_SRP;
            memset(&ctrlBind.ctrlResult, 0, sizeof(ctrlBind.ctrlResult));
            ctrlBind.ctrlResult.PPolicyState.PPolicyError = PP_noError;

            dwError = TestPPCtrlBind(&ctrlBind);
            BAIL_ON_VMDIR_ERROR(dwError);

            TestAssertEquals(ctrlBind.ctrlResult.bHasPPCtrlResponse, TestErrorExpireParam[dwCnt].bHasPPCtrlResp);
            TestAssertEquals(ctrlBind.ctrlResult.dwOpResult, TestErrorExpireParam[dwCnt].dwResult);
            TestAssertEquals(ctrlBind.ctrlResult.PPolicyState.PPolicyError, TestErrorExpireParam[dwCnt].PolicyError);
        }

        {
            ctrlBind.pszMech = TEST_SASL_SIMPLE;
            memset(&ctrlBind.ctrlResult, 0, sizeof(ctrlBind.ctrlResult));
            ctrlBind.ctrlResult.PPolicyState.PPolicyError = PP_noError;

            dwError = TestPPCtrlBind(&ctrlBind);
            BAIL_ON_VMDIR_ERROR(dwError);

            TestAssertEquals(ctrlBind.ctrlResult.bHasPPCtrlResponse, TestErrorExpireParam[dwCnt].bHasPPCtrlResp);
            TestAssertEquals(ctrlBind.ctrlResult.dwOpResult, TestErrorExpireParam[dwCnt].dwResult);
            TestAssertEquals(ctrlBind.ctrlResult.PPolicyState.PPolicyError, TestErrorExpireParam[dwCnt].PolicyError);
        }
    }

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
    PSTR    ppszValue[] = { "0", NULL };

    dwError = TestPolicyControlErrorExpire(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pszTestUserDN,
        ATTR_USER_ACCOUNT_CONTROL,
        (PCSTR*)ppszValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestPolicyControlWarnExpire(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    printf("%s %s (%d)\n", __FUNCTION__, dwError ? "failed" : "succeeded", dwError);
    return dwError;

error:
    goto cleanup;
}


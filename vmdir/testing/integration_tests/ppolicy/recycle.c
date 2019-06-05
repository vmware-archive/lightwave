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
InitializeRecycleSetup(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    )
{
    DWORD   dwError = 0;
    PSTR ppszAttrValue[] = { NULL, NULL };

    struct _RecycleParam
    {
        PSTR pszAttrName;
        PSTR pszValue;
    }
    RecycleParam[] =
    {
        { ATTR_PASS_RECYCLE_CNT,  "3" },
    };

    ppszAttrValue[0] = RecycleParam[0].pszValue;
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pszPolicyDN,
        RecycleParam[0].pszAttrName,
        (PCSTR*)ppszAttrValue);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
TestAdminRecycle(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVMDIR_TEST_STATE pState = pPolicyContext->pTestState;
    VMDIR_PP_CTRL_MODIFY   ctrlModify = {0};

    struct _RecycleRec
    {
        PSTR    pszPwd;
        DWORD   dwResult;
        DWORD   dwPPolicyError;
    }
    RecycleRec[] =
    {
        { "Recycle-0-1",  0 , 0},
        { "Recycle-0-2",  0 , 0},
        { "Recycle-0-3",  0 , 0},
        { "Recycle-0-3",  0 , 0}, // admin NOT subject to recycle rule
        { "Recycle-0-2",  0 , 0}, // regardless who's password it modify
        { "Recycle-0-1",  0 , 0},
    };

    for (dwCnt=0; dwCnt < sizeof(RecycleRec)/sizeof(RecycleRec[0]); dwCnt++)
    {
        memset(&ctrlModify, 0, sizeof(ctrlModify));
        ctrlModify.pszTargetDN = pPolicyContext->pszTestUserDN;
        ctrlModify.pszPassword = RecycleRec[dwCnt].pszPwd;

        dwError = TestModifyPassword(
            pPolicyContext->pTestState->pLd,    // admin user modify normal user pwd
            &ctrlModify);
        BAIL_ON_VMDIR_ERROR(dwError);

        TestAssertEquals(ctrlModify.ctrlResult.dwOpResult, RecycleRec[dwCnt].dwResult);
        if (RecycleRec[dwCnt].dwPPolicyError != 0)
        {
            TestAssertEquals(ctrlModify.ctrlResult.bHasPPCtrlResponse, 1);
            TestAssertEquals(ctrlModify.ctrlResult.PPolicyState.PPolicyError, RecycleRec[dwCnt].dwPPolicyError);
        }

        memset(&ctrlModify, 0, sizeof(ctrlModify));
        ctrlModify.pszTargetDN = pPolicyContext->pTestState->pszUserDN;
        ctrlModify.pszPassword = RecycleRec[dwCnt].pszPwd;

        dwError = TestModifyPassword(
            pPolicyContext->pTestState->pLd,    // admin user modify admin user pwd
            &ctrlModify);
        BAIL_ON_VMDIR_ERROR(dwError);

        TestAssertEquals(ctrlModify.ctrlResult.dwOpResult, RecycleRec[dwCnt].dwResult);
        if (RecycleRec[dwCnt].dwPPolicyError != 0)
        {
            TestAssertEquals(ctrlModify.ctrlResult.bHasPPCtrlResponse, 1);
            TestAssertEquals(ctrlModify.ctrlResult.PPolicyState.PPolicyError, RecycleRec[dwCnt].dwPPolicyError);
        }

    }

error:
    return dwError;
}

DWORD
TestUserRecycle(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVMDIR_TEST_STATE pState = pPolicyContext->pTestState;
    VMDIR_PP_CTRL_MODIFY   ctrlModify = {0};

    struct _RecycleRec
    {
        PSTR    pszPwd;
        DWORD   dwResult;
        DWORD   dwPPolicyError;
    }
    RecycleRec[] =
    {
        { "Recycle-1-1",  0 , 0},
        { "Recycle-1-2",  0 , 0},
        { "Recycle-1-3",  0 , 0},
        { "Recycle-1-1",  19, 8}, // user recycle should fail
        { "Recycle-1-4",  0 , 0},
        { "Recycle-1-1",  0 , 0}, // recycle ok, pass count 3.
    };

    for (dwCnt=0; dwCnt < sizeof(RecycleRec)/sizeof(RecycleRec[0]); dwCnt++)
    {
        memset(&ctrlModify, 0, sizeof(ctrlModify));
        ctrlModify.pszTargetDN = pPolicyContext->pszTestUserDN;
        ctrlModify.pszPassword = RecycleRec[dwCnt].pszPwd;

        dwError = TestModifyPassword(
            pPolicyContext->pLdUser,    // normal user modify self pwd
            &ctrlModify);
        BAIL_ON_VMDIR_ERROR(dwError);

        TestAssertEquals(ctrlModify.ctrlResult.dwOpResult, RecycleRec[dwCnt].dwResult);
        if (RecycleRec[dwCnt].dwPPolicyError != 0)
        {
            TestAssertEquals(ctrlModify.ctrlResult.bHasPPCtrlResponse, 1);
            TestAssertEquals(ctrlModify.ctrlResult.PPolicyState.PPolicyError, RecycleRec[dwCnt].dwPPolicyError);
        }
    }

error:
    return dwError;
}

/*
 * restore original password
 * restore recycle count to 0
 */
DWORD
CleanRecycleSetup(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    )
{
    DWORD   dwError = 0;

    PSTR ppszAttrValue[] = { NULL, NULL };

    ppszAttrValue[0] = (PSTR)pPolicyContext->pTestState->pszPassword;
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pTestState->pszUserDN,
        ATTR_USER_PASSWORD,
        (PCSTR*)ppszAttrValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppszAttrValue[0] = (PSTR)pPolicyContext->pszTestUserPassword;
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pszTestUserDN,
        ATTR_USER_PASSWORD,
        (PCSTR*)ppszAttrValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppszAttrValue[0] = "0";
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pszPolicyDN,
        ATTR_PASS_RECYCLE_CNT,
        (PCSTR*)ppszAttrValue);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
TestRecycle(
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext
    )
{
    DWORD   dwError = 0;

    dwError = InitializeRecycleSetup(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestAdminRecycle(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestUserRecycle(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = CleanRecycleSetup(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    printf("%s %s (%d)\n", __FUNCTION__, dwError ? "failed" : "succeeded", dwError);
    return dwError;

error:
    goto cleanup;
}

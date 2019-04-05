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

DWORD
_TestPwdStrength(
    PVMDIR_PPOLICY_TEST_CONTEXT     pPolicyContext,
    PVMDIR_STRENGTH_TEST_REC        pRec
    )
{
    DWORD   dwError = 0;
    PVMDIR_TEST_STATE pState = pPolicyContext->pTestState;
    PSTR ppszAttrValue[] = { NULL, NULL };
    PSTR ppszBad[] = { NULL, NULL };
    PSTR ppszGood[] = { NULL, NULL };

    ppszAttrValue[0] = pRec->pszTestValue;
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pszPolicyDN,
        pRec->pszAttr,
        (PCSTR*)ppszAttrValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppszBad[0] = pRec->pszBadPwd;
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,    // via admin creds
        pPolicyContext->pszTestUserDN,
        ATTR_USER_PASSWORD,
        (PCSTR*)ppszBad);
    TestAssertEquals(dwError, 19);

    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pLdUser,            // via user/self creds
        pPolicyContext->pszTestUserDN,
        ATTR_USER_PASSWORD,
        (PCSTR*)ppszBad);
    TestAssertEquals(dwError, 19);

    ppszGood[0] = pRec->pszGoodPwd;
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,    // via admin creds
        pPolicyContext->pszTestUserDN,
        ATTR_USER_PASSWORD,
        (PCSTR*)ppszGood);
    TestAssertEquals(dwError, 0);

    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pLdUser,            // via user/self creds
        pPolicyContext->pszTestUserDN,
        ATTR_USER_PASSWORD,
        (PCSTR*)ppszGood);
    TestAssertEquals(dwError, 0);

    ppszAttrValue[0] = pRec->pszRestoreValue;
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pszPolicyDN,
        pRec->pszAttr,
        (PCSTR*)ppszAttrValue);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
TestPwdStrength(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    VMDIR_STRENGTH_TEST_REC initTbl[] =
    {                                                       \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MIN_SIZE,    \
            /*.pszTestValue     = */ "2",                   \
            /*.pszRestoreValue  = */ "2",                   \
            /*.pszGoodPwd       = */ "ab",                  \
            /*.pszBadPwd        = */ "a",                   \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MIN_MUN_CHAR,    \
            /*.pszTestValue     = */ "1",                   \
            /*.pszRestoreValue  = */ "0",                   \
            /*.pszGoodPwd       = */ "b1",                  \
            /*.pszBadPwd        = */ "bc",                  \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MIN_SP_CHAR,    \
            /*.pszTestValue     = */ "1",                   \
            /*.pszRestoreValue  = */ "0",                   \
            /*.pszGoodPwd       = */ "c!",                  \
            /*.pszBadPwd        = */ "c1",                  \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MIN_ALPHA_CHAR,  \
            /*.pszTestValue     = */ "1",                   \
            /*.pszRestoreValue  = */ "1",                   \
            /*.pszGoodPwd       = */ "d0",                  \
            /*.pszBadPwd        = */ "0!",                  \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MIN_LOWER_CHAR,  \
            /*.pszTestValue     = */ "1",                   \
            /*.pszRestoreValue  = */ "0",                   \
            /*.pszGoodPwd       = */ "eF",                  \
            /*.pszBadPwd        = */ "EF",                  \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MIN_UPPER_CHAR,  \
            /*.pszTestValue     = */ "1",                   \
            /*.pszRestoreValue  = */ "0",                   \
            /*.pszGoodPwd       = */ "Fg",                  \
            /*.pszBadPwd        = */ "fg",                  \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MAX_SIZE,    \
            /*.pszTestValue     = */ "3",                   \
            /*.pszRestoreValue  = */ "20",                  \
            /*.pszGoodPwd       = */ "gHI",                 \
            /*.pszBadPwd        = */ "gHIJ",                \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MAX_SAME_ADJ_CHAR,    \
            /*.pszTestValue     = */ "2",                   \
            /*.pszRestoreValue  = */ "1",                   \
            /*.pszGoodPwd       = */ "hIIj",                \
            /*.pszBadPwd        = */ "hIIIj",               \
        },                                                  \
    };

    for (dwCnt=0; dwCnt < sizeof(initTbl)/sizeof(initTbl[0]); dwCnt++)
    {
        PVMDIR_STRENGTH_TEST_REC pRec = &initTbl[dwCnt];
        dwError = _TestPwdStrength(pPolicyContext, pRec);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    printf("%s %s (%d)\n", __FUNCTION__, dwError ? "failed" : "succeeded", dwError);
    return dwError;

error:
    goto cleanup;
}
